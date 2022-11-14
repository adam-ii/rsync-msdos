/* 
   Copyright (c) 2002 Andy Wightman <andy@2net.co.uk>

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

/////////////// various emulated unix declares for DOS/WINDOWS /////////////////////

#if defined(__BORLANDC__) || defined(__WATCOMC__)
#include	<sys/wtime.h>
#include	<sys/wtypes.h>
#include	<sys/socket.h>
#include	<sys/so_ioctl.h>
#include	<process.h>
// syslog.h ////////////////////////////////////////////////////////////////
#define syslog(p1, p2, p3)
#define openlog(p1, p2)
#define LOG_PID 0
#define LOG_WARNING 0
#define LOG_INFO 0
#if defined(__BORLANDC__)
// unistd.h ////////////////////////////////////////////////////////////////
#define R_OK	0x02
// sys/stat.h ////////////////////////////////////////////////////////////////
#define	S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define	S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)

#define	_IFMT		0170000	/* type of file */
#define	_IFDIR		0040000	/* directory */
#define	_IFCHR		0020000	/* character special */
#define	_IFBLK		0060000	/* block special */
#define	_IFREG		0100000	/* regular */
#define	_IFLNK		0120000	/* symbolic link */
#define	_IFSOCK		0140000	/* socket */
#define	_IFIFO		0010000	/* fifo */

#define _S_IFMT		_IFMT
#define _S_IFDIR	_IFDIR
#define _S_IFCHR	_IFCHR
#define _S_IFIFO	_IFIFO
#define _S_IFREG	_IFREG
#define _S_IREAD	0000400
#define _S_IWRITE	0000200
#define _S_IEXEC	0000100

#define _S_IFBLK		0x1000
#endif
// grp.h ////////////////////////////////////////////////////////////////
struct group {
  gid_t		gr_gid;
  char **   gr_mem;
  char *	gr_name;
};
struct group *	getgrgid(gid_t _gid);
struct group *	getgrnam(const char *_name);
// pwd.h ////////////////////////////////////////////////////////////////
struct passwd {
  char *	pw_name;
  uid_t		pw_uid;
  gid_t		pw_gid;
  char *	pw_dir;
  char *	pw_shell;
};
struct passwd *	getpwuid(uid_t _uid);
struct passwd *	getpwnam(const char *_name);

////////////////////////////------------------////////////////////////////////////
#else // ~__BORLANDC__
////////////////////////////------------------////////////////////////////////////

#pragma warning( disable : 4115 )  // Disable M/S 'named type definition in parentheses
#pragma warning( disable : 4131 )  // Disable M/S 'uses old-style declarator
#pragma warning( disable : 4201 )  // Disable M/S 'nameless struct warning messages
#pragma warning( disable : 4214 )  // Disable M/S 'bit field types other than int
#pragma warning( disable : 4514 )  // Disable M/S 'unref inline function
#include	<winsock2.h>
#pragma warning( default : 4201 )
#pragma warning( default : 4214 )
#include	<direct.h>
#include	<sys\utime.h>
// syslog.h ////////////////////////////////////////////////////////////////
#ifdef _WINDOWS
void	__cdecl logPrintf(char *f, ...);
void	exitToCaller(int code);
#endif
#define syslog(p1, p2, p3)
#define openlog(p1, p2)
#define getpid() 0
#define LOG_PID 0
#define LOG_WARNING 0
#define LOG_INFO 0
// unistd.h ////////////////////////////////////////////////////////////////
#define R_OK	0x02
// sys/stat.h ////////////////////////////////////////////////////////////////
#define _S_IFBLK		0x1000
// dirent.h ////////////////////////////////////////////////////////////////
struct dirent
{
    char        d_name[_MAX_FNAME];
};
typedef struct
{
    struct dirent	_d_dirent;
    char			*_d_dirname;
    char			_d_first;
	WIN32_FIND_DATA	sFindFileData;
	HANDLE			*pSHandle;
} DIR;

int				closedir(DIR *dirp);
DIR *			opendir(const char *_dirname);
struct dirent	*readdir(DIR *_dirp);
void			rewinddir(DIR *_dirp);
// grp.h ////////////////////////////////////////////////////////////////
struct group {
  gid_t		gr_gid;
  char **   gr_mem;
  char *	gr_name;
};

struct group *	getgrgid(gid_t _gid);
struct group *	getgrnam(const char *_name);
// pwd.h ////////////////////////////////////////////////////////////////
struct passwd {
  char *	pw_name;
  uid_t		pw_uid;
  gid_t		pw_gid;
  char *	pw_dir;
  char *	pw_shell;
};
  
struct passwd *	getpwuid(uid_t _uid);
struct passwd *	getpwnam(const char *_name);
// netinet\in.h ////////////////////////////////////////////////////////////////
// extension to winsock2.h
#define	IN_EXPERIMENTAL(a)	((((long int) (a)) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(a)		((((long int) (a)) & 0xf0000000) == 0xf0000000)

/* Network number for local host loopback. */
#define	IN_LOOPBACKNET		127

//from cygwin\usr\include\sys\errno.h
#define EAFNOSUPPORT 106 /* Address family not supported by protocol family */

// extension for winsock2.h
struct timezone {
       int tz_minuteswest;
       int tz_dsttime;
     };
int		gettimeofday(struct timeval *_tp, struct timezone *_tzp);
int		strcasecmp(const char *_s1, const char *_s2);
#ifndef HAVE_GETNAMEINFO
int		getnameinfo(const struct sockaddr *sa, size_t salen, char *host,
					size_t hostlen, char *serv, size_t servlen, int flags);
#endif
#endif

int getaddrinfo(const char *hostname, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *ai);

// should really be in proto.h
int recv_gen_files(int f_in,int f_out,struct file_list *flist,char *local_name);
void generate_files_phase1(int f,struct file_list *flist,char *local_name);
void generate_files_phase2(int f,struct file_list *flist,char *local_name,int i);
