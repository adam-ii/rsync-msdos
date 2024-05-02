/*
   Copyright (C) Andrew Tridgell 1996
   Copyright (C) Paul Mackerras 1996
   Copyright (C) 2001, 2002 by Martin Pool <mbp@samba.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/** @file flist.c
 * Generate and receive file lists
 *
 * @todo Get rid of the string_area optimization.  Efficiently
 * allocating blocks is the responsibility of the system's malloc
 * library, not of rsync.
 *
 * @sa http://lists.samba.org/pipermail/rsync/2000-June/002351.html
 *
 **/

#include "rsync.h"

extern struct stats stats;

extern int verbose;
extern int do_progress;
extern int am_server;
extern int always_checksum;

extern int cvs_exclude;

extern int recurse;
extern char *files_from;
extern int filesfrom_fd;

extern int one_file_system;
extern int make_backups;
extern int preserve_links;
extern int preserve_hard_links;
extern int preserve_perms;
extern int preserve_devices;
extern int preserve_uid;
extern int preserve_gid;
extern int preserve_times;
extern int relative_paths;
extern int implied_dirs;
extern int copy_links;
extern int copy_unsafe_links;
extern int protocol_version;
extern int sanitize_paths;

extern int read_batch;
extern int write_batch;

extern struct exclude_struct **exclude_list;
extern struct exclude_struct **server_exclude_list;
extern struct exclude_struct **local_exclude_list;

int io_error;

static struct file_struct null_file;

static void clean_flist(struct file_list *flist, int strip_root, int no_dups);


static int show_filelist_p(void)
{
	return verbose && (recurse || files_from) && !am_server;
}

static void start_filelist_progress(char *kind)
{
	rprintf(FINFO, "%s ... ", kind);
	if ((verbose > 1) || do_progress)
		rprintf(FINFO, "\n");
	rflush(FINFO);
}


static void emit_filelist_progress(const struct file_list *flist)
{
	rprintf(FINFO, " %d files...\r", flist->count);
}


static void maybe_emit_filelist_progress(const struct file_list *flist)
{
	if (do_progress && show_filelist_p() && ((flist->count % 100) == 0))
		emit_filelist_progress(flist);
}


static void finish_filelist_progress(const struct file_list *flist)
{
	if (do_progress) {
		/* This overwrites the progress line */
		rprintf(FINFO, "%d file%sto consider\n",
			flist->count, flist->count == 1 ? " " : "s ");
	} else {
		rprintf(FINFO, "done\n");
	}
}

void show_flist_stats(void)
{
	/* Nothing yet */
}


static struct string_area *string_area_new(int size)
{
	struct string_area *a;

	if (size <= 0)
		size = ARENA_SIZE;
	a = new(struct string_area);
	if (!a)
		out_of_memory("string_area_new");
	a->current = a->base = new_array(char, size);
	if (!a->current)
		out_of_memory("string_area_new buffer");
	a->end = a->base + size;
	a->next = NULL;

	return a;
}

static void string_area_free(struct string_area *a)
{
	struct string_area *next;

	for (; a; a = next) {
		next = a->next;
		free(a->base);
	}
}

static char *string_area_malloc(struct string_area **ap, int size)
{
	char *p;
	struct string_area *a;

	/* does the request fit into the current space? */
	a = *ap;
	if (a->current + size >= a->end) {
		/* no; get space, move new string_area to front of the list */
		a = string_area_new(size > ARENA_SIZE ? size : ARENA_SIZE);
		a->next = *ap;
		*ap = a;
	}

	/* have space; do the "allocation." */
	p = a->current;
	a->current += size;
	return p;
}

static char *string_area_strdup(struct string_area **ap, const char *src)
{
	char *dest = string_area_malloc(ap, strlen(src) + 1);
	return strcpy(dest, src);
}

static void list_file_entry(struct file_struct *f)
{
	char perms[11];

	if (!f->basename)
		/* this can happen if duplicate names were removed */
		return;

	permstring(perms, f->mode);

	if (preserve_links && S_ISLNK(f->mode)) {
		rprintf(FINFO, "%s %11.0f %s %s -> %s\n",
			perms,
			(double) f->length, timestring(f->modtime),
			f_name(f), f->link);
	} else {
		rprintf(FINFO, "%s %11.0f %s %s\n",
			perms,
			(double) f->length, timestring(f->modtime),
			f_name(f));
	}
}


/**
 * Stat either a symlink or its referent, depending on the settings of
 * copy_links, copy_unsafe_links, etc.
 *
 * @retval -1 on error
 *
 * @retval 0 for success
 *
 * @post If @p path is a symlink, then @p linkbuf (of size @c
 * MAXPATHLEN) contains the symlink target.
 *
 * @post @p buffer contains information about the link or the
 * referrent as appropriate, if they exist.
 **/
int readlink_stat(const char *path, STRUCT_STAT * buffer, char *linkbuf)
{
#if SUPPORT_LINKS
	if (copy_links) {
		return do_stat(path, buffer);
	}
	if (do_lstat(path, buffer) == -1) {
		return -1;
	}
	if (S_ISLNK(buffer->st_mode)) {
		int l;
		l = readlink((char *) path, linkbuf, MAXPATHLEN - 1);
		if (l == -1)
			return -1;
		linkbuf[l] = 0;
		if (copy_unsafe_links && unsafe_symlink(linkbuf, path)) {
			if (verbose > 1) {
				rprintf(FINFO,"copying unsafe symlink \"%s\" -> \"%s\"\n",
					path, linkbuf);
			}
			return do_stat(path, buffer);
		}
	}
	return 0;
#else
	return do_stat(path, buffer);
#endif
}

int link_stat(const char *path, STRUCT_STAT * buffer)
{
#if SUPPORT_LINKS
	if (copy_links) {
		return do_stat(path, buffer);
	} else {
		return do_lstat(path, buffer);
	}
#else
	return do_stat(path, buffer);
#endif
}

/*
 * This function is used to check if a file should be included/excluded
 * from the list of files based on its name and type etc.  The value of
 * exclude_level is set to either SERVER_EXCLUDES or ALL_EXCLUDES.
 */
static int check_exclude_file(char *fname, int is_dir, int exclude_level)
{
#if 0 /* This currently never happens, so avoid a useless compare. */
	if (exclude_level == NO_EXCLUDES)
		return 0;
#endif
	if (fname) {
		/* never exclude '.', even if somebody does --exclude '*' */
		if (fname[0] == '.' && !fname[1])
			return 0;
		/* Handle the -R version of the '.' dir. */
		if (fname[0] == '/') {
			int len = strlen(fname);
			if (fname[len-1] == '.' && fname[len-2] == '/')
				return 0;
		}
	}
	if (server_exclude_list
	 && check_exclude(server_exclude_list, fname, is_dir))
		return 1;
	if (exclude_level != ALL_EXCLUDES)
		return 0;
	if (exclude_list && check_exclude(exclude_list, fname, is_dir))
		return 1;
	if (local_exclude_list
	 && check_exclude(local_exclude_list, fname, is_dir))
		return 1;
	return 0;
}

/* used by the one_file_system code */
static dev_t filesystem_dev;

static void set_filesystem(char *fname)
{
	STRUCT_STAT st;
	if (link_stat(fname, &st) != 0)
		return;
	filesystem_dev = st.st_dev;
}


static int32 to_wire_mode(mode_t mode)
{
	if (S_ISLNK(mode) && (_S_IFLNK != 0120000)) {
		return (mode & ~(_S_IFMT)) | 0120000;
	}
	return (int32) mode;
}

static mode_t from_wire_mode(int32 mode)
{
	if ((mode & (_S_IFMT)) == 0120000 && (_S_IFLNK != 0120000)) {
		return (mode & ~(_S_IFMT)) | _S_IFLNK;
	}
	return (mode_t) mode;
}


static void send_directory(int f, struct file_list *flist, char *dir);

static char *flist_dir;


/**
 * Make sure @p flist is big enough to hold at least @p flist->count
 * entries.
 **/
static void flist_expand(struct file_list *flist)
{
	if (flist->count >= flist->malloced) {
		void *new_ptr;

		if (flist->malloced < 1000)
			flist->malloced += 1000;
		else
			flist->malloced *= 2;

		if (flist->files) {
			new_ptr = realloc_array(flist->files,
						struct file_struct *,
						flist->malloced);
		} else {
			new_ptr = new_array(struct file_struct *,
					    flist->malloced);
		}

		if (verbose >= 2) {
			rprintf(FINFO, "expand file_list to %.0f bytes, did%s move\n",
				(double)sizeof(flist->files[0])
				* flist->malloced,
				(new_ptr == flist->files) ? " not" : "");
		}

		flist->files = (struct file_struct **) new_ptr;

		if (!flist->files)
			out_of_memory("flist_expand");
	}
}


static void send_file_entry(struct file_struct *file, int f,
			    unsigned base_flags)
{
	unsigned char flags;
	static time_t last_time;
	static mode_t last_mode;
	static DEV64_T last_rdev;
	static uid_t last_uid;
	static gid_t last_gid;
	static char lastname[MAXPATHLEN];
	char *fname;
	int l1, l2;

	if (f == -1)
		return;

	if (!file) {
		write_byte(f, 0);
		return;
	}

	io_write_phase = "send_file_entry";

	fname = f_name(file);

	flags = base_flags;

	if (file->mode == last_mode)
		flags |= SAME_MODE;
	if (file->rdev == last_rdev)
		flags |= SAME_RDEV;
	if (file->uid == last_uid)
		flags |= SAME_UID;
	if (file->gid == last_gid)
		flags |= SAME_GID;
	if (file->modtime == last_time)
		flags |= SAME_TIME;

	for (l1 = 0;
	     lastname[l1] && (fname[l1] == lastname[l1]) && (l1 < 255);
	     l1++) {}
	l2 = strlen(fname) - l1;

	if (l1 > 0)
		flags |= SAME_NAME;
	if (l2 > 255)
		flags |= LONG_NAME;

	/* we must make sure we don't send a zero flags byte or the other
	   end will terminate the flist transfer */
	if (flags == 0 && !S_ISDIR(file->mode))
		flags |= FLAG_DELETE;
	if (flags == 0)
		flags |= LONG_NAME;

	write_byte(f, flags);
	if (flags & SAME_NAME)
		write_byte(f, l1);
	if (flags & LONG_NAME)
		write_int(f, l2);
	else
		write_byte(f, l2);
	write_buf(f, fname + l1, l2);

	write_longint(f, file->length);
	if (!(flags & SAME_TIME))
		write_int(f, file->modtime);
	if (!(flags & SAME_MODE))
		write_int(f, to_wire_mode(file->mode));
	if (preserve_uid && !(flags & SAME_UID)) {
		add_uid(file->uid);
		write_int(f, file->uid);
	}
	if (preserve_gid && !(flags & SAME_GID)) {
		add_gid(file->gid);
		write_int(f, file->gid);
	}
	if (preserve_devices && IS_DEVICE(file->mode)
	    && !(flags & SAME_RDEV))
		write_int(f, file->rdev);

#if SUPPORT_LINKS
	if (preserve_links && S_ISLNK(file->mode)) {
		write_int(f, strlen(file->link));
		write_buf(f, file->link, strlen(file->link));
	}
#endif

#if SUPPORT_HARD_LINKS
	if (preserve_hard_links && S_ISREG(file->mode)) {
		if (protocol_version < 26) {
			/* 32-bit dev_t and ino_t */
			write_int(f, (int) file->dev);
			write_int(f, (int) file->inode);
		} else {
			/* 64-bit dev_t and ino_t */
			write_longint(f, file->dev);
			write_longint(f, file->inode);
		}
	}
#endif

	if (always_checksum) {
		if (protocol_version < 21) {
			write_buf(f, file->sum, 2);
		} else {
			write_buf(f, file->sum, MD4_SUM_LENGTH);
		}
	}

	last_mode = file->mode;
	last_rdev = file->rdev;
	last_uid = file->uid;
	last_gid = file->gid;
	last_time = file->modtime;

	strlcpy(lastname, fname, MAXPATHLEN);
	lastname[MAXPATHLEN - 1] = 0;

	io_write_phase = "unknown";
}



static void receive_file_entry(struct file_struct **fptr,
			       unsigned flags, int f)
{
	static time_t last_time;
	static mode_t last_mode;
	static DEV64_T last_rdev;
	static uid_t last_uid;
	static gid_t last_gid;
	static char lastname[MAXPATHLEN];
	char thisname[MAXPATHLEN];
	unsigned int l1 = 0, l2 = 0;
	char *p;
	struct file_struct *file;

	if (flags & SAME_NAME)
		l1 = read_byte(f);

	if (flags & LONG_NAME)
		l2 = read_int(f);
	else
		l2 = read_byte(f);

	file = new(struct file_struct);
	if (!file)
		out_of_memory("receive_file_entry");
	memset((char *) file, 0, sizeof(*file));
	(*fptr) = file;

	if (l2 >= MAXPATHLEN - l1) {
		rprintf(FERROR,
			"overflow: flags=0x%x l1=%d l2=%d lastname=%s\n",
			flags, l1, l2, lastname);
		overflow("receive_file_entry");
	}

	strlcpy(thisname, lastname, l1 + 1);
	read_sbuf(f, &thisname[l1], l2);
	thisname[l1 + l2] = 0;

	strlcpy(lastname, thisname, MAXPATHLEN);
	lastname[MAXPATHLEN - 1] = 0;

	clean_fname(thisname);

	if (sanitize_paths) {
		sanitize_path(thisname, NULL);
	}

	if ((p = strrchr(thisname, '/'))) {
		static char *lastdir;
		*p = 0;
		if (lastdir && strcmp(thisname, lastdir) == 0) {
			file->dirname = lastdir;
		} else {
			file->dirname = strdup(thisname);
			lastdir = file->dirname;
		}
		file->basename = strdup(p + 1);
	} else {
		file->dirname = NULL;
		file->basename = strdup(thisname);
	}

	if (!file->basename)
		out_of_memory("receive_file_entry 1");


	file->flags = flags;
	file->length = read_longint(f);
	file->modtime =
	    (flags & SAME_TIME) ? last_time : (time_t) read_int(f);
	file->mode =
	    (flags & SAME_MODE) ? last_mode : from_wire_mode(read_int(f));
	if (preserve_uid)
		file->uid =
		    (flags & SAME_UID) ? last_uid : (uid_t) read_int(f);
	if (preserve_gid)
		file->gid =
		    (flags & SAME_GID) ? last_gid : (gid_t) read_int(f);
	if (preserve_devices && IS_DEVICE(file->mode))
		file->rdev =
		    (flags & SAME_RDEV) ? last_rdev : (DEV64_T) read_int(f);

	if (preserve_links && S_ISLNK(file->mode)) {
		int l = read_int(f);
		if (l < 0) {
			rprintf(FERROR, "overflow: l=%d\n", l);
			overflow("receive_file_entry");
		}
		file->link = new_array(char, l + 1);
		if (!file->link)
			out_of_memory("receive_file_entry 2");
		read_sbuf(f, file->link, l);
		if (sanitize_paths) {
			sanitize_path(file->link, file->dirname);
		}
	}
#if SUPPORT_HARD_LINKS
	if (preserve_hard_links && S_ISREG(file->mode)) {
		if (protocol_version < 26) {
			file->dev = read_int(f);
			file->inode = read_int(f);
		} else {
			file->dev = read_longint(f);
			file->inode = read_longint(f);
		}
	}
#endif

	if (always_checksum) {
		file->sum = new_array(char, MD4_SUM_LENGTH);
		if (!file->sum)
			out_of_memory("md4 sum");
		if (protocol_version < 21) {
			read_buf(f, file->sum, 2);
		} else {
			read_buf(f, file->sum, MD4_SUM_LENGTH);
		}
	}

	last_mode = file->mode;
	last_rdev = file->rdev;
	last_uid = file->uid;
	last_gid = file->gid;
	last_time = file->modtime;

	if (!preserve_perms) {
		extern int orig_umask;
		/* set an appropriate set of permissions based on original
		   permissions and umask. This emulates what GNU cp does */
		file->mode &= ~orig_umask;
	}
}


/* determine if a file in a different filesstem should be skipped
   when one_file_system is set. We bascally only want to include
   the mount points - but they can be hard to find! */
static int skip_filesystem(char *fname, STRUCT_STAT * st)
{
	STRUCT_STAT st2;
	char *p = strrchr(fname, '/');

	/* skip all but directories */
	if (!S_ISDIR(st->st_mode))
		return 1;

	/* if its not a subdirectory then allow */
	if (!p)
		return 0;

	*p = 0;
	if (link_stat(fname, &st2)) {
		*p = '/';
		return 0;
	}
	*p = '/';

	return (st2.st_dev != filesystem_dev);
}

#define STRDUP(ap, p)	(ap ? string_area_strdup(ap, p) : strdup(p))
/* IRIX cc cares that the operands to the ternary have the same type. */
#define MALLOC(ap, i)	(ap ? (void*) string_area_malloc(ap, i) : malloc(i))

/**
 * Create a file_struct for a named file by reading its stat()
 * information and performing extensive checks against global
 * options.
 *
 * @return the new file, or NULL if there was an error or this file
 * should be excluded.
 *
 * @todo There is a small optimization opportunity here to avoid
 * stat()ing the file in some circumstances, which has a certain cost.
 * We are called immediately after doing readdir(), and so we may
 * already know the d_type of the file.  We could for example avoid
 * statting directories if we're not recursing, but this is not a very
 * important case.  Some systems may not have d_type.
 **/
struct file_struct *make_file(char *fname, struct string_area **ap,
			      int exclude_level)
{
	struct file_struct *file;
	STRUCT_STAT st;
	char sum[SUM_LENGTH];
	char *p;
	char cleaned_name[MAXPATHLEN];
	char linkbuf[MAXPATHLEN];
	extern int module_id;

	strlcpy(cleaned_name, fname, MAXPATHLEN);
	cleaned_name[MAXPATHLEN - 1] = 0;
	clean_fname(cleaned_name);
	if (sanitize_paths) {
		sanitize_path(cleaned_name, NULL);
	}
	fname = cleaned_name;

	memset(sum, 0, SUM_LENGTH);

	if (readlink_stat(fname, &st, linkbuf) != 0) {
		int save_errno = errno;
		if (errno == ENOENT && exclude_level != NO_EXCLUDES) {
			/* either symlink pointing nowhere or file that
			 * was removed during rsync run; see if excluded
			 * before reporting an error */
			if (check_exclude_file(fname, 0, exclude_level)) {
				/* file is excluded anyway, ignore silently */
				return NULL;
			}
		}
		io_error |= IOERR_GENERAL;
		rprintf(FERROR, "readlink %s failed: %s\n",
			full_fname(fname), strerror(save_errno));
		return NULL;
	}

	/* backup.c calls us with exclude_level set to NO_EXCLUDES. */
	if (exclude_level == NO_EXCLUDES)
		goto skip_excludes;

	if (S_ISDIR(st.st_mode) && !recurse && !files_from) {
		rprintf(FINFO, "skipping directory %s\n", fname);
		return NULL;
	}

	if (one_file_system && st.st_dev != filesystem_dev) {
		if (skip_filesystem(fname, &st))
			return NULL;
	}

	if (check_exclude_file(fname, S_ISDIR(st.st_mode) != 0, exclude_level))
		return NULL;

	if (lp_ignore_nonreadable(module_id) && access(fname, R_OK) != 0)
		return NULL;

      skip_excludes:

	if (verbose > 2)
		rprintf(FINFO, "make_file(%s,*,%d)\n", fname, exclude_level);

	file = new(struct file_struct);
	if (!file)
		out_of_memory("make_file");
	memset((char *) file, 0, sizeof(*file));

	if ((p = strrchr(fname, '/'))) {
		static char *lastdir;
		*p = 0;
		if (lastdir && strcmp(fname, lastdir) == 0) {
			file->dirname = lastdir;
		} else {
			file->dirname = strdup(fname);
			lastdir = file->dirname;
		}
		file->basename = STRDUP(ap, p + 1);
		*p = '/';
	} else {
		file->dirname = NULL;
		file->basename = STRDUP(ap, fname);
	}

	file->modtime = st.st_mtime;
	file->length = st.st_size;
	file->mode = st.st_mode;
	file->uid = st.st_uid;
	file->gid = st.st_gid;
	file->dev = st.st_dev;
	file->inode = st.st_ino;
#ifdef HAVE_STRUCT_STAT_ST_RDEV
	file->rdev = st.st_rdev;
#endif

#if SUPPORT_LINKS
	if (S_ISLNK(st.st_mode)) {
		file->link = STRDUP(ap, linkbuf);
	}
#endif

	if (always_checksum) {
		file->sum = (char *) MALLOC(ap, MD4_SUM_LENGTH);
		if (!file->sum)
			out_of_memory("md4 sum");
		/* drat. we have to provide a null checksum for non-regular
		   files in order to be compatible with earlier versions
		   of rsync */
		if (S_ISREG(st.st_mode)) {
			file_checksum(fname, file->sum, st.st_size);
		} else {
			memset(file->sum, 0, MD4_SUM_LENGTH);
		}
	}

	if (flist_dir) {
		static char *lastdir;
		if (lastdir && strcmp(lastdir, flist_dir) == 0) {
			file->basedir = lastdir;
		} else {
			file->basedir = strdup(flist_dir);
			lastdir = file->basedir;
		}
	} else {
		file->basedir = NULL;
	}

	if (!S_ISDIR(st.st_mode))
		stats.total_size += st.st_size;

	return file;
}



void send_file_name(int f, struct file_list *flist, char *fname,
		    int recursive, unsigned base_flags)
{
	struct file_struct *file;
	extern int delete_excluded;

	/* f is set to -1 when calculating deletion file list */
	file = make_file(fname, &flist->string_area,
			 f == -1 && delete_excluded? SERVER_EXCLUDES
						   : ALL_EXCLUDES);

	if (!file)
		return;

	maybe_emit_filelist_progress(flist);

	flist_expand(flist);

	if (write_batch)	/*  dw  */
		file->flags = FLAG_DELETE;

	if (file->basename[0]) {
		flist->files[flist->count++] = file;
		send_file_entry(file, f, base_flags);
	}

	if (S_ISDIR(file->mode) && recursive) {
		struct exclude_struct **last_exclude_list =
		    local_exclude_list;
		send_directory(f, flist, f_name(file));
		local_exclude_list = last_exclude_list;
		return;
	}
}



static void send_directory(int f, struct file_list *flist, char *dir)
{
	DIR *d;
	struct dirent *di;
	char fname[MAXPATHLEN];
	int l;
	char *p;

	d = opendir(dir);
	if (!d) {
		io_error |= IOERR_GENERAL;
		rprintf(FERROR, "opendir %s failed: %s\n",
			full_fname(dir), strerror(errno));
		return;
	}

	strlcpy(fname, dir, MAXPATHLEN);
	l = strlen(fname);
	if (fname[l - 1] != '/') {
		if (l == MAXPATHLEN - 1) {
			io_error |= IOERR_GENERAL;
			rprintf(FERROR, "skipping long-named directory: %s\n",
				full_fname(fname));
			closedir(d);
			return;
		}
		strlcat(fname, "/", MAXPATHLEN);
		l++;
	}
	p = fname + strlen(fname);

	local_exclude_list = NULL;

	if (cvs_exclude) {
		if (strlen(fname) + strlen(".cvsignore") <= MAXPATHLEN - 1) {
			strcpy(p, ".cvsignore");
			add_exclude_file(&exclude_list,fname,MISSING_OK,ADD_EXCLUDE);
		} else {
			io_error |= IOERR_GENERAL;
			rprintf(FINFO,
				"cannot cvs-exclude in long-named directory %s\n",
				full_fname(fname));
		}
	}

	for (errno = 0, di = readdir(d); di; errno = 0, di = readdir(d)) {
		char *dname = d_name(di);
		if (dname[0] == '.' && (dname[1] == '\0'
		    || (dname[1] == '.' && dname[2] == '\0')))
			continue;
		strlcpy(p, dname, MAXPATHLEN - l);
		send_file_name(f, flist, fname, recurse, 0);
	}
#if defined(__WATCOMC__) && defined(MSDOS)
	/* Spurious ENOENT on the final readdir() of a subdirectory */
	if (errno == ENOENT) {
		errno = 0;
	}
#endif
	if (errno) {
		io_error |= IOERR_GENERAL;
		rprintf(FERROR, "readdir(%s): (%d) %s\n",
		    dir, errno, strerror(errno));
	}

	if (local_exclude_list)
		free_exclude_list(&local_exclude_list); /* Zeros pointer too */

	closedir(d);
}


/**
 * The delete_files() function in receiver.c sets f to -1 so that we just
 * construct the file list in memory without sending it over the wire.  It
 * also has the side-effect of ignoring user-excludes if delete_excluded
 * is set (so that the delete list includes user-excluded files).
 **/
struct file_list *send_file_list(int f, int argc, char *argv[])
{
	int l;
	STRUCT_STAT st;
	char *p, *dir, *olddir;
	char lastpath[MAXPATHLEN] = "";
	struct file_list *flist;
	int64 start_write;
	int use_ff_fd = 0;

	if (show_filelist_p() && f != -1)
		start_filelist_progress("building file list");

	start_write = stats.total_written;

	flist = flist_new();

	if (f != -1) {
		io_start_buffering(f);
		if (filesfrom_fd >= 0) {
			if (argv[0] && !push_dir(argv[0], 0)) {
				rprintf(FERROR, "push_dir %s failed: %s\n",
					full_fname(argv[0]), strerror(errno));
				exit_cleanup(RERR_FILESELECT);
			}
			use_ff_fd = 1;
		}
	}

	while (1) {
		char fname2[MAXPATHLEN];
		char *fname = fname2;

		if (use_ff_fd) {
			if (read_filesfrom_line(filesfrom_fd, fname) == 0)
				break;
			sanitize_path(fname, NULL);
		} else {
			if (argc-- == 0)
				break;
			strlcpy(fname, *argv++, MAXPATHLEN);
			if (sanitize_paths)
				sanitize_path(fname, NULL);
		}

		l = strlen(fname);
		if (fname[l - 1] == '/') {
			if (l == 2 && fname[0] == '.') {
				/* Turn "./" into just "." rather than "./." */
				fname[1] = '\0';
			} else {
				strlcat(fname, ".", MAXPATHLEN);
			}
		}

		if (link_stat(fname, &st) != 0) {
			if (f != -1) {
				io_error |= IOERR_GENERAL;
				rprintf(FERROR, "link_stat %s failed: %s\n",
					full_fname(fname), strerror(errno));
			}
			continue;
		}

		if (S_ISDIR(st.st_mode) && !recurse && !files_from) {
			rprintf(FINFO, "skipping directory %s\n", fname);
			continue;
		}

		dir = NULL;
		olddir = NULL;

		if (!relative_paths) {
			p = strrchr(fname, '/');
			if (p) {
				*p = 0;
				if (p == fname)
					dir = "/";
				else
					dir = fname;
				fname = p + 1;
			}
		} else if (f != -1 && implied_dirs && (p=strrchr(fname,'/')) && p != fname) {
			/* this ensures we send the intermediate directories,
			   thus getting their permissions right */
			char *lp = lastpath, *fn = fname, *slash = fname;
			*p = 0;
			/* Skip any initial directories in our path that we
			 * have in common with lastpath. */
			while (*fn && *lp == *fn) {
				if (*fn == '/')
					slash = fn;
				lp++, fn++;
			}
			*p = '/';
			if (fn != p || (*lp && *lp != '/')) {
				int copy_links_saved = copy_links;
				int recurse_saved = recurse;
				copy_links = copy_unsafe_links;
				/* set recurse to 1 to prevent make_file
				 * from ignoring directory, but still
				 * turn off the recursive parameter to
				 * send_file_name */
				recurse = 1;
				while ((slash = strchr(slash+1, '/')) != 0) {
					*slash = 0;
					send_file_name(f, flist, fname, 0, 0);
					*slash = '/';
				}
				copy_links = copy_links_saved;
				recurse = recurse_saved;
				*p = 0;
				strlcpy(lastpath, fname, sizeof lastpath);
				*p = '/';
			}
		}

		if (!*fname)
			fname = ".";

		if (dir && *dir) {
			olddir = push_dir(dir, 1);

			if (!olddir) {
				io_error |= IOERR_GENERAL;
				rprintf(FERROR, "push_dir %s failed: %s\n",
					full_fname(dir), strerror(errno));
				continue;
			}

			flist_dir = dir;
		}

		if (one_file_system)
			set_filesystem(fname);

		send_file_name(f, flist, fname, recurse, FLAG_DELETE);

		if (olddir != NULL) {
			flist_dir = NULL;
			if (pop_dir(olddir) != 0) {
				rprintf(FERROR, "pop_dir %s failed: %s\n",
					full_fname(dir), strerror(errno));
				exit_cleanup(RERR_FILESELECT);
			}
		}
	}

	if (f != -1) {
		send_file_entry(NULL, f, 0);
	}

	if (show_filelist_p() && f != -1) {
		finish_filelist_progress(flist);
	}

	clean_flist(flist, 0, 0);

	/* now send the uid/gid list. This was introduced in protocol
	   version 15 */
	if (f != -1) {
		send_uid_list(f);
	}

	/* send the io_error flag */
	if (f != -1) {
		extern int module_id;
		write_int(f, lp_ignore_errors(module_id) ? 0 : io_error);
	}

	if (f != -1) {
		io_end_buffering();
		stats.flist_size = stats.total_written - start_write;
		stats.num_files = flist->count;
		if (write_batch)	/*  dw  */
			write_batch_flist_info(flist->count, flist->files);
	}

	if (verbose > 2)
		rprintf(FINFO, "send_file_list done\n");

	return flist;
}


struct file_list *recv_file_list(int f)
{
	struct file_list *flist;
	unsigned char flags;
	int64 start_read;
	extern int list_only;

	if (show_filelist_p())
		start_filelist_progress("receiving file list");

	start_read = stats.total_read;

	flist = new(struct file_list);
	if (!flist)
		goto oom;

	flist->count = 0;
	flist->malloced = 1000;
	flist->files = new_array(struct file_struct *, flist->malloced);
	if (!flist->files)
		goto oom;


	for (flags = read_byte(f); flags; flags = read_byte(f)) {
		int i = flist->count;

		flist_expand(flist);

		receive_file_entry(&flist->files[i], flags, f);

		if (S_ISREG(flist->files[i]->mode))
			stats.total_size += flist->files[i]->length;

		flist->count++;

		maybe_emit_filelist_progress(flist);

		if (verbose > 2)
			rprintf(FINFO, "recv_file_name(%s)\n",
				f_name(flist->files[i]));
	}


	if (verbose > 2)
		rprintf(FINFO, "received %d names\n", flist->count);

	clean_flist(flist, relative_paths, 1);

	if (show_filelist_p()) {
		finish_filelist_progress(flist);
	}

	/* now recv the uid/gid list. This was introduced in protocol version 15 */
	if (f != -1) {
		recv_uid_list(f, flist);
	}

	/* recv the io_error flag */
	if (f != -1 && !read_batch) {	/* dw-added readbatch */
		extern int module_id;
		extern int ignore_errors;
		if (lp_ignore_errors(module_id) || ignore_errors) {
			read_int(f);
		} else {
			io_error |= read_int(f);
		}
	}

	if (list_only) {
		int i;
		for (i = 0; i < flist->count; i++) {
			list_file_entry(flist->files[i]);
		}
	}


	if (verbose > 2)
		rprintf(FINFO, "recv_file_list done\n");

	stats.flist_size = stats.total_read - start_read;
	stats.num_files = flist->count;

	return flist;

      oom:
	out_of_memory("recv_file_list");
	return NULL;		/* not reached */
}


/*
 * XXX: This is currently the hottest function while building the file
 * list, because building f_name()s every time is expensive.
 **/
int file_compare(struct file_struct **f1, struct file_struct **f2)
{
	if (!(*f1)->basename && !(*f2)->basename)
		return 0;
	if (!(*f1)->basename)
		return -1;
	if (!(*f2)->basename)
		return 1;
	if ((*f1)->dirname == (*f2)->dirname)
		return u_strcmp((*f1)->basename, (*f2)->basename);
	return u_strcmp(f_name(*f1), f_name(*f2));
}


int flist_find(struct file_list *flist, struct file_struct *f)
{
	int low = 0, high = flist->count - 1;

	while (high >= 0 && !flist->files[high]->basename) high--;

	if (high < 0)
		return -1;

	while (low != high) {
		int mid = (low + high) / 2;
		int ret =
		    file_compare(&flist->files[flist_up(flist, mid)], &f);
		if (ret == 0)
			return flist_up(flist, mid);
		if (ret > 0) {
			high = mid;
		} else {
			low = mid + 1;
		}
	}

	if (file_compare(&flist->files[flist_up(flist, low)], &f) == 0)
		return flist_up(flist, low);
	return -1;
}


/*
 * free up one file
 */
void free_file(struct file_struct *file)
{
	if (!file)
		return;
	if (file->basename)
		free(file->basename);
	if (file->link)
		free(file->link);
	if (file->sum)
		free(file->sum);
	*file = null_file;
}


/*
 * allocate a new file list
 */
struct file_list *flist_new(void)
{
	struct file_list *flist;

	flist = new(struct file_list);
	if (!flist)
		out_of_memory("send_file_list");

	flist->count = 0;
	flist->malloced = 0;
	flist->files = NULL;

#if ARENA_SIZE > 0
	flist->string_area = string_area_new(0);
#else
	flist->string_area = NULL;
#endif
	return flist;
}

/*
 * free up all elements in a flist
 */
void flist_free(struct file_list *flist)
{
	int i;
	for (i = 1; i < flist->count; i++) {
		if (!flist->string_area)
			free_file(flist->files[i]);
		free(flist->files[i]);
	}
	/* FIXME: I don't think we generally need to blank the flist
	 * since it's about to be freed.  This will just cause more
	 * memory traffic.  If you want a freed-memory debugger, you
	 * know where to get it. */
	memset((char *) flist->files, 0,
	       sizeof(flist->files[0]) * flist->count);
	free(flist->files);
	if (flist->string_area)
		string_area_free(flist->string_area);
	memset((char *) flist, 0, sizeof(*flist));
	free(flist);
}


/*
 * This routine ensures we don't have any duplicate names in our file list.
 * duplicate names can cause corruption because of the pipelining
 */
static void clean_flist(struct file_list *flist, int strip_root, int no_dups)
{
	int i, prev_i = 0;
	char *name, *prev_name = NULL;

	if (!flist || flist->count == 0)
		return;

	qsort(flist->files, flist->count,
	      sizeof(flist->files[0]), (int (*)()) file_compare);

	for (i = no_dups? 0 : flist->count; i < flist->count; i++) {
		if (flist->files[i]->basename) {
			prev_i = i;
			prev_name = f_name(flist->files[i]);
			break;
		}
	}
	while (++i < flist->count) {
		if (!flist->files[i]->basename)
			continue;
		name = f_name(flist->files[i]);
		if (strcmp(name, prev_name) == 0) {
			if (verbose > 1 && !am_server) {
				rprintf(FINFO,
					"removing duplicate name %s from file list %d\n",
					name, i);
			}
			/* Make sure that if we unduplicate '.', that we don't
			 * lose track of a user-specified starting point (or
			 * else deletions will mysteriously fail with -R). */
			if (flist->files[i]->flags & FLAG_DELETE)
				flist->files[prev_i]->flags |= FLAG_DELETE;
			/* it's not great that the flist knows the semantics of
			 * the file memory usage, but i'd rather not add a flag
			 * byte to that struct.
			 * XXX can i use a bit in the flags field? */
			if (flist->string_area)
				flist->files[i][0] = null_file;
			else
				free_file(flist->files[i]);
		}
		else
			prev_i = i;
		/* We set prev_name every iteration to avoid it becoming
		 * invalid when names[][] in f_name() wraps around. */
		prev_name = name;
	}

	if (strip_root) {
		/* we need to strip off the root directory in the case
		   of relative paths, but this must be done _after_
		   the sorting phase */
		for (i = 0; i < flist->count; i++) {
			if (flist->files[i]->dirname &&
			    flist->files[i]->dirname[0] == '/') {
				memmove(&flist->files[i]->dirname[0],
					&flist->files[i]->dirname[1],
					strlen(flist->files[i]->dirname));
			}

			if (flist->files[i]->dirname &&
			    !flist->files[i]->dirname[0]) {
				flist->files[i]->dirname = NULL;
			}
		}
	}

	if (verbose <= 3)
		return;

	for (i = 0; i < flist->count; i++) {
		rprintf(FINFO, "[%d] i=%d %s %s mode=0%o len=%.0f\n",
			(int) getpid(), i,
			NS(flist->files[i]->dirname),
			NS(flist->files[i]->basename),
			(int) flist->files[i]->mode,
			(double) flist->files[i]->length);
	}
}


/*
 * return the full filename of a flist entry
 *
 * This function is too expensive at the moment, because it copies
 * strings when often we only want to compare them.  In any case,
 * using strlcat is silly because it will walk the string repeatedly.
 */
char *f_name(struct file_struct *f)
{
	static char names[10][MAXPATHLEN];
	static int n;
	char *p = names[n];

	if (!f || !f->basename)
		return NULL;

	n = (n + 1) % 10;

	if (f->dirname) {
		int off;

		off = strlcpy(p, f->dirname, MAXPATHLEN);
		off += strlcpy(p + off, "/", MAXPATHLEN - off);
		off += strlcpy(p + off, f->basename, MAXPATHLEN - off);
	} else {
		strlcpy(p, f->basename, MAXPATHLEN);
	}

	return p;
}
