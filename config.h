/* config.h Generated manually for Borland C/C++ 3.1 16bit.  */
#include "confdefs.h"

#undef ino_t
#undef HAVE_CONNECT
#undef HAVE_SHORT_INO_T
#undef HAVE_GETOPT_LONG
#undef REPLACE_INET_NTOA
#undef REPLACE_INET_ATON
#undef ENABLE_IPV6
#undef HAVE_SOCKADDR_LEN
#undef HAVE_SOCKETPAIR

/* Define to the type of elements in the array set by `getgroups'. Usually
   this is either `int' or `gid_t'. */
#undef GETGROUPS_T

/* Define if you have the <alloca.h> header file. */
#undef HAVE_ALLOCA_H

/* Define if you have the <arpa/inet.h> header file. */
#undef HAVE_ARPA_INET_H
#define HAVE_ARPA_INET_H 1

/* Define if you have the <arpa/nameser.h> header file. */
#undef HAVE_ARPA_NAMESER_H

/* Define if you have the `asprintf' function. */
#undef HAVE_ASPRINTF

/* */
#undef HAVE_BROKEN_READDIR

/* */
#undef HAVE_C99_VSNPRINTF

/* Define if you have the `chmod' function. */
#undef HAVE_CHMOD
#define HAVE_CHMOD 1

/* Define if you have the `chown' function. */
#undef HAVE_CHOWN

/* Define if you have the <compat.h> header file. */
#undef HAVE_COMPAT_H

/* */
#undef HAVE_CONNECT

/* Define if you have the <ctype.h> header file. */
#undef HAVE_CTYPE_H
#define HAVE_CTYPE_H 1

/* Define if you have the <dirent.h> header file, and it defines `DIR'. */
#undef HAVE_DIRENT_H
#define HAVE_DIRENT_H 1

/* */
#undef HAVE_ERRNO_DECL

/* Define if you have the `fchmod' function. */
#undef HAVE_FCHMOD

/* Define if you have the <fcntl.h> header file. */
#undef HAVE_FCNTL_H
#define HAVE_FCNTL_H 1

/* */
#undef HAVE_FNMATCH

/* Define if you have the `fstat' function. */
#undef HAVE_FSTAT

/* Define if you have the `getaddrinfo' function. */
#undef HAVE_GETADDRINFO

/* Define if you have the `getcwd' function. */
#undef HAVE_GETCWD
#define HAVE_GETCWD 1

/* Define if you have the `getnameinfo' function. */
#undef HAVE_GETNAMEINFO
#ifndef __BORLANDC__
#define HAVE_GETNAMEINFO 1
#endif

/* some systems need a version with 2 parameters */
#undef HAVE_GETTIMEOFDAY_TZ
#define HAVE_GETTIMEOFDAY_TZ 1

/* Define if you have the `glob' function. */
#undef HAVE_GLOB

/* Define if you have the <glob.h> header file. */
#undef HAVE_GLOB_H

/* Define if you have the <grp.h> header file. */
#undef HAVE_GRP_H

/* Define if you have the `inet_ntop' function. */
#undef HAVE_INET_NTOP

/* Define if you have the `inet_pton' function. */
#undef HAVE_INET_PTON

/* Define if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define if you have the `lchown' function. */
#undef HAVE_LCHOWN

/* Define if you have the `inet' library (-linet). */
#undef HAVE_LIBINET

/* Define if you have the `nsl' library (-lnsl). */
#undef HAVE_LIBNSL

/* Define if you have the `nsl_s' library (-lnsl_s). */
#undef HAVE_LIBNSL_S

/* Define if you have the `popt' library (-lpopt). */
#undef HAVE_LIBPOPT

/* Define if you have the `resolv' library (-lresolv). */
#undef HAVE_LIBRESOLV

/* Define if you have the `socket' library (-lsocket). */
#undef HAVE_LIBSOCKET

/* Define if you have the `link' function. */
#undef HAVE_LINK

/* */
#undef HAVE_LONGLONG

/* Define if you have the `mallinfo' function. */
#undef HAVE_MALLINFO

/* Define if you have the <malloc.h> header file. */
#undef HAVE_MALLOC_H
#define HAVE_MALLOC_H 1

/* Define if you have the <mcheck.h> header file. */
#undef HAVE_MCHECK_H

/* Define if you have the `memmove' function. */
#undef HAVE_MEMMOVE
#define HAVE_MEMMOVE 1

/* Define if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H
#define HAVE_MEMORY_H 1

/* Define if you have the `mknod' function. */
#undef HAVE_MKNOD

/* Define if you have the `mtrace' function. */
#undef HAVE_MTRACE

/* Define if you have the <ndir.h> header file, and it defines `DIR'. */
#undef HAVE_NDIR_H

/* Define if you have the <netdb.h> header file. */
#undef HAVE_NETDB_H

/* */
#undef HAVE_OFF64_T

/* Define if you have the `readlink' function. */
#undef HAVE_READLINK

/* remote shell is remsh not rsh */
#undef HAVE_REMSH

/* */
#undef HAVE_SECURE_MKSTEMP

/* Define if you have the `setgroups' function. */
#undef HAVE_SETGROUPS

/* Define if you have the `setsid' function. */
#undef HAVE_SETSID

/* */
#undef HAVE_SHORT_INO_T

/* Define if you have the `snprintf' function. */
#undef HAVE_SNPRINTF

/* Define if you have strct sockaddr_storage. */
#undef HAVE_SOCKADDR_STORAGE
#ifndef __BORLANDC__
#define HAVE_SOCKADDR_STORAGE 1
#endif

/* */
#undef HAVE_SOCKETPAIR

/* Define if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define if you have the <stdlib.h> header file. */
#undef HAVE_STDLIB_H
#define HAVE_STDLIB_H 1

/* Define if you have the `strcasecmp' function. */
#undef HAVE_STRCASECMP

/* Define if you have the `strchr' function. */
#undef HAVE_STRCHR
#define HAVE_STRCHR 1

/* Define if you have the `strdup' function. */
#undef HAVE_STRDUP
#define HAVE_STRDUP 1

/* Define if you have the `strerror' function. */
#undef HAVE_STRERROR
#ifdef __WATCOMC__
#define HAVE_STRERROR 1
#endif

/* Define if you have the `strftime' function. */
#undef HAVE_STRFTIME

/* Define if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define if you have the <string.h> header file. */
#undef HAVE_STRING_H
#define HAVE_STRING_H 1

/* Define if you have the `strlcat' function. */
#undef HAVE_STRLCAT

/* Define if you have the `strlcpy' function. */
#undef HAVE_STRLCPY

/* Define if you have the `strpbrk' function. */
#undef HAVE_STRPBRK

/* Define if `st_rdev' is member of `struct stat'. */
#undef HAVE_STRUCT_STAT_ST_RDEV

/* Define if you have the <sys/dir.h> header file, and it defines `DIR'. */
#undef HAVE_SYS_DIR_H

/* Define if you have the <sys/fcntl.h> header file. */
#undef HAVE_SYS_FCNTL_H

/* Define if you have the <sys/filio.h> header file. */
#undef HAVE_SYS_FILIO_H

/* Define if you have the <sys/ioctl.h> header file. */
#undef HAVE_SYS_IOCTL_H

/* Define if you have the <sys/mode.h> header file. */
#undef HAVE_SYS_MODE_H

/* Define if you have the <sys/ndir.h> header file, and it defines `DIR'. */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/param.h> header file. */
#undef HAVE_SYS_PARAM_H

/* Define if you have the <sys/select.h> header file. */
#undef HAVE_SYS_SELECT_H

/* Define if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/sysctl.h> header file. */
#undef HAVE_SYS_SYSCTL_H

/* Define if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <sys/unistd.h> header file. */
#undef HAVE_SYS_UNISTD_H

/* Define if you have the <sys/wait.h> header file. */
#undef HAVE_SYS_WAIT_H

/* Define if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* */
#undef HAVE_UNSIGNED_CHAR

/* */
#undef HAVE_UTIMBUF
#define HAVE_UTIMBUF 1

/* Define if you have the `utime' function. */
#undef HAVE_UTIME
#define HAVE_UTIME 1

/* Define if you have the `utimes' function. */
#undef HAVE_UTIMES

/* Define if you have the <utime.h> header file. */
#undef HAVE_UTIME_H
#define HAVE_UTIME_H 1

/* Define if `utime(file, NULL)' sets file's timestamp to the present. */
#undef HAVE_UTIME_NULL

/* Define if you have the `vsnprintf' function. */
#undef HAVE_VSNPRINTF

/* Define if you have the `wait4' function. */
#undef HAVE_WAIT4

/* Define if you have the `waitpid' function. */
#undef HAVE_WAITPID
#define HAVE_WAITPID 1

/* true if you have IPv6 */
#undef INET6

/* */
#undef REPLACE_INET_ATON

/* */
#undef REPLACE_INET_NTOA

/* Define as the return type of signal handlers (`int' or `void'). */
#undef RETSIGTYPE

/* location of rsync on remote machine */
#undef RSYNC_PATH
#define RSYNC_PATH "c:\\rsync.exe"

/* default -e command */
#undef RSYNC_RSH

/* rsync release version */
#undef RSYNC_VERSION
#define RSYNC_VERSION "2.5.5"

/* The size of a `int', as computed by sizeof. */
#undef SIZEOF_INT
#ifdef _M_IX86
#define SIZEOF_INT 2
#else
#define SIZEOF_INT 4
#endif

/* The size of a `long', as computed by sizeof. */
#undef SIZEOF_LONG
#define SIZEOF_LONG 4

/* The size of a `short', as computed by sizeof. */
#undef SIZEOF_SHORT
#define SIZEOF_SHORT 2

/* Define if you have the ANSI C header files. */
#undef STDC_HEADERS
#define STDC_HEADERS

/* Define if you can safely include both <sys/time.h> and <time.h>. */
#undef TIME_WITH_SYS_TIME

/* Define if your processor stores words with the most significant byte first
   (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define _GNU_SOURCE so that we get all necessary prototypes */
#undef _GNU_SOURCE

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to `int' if <sys/types.h> doesn't define. */
#ifdef __BORLANDC__
#undef gid_t
#define gid_t int
#endif

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
#undef inline
#define inline __inline

/* Define to `unsigned' if <sys/types.h> does not define. */
#undef ino_t

/* Define to `int' if <sys/types.h> does not define. */
#ifdef __BORLANDC__
#undef mode_t
#define mode_t int
#endif

#ifdef __BORLANDC__
#undef dev_t
#define dev_t long
#endif

/* Define to `long' if <sys/types.h> does not define. */
#undef off_t
#ifdef __BORLANDC__
#define off_t long
#endif

/* Define to `int' if <sys/types.h> does not define. */
#ifdef __BORLANDC__
#undef pid_t
#define pid_t int
#endif

/* Define to `unsigned' if <sys/types.h> does not define. */
#undef size_t

/* type to use in place of socklen_t if not defined */
#undef socklen_t
#define socklen_t int

/* Define to `int' if <sys/types.h> doesn't define. */
#ifdef __BORLANDC__
#undef uid_t
#define uid_t int
#endif
