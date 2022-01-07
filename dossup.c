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

// This file contains helper routines to bridge between the
// unix functionality of rsync and the DOS platform.

#include "rsync.h"

#ifdef __BORLANDC__
/* we need this function because of the silly way in which duplicate
   entries are handled in the file lists - we can't change this
   without breaking existing versions */
int flist_up(struct file_list *flist, int i)
{
	while (!flist->files[i]->basename) i++;
	return i;
}
void set_nonblocking(int fd)
{
char	cmd = 1;

	ioctlsocket(fd, FIONBIO, &cmd);
}
void set_blocking(int fd)
{
char	cmd = 0;

	ioctlsocket(fd, FIONBIO, &cmd);
}

#else // ~__BORLANDC__

//////////////////////////////////////////////////////////////////////////
int		closedir(DIR *pDir)
{
	if (pDir->pSHandle != INVALID_HANDLE_VALUE)
		FindClose(pDir->pSHandle);
	free(pDir);
	return(0);
}
DIR		*opendir(const char *_dirname)
{
DIR *pDir;
char	pD[_MAX_FNAME];

	if ((pDir=(DIR *)malloc(sizeof(DIR))) == NULL)
		return(NULL);
	pDir->_d_dirname = (char *)_dirname;
	strcpy(pD, pDir->_d_dirname);
	if (pD[strlen(pD)-1] == '.')
		strcpy(&pD[strlen(pD)-1], "*.*");
	else
		strcat(pD, "/*.*");
	pDir->pSHandle = FindFirstFile(pD, &pDir->sFindFileData);
	if (pDir->pSHandle == INVALID_HANDLE_VALUE)
	{
		free(pDir);
		return(NULL);
	}
	pDir->_d_first = 1;
	return(pDir);
}
struct dirent	*readdir(DIR *pDir)
{

	if (pDir->pSHandle == INVALID_HANDLE_VALUE)
		return(NULL);
	if (pDir->_d_first == 0)
	{
		if (FindNextFile(pDir->pSHandle, &pDir->sFindFileData) == 0)
			return(NULL);
	}
	pDir->_d_first = 0;
	strcpy(&pDir->_d_dirent.d_name[0], &pDir->sFindFileData.cFileName[0]);
	return(&pDir->_d_dirent);
}
void	rewinddir(DIR *pDir)
{
char	pD[_MAX_FNAME];

	FindClose(pDir->pSHandle);
	strcpy(pD, pDir->_d_dirname);
	strcpy(&pD[strlen(pD)-1], "*.*");
	pDir->pSHandle = FindFirstFile(pD, &pDir->sFindFileData);
	pDir->_d_first = 1;
}
//////////////////////////////////////////////////////////////////////////
int		gettimeofday(struct timeval *_tp, struct timezone *_tzp)
{
	struct tm		*pTm;
	struct timeval	tv_tmp;

	if (_tp == NULL)
		_tp = &tv_tmp;

	time(&(_tp->tv_sec));
	if (_tzp != NULL)
	{
		pTm = localtime(&(_tp->tv_sec));
		_tzp->tz_minuteswest = - _timezone / 60;
		_tzp->tz_dsttime = pTm->tm_isdst;
	}
	return(0);
}
void set_nonblocking(int fd)
{
unsigned long dwLong = 1;

	if (ioctlsocket(fd, FIONBIO, &dwLong) == SOCKET_ERROR)
		rprintf(FERROR, "set_nonblocking() failed on socket %d", fd);
}
void set_blocking(int fd)
{
unsigned long dwLong = 0;

	if (ioctlsocket(fd, FIONBIO, &dwLong) == SOCKET_ERROR)
		rprintf(FERROR, "set_blocking() failed on socket %d", fd);
}
#endif

//////////////////////////////////////////////////////////////////////////
struct passwd *	getpwuid(uid_t _uid)
{
	return(NULL);
}
struct passwd *	getpwnam(const char *_name)
{
	return(NULL);
}
struct group *	getgrgid(gid_t _gid)
{
	return(NULL);
}
struct group *	getgrnam(const char *_name)
{
	return(NULL);
}
//////////////////////////////////////////////////////////////////////////
int do_lchown(const char *path, uid_t owner, gid_t group)
{
	return 0;
}
int do_lstat(const char *fname, STRUCT_STAT *st)
{
	return 0;
}
int	strcasecmp(const char *_s1, const char *_s2)
{
	return(strcmp(_s1, _s2));
}
//////////////////////////////////////////////////////////////////////////
// rather than including permstring.c in the lib (needs defines)
void permstring(char *perms, int mode)
{
	static const char *perm_map = "rwxrwxrwx";

	strcpy(perms, "----------");
}
pid_t	waitpid(pid_t pid, int *statptr, int options)
{
	return(0);
}
