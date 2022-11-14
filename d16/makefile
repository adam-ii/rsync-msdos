# Makefile for rsync for 16bit DOS
#
# Copyright (c) 2002 Andy Wightman <andy@2net.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

# Andy Wightman. 14/02/2003
# This has been manually processed for Borland C/C++ 3.1 16 bit real mode DOS.
# Note that you must use the Borland make utility: any other will not do.
# It will probably work with other versions of Borland C, such as 4.52, but I
# have not tested it.
# this file and the objects are placed in the current target directory $(RT)\d16


MODEL		= l

RT              = c:\tmp\rsync
BCC31           = c:\bcc31
CLIB		= $(BCC31)\lib\C$(MODEL).LIB
MLIB		= $(BCC31)\lib\MATH$(MODEL).LIB
ELIB		= $(BCC31)\lib\EMU.LIB
CSTART		= $(BCC31)\lib\C0$(MODEL).OBJ
CINCLUDE	= $(BCC31)\include
CC		= $(BCC31)\bin\bcc.exe
LNK		= $(BCC31)\bin\tlink.exe
LIB             = $(BCC31)\bin\tlib.exe
RM              = del

PL		= $(RT)\popt
RL		= $(RT)\lib
ZL		= $(RT)\zlib
INC		= $(RT)\include
WS		= $(RT)\wattcp\inc

PLIB		= plib.lib
RLIB		= rlib.lib
WLIB		= $(RT)\wattcp\lib\wattcpbl.lib

# Compiler flags, -w display warnings, -d merge literal strings, -f- no floating point
# -O Optimise jumps, -X remove autodependency information in objects and libs
# -Z suppress register reloads, compile only, -3 386 processor
# -v include debugging information
CC_FLGS 	= -1- -w -d -f- -O -X -Z -c -m$(MODEL) -I$(CINCLUDE)
# allow 32bit, map file with segments, align segments on paragraph
LNK_FLGS	= /3 /m /s /A=16 /L$(BCC31)\lib

{$(PL)}.c.obj:
        $(CC) @&&!
$(CC_FLGS) -DHAVE_CONFIG_H -I$(RT) -I$(ZL) -I$(RL) -I$(WS) -I$(PL) $*.c
!
{$(RL)}.c.obj:
        $(CC) @&&!
$(CC_FLGS) -I$(RT) -I$(ZL) -I$(RL) -I$(WS) $*.c
!
{$(RT)}.c.obj:
        $(CC) @&&!
$(CC_FLGS) -I$(RT) -I$(ZL) -I$(RL) -I$(WS) -I$(PL) $*.c
!

LIB_INCLUDES	= $(RL)\addrinfo.h  $(RT)\confdefs.h $(RT)\config.h $(RT)\errcode.h $(RT)\dossup.h \
		$(RL)\fnmatch.h $(WS)\netinet\in.h $(WS)\arpa\inet.h $(RL)\mdfour.h \
		 $(RT)\proto.h $(RT)\rsync.h $(RT)\byteord.h $(RL)\permstr_.h

PLIB_INCLUDES	= $(PL)\findme.h $(PL)\popt.h $(PL)\poptint.h $(PL)\system.h $(RT)\confdefs.h $(RT)\config.h

INCLUDES	= $(ZL)\zconf.h $(ZL)\zlib.h $(PL)\popt.h $(LIB_INCLUDES)

LIBOBJ		= lcompat.obj fnmatch.obj getaddri.obj inet_pto.obj mdfour.obj snprintf.obj\
		inet_nto.obj getnamei.obj # permstr.obj 

PLIBOBJ		= findme.obj popt.obj poptcfg.obj popthelp.obj poptprse.obj

OBJS		= authent.obj batch.obj checksum.obj cleanup.obj clientsv.obj compat.obj dossup.obj \
		exclude.obj fileio.obj flist.obj io.obj loadparm.obj log.obj main.obj match.obj \
		options.obj rsync.obj sender.obj socket.obj syscall.obj token.obj uidlist.obj util.obj \
		generat.obj receiver.obj backup.obj hlink.obj clientna.obj params.obj 
		# access.obj connect.obj

all: rsync.exe

clean:
        -$(RM) *.obj
        -$(RM) *.map
        -$(RM) *.bak
        -$(RM) *.exe
        -$(RM) *.lib

# sometimes the &$(LIB) command is not repeated, only the first object gets included
# delete the target lib and run make again forces all the objects to be linked in!!
$(RLIB):	$(LIBOBJ)
	&$(LIB) $(RLIB) -+$**

$(PLIB):	$(PLIBOBJ)
	&$(LIB) $(PLIB) -+$**

rsync.exe: $(OBJS) $(RLIB) $(PLIB) $(WLIB)
	$(LNK) $(LNK_FLGS) @&&!
$(CSTART)+authent.obj+batch.obj+checksum.obj+cleanup.obj+clientsv.obj+compat.obj+\
dossup.obj+exclude.obj+fileio.obj+flist.obj+io.obj+loadparm.obj+log.obj+\
main.obj+match.obj+options.obj+rsync.obj+sender.obj+socket.obj+syscall.obj+token.obj+\
uidlist.obj+util.obj+generat.obj+receiver.obj+backup.obj+hlink.obj+clientna.obj+params.obj 
$@
rsync.map
$(RLIB)+$(WLIB)+$(PLIB)+$(CLIB)+$(MLIB)+$(ELIB)

!

lcompat.obj:		$(RL)\lcompat.c $(RLIB_INCLUDES)
fnmatch.obj:		$(RL)\fnmatch.c $(RLIB_INCLUDES)
getaddri.obj:		$(RL)\getaddri.c $(RLIB_INCLUDES)
inet_pto.obj:		$(RL)\inet_pto.c $(RLIB_INCLUDES)
mdfour.obj:		$(RL)\mdfour.c $(RLIB_INCLUDES)
snprintf.obj:		$(RL)\snprintf.c $(RLIB_INCLUDES)
permstr.obj:		$(RL)\permstr.c $(RLIB_INCLUDES)
inet_nto.obj:		$(RL)\inet_nto.c $(RLIB_INCLUDES)
getnamei.obj:		$(RL)\getnamei.c $(RLIB_INCLUDES)

findme.obj:		$(PL)\findme.c $(PLIB_INCLUDES)
popt.obj:		$(PL)\popt.c $(PLIB_INCLUDES)
poptcfg.obj:		$(PL)\poptcfg.c  $(PLIB_INCLUDES)
popthelp.obj:		$(PL)\popthelp.c $(PLIB_INCLUDES)
poptprse.obj:		$(PL)\poptprse.c $(PLIB_INCLUDES)

authent.obj:		$(RT)\authent.c $(INCLUDES) 
batch.obj:		$(RT)\batch.c $(INCLUDES) 
checksum.obj:		$(RT)\checksum.c $(INCLUDES) 
cleanup.obj:		$(RT)\cleanup.c $(INCLUDES) 
clientsv.obj:		$(RT)\clientsv.c $(INCLUDES) 
dossup.obj:		$(RT)\dossup.c $(INCLUDES) 
exclude.obj:		$(RT)\exclude.c $(INCLUDES) 
fileio.obj:		$(RT)\fileio.c $(INCLUDES) 
flist.obj:		$(RT)\flist.c $(INCLUDES) 
io.obj:			$(RT)\io.c $(INCLUDES) 
loadparm.obj:		$(RT)\loadparm.c $(INCLUDES) 
log.obj:		$(RT)\log.c $(INCLUDES) 
main.obj:		$(RT)\main.c $(INCLUDES) 
match.obj:		$(RT)\match.c $(INCLUDES) 
options.obj:		$(RT)\options.c $(INCLUDES) 
compat.obj:		$(RT)\compat.c $(INCLUDES) 
rsync.obj:		$(RT)\rsync.c $(INCLUDES) 
sender.obj:		$(RT)\sender.c $(INCLUDES) 
socket.obj:		$(RT)\socket.c $(INCLUDES) 
syscall.obj:		$(RT)\syscall.c $(INCLUDES) 
token.obj:		$(RT)\token.c $(INCLUDES) 
uidlist.obj:		$(RT)\uidlist.c $(INCLUDES) 
util.obj:		$(RT)\util.c $(INCLUDES) 
generat.obj:		$(RT)\generat.c $(INCLUDES) 
receiver.obj:		$(RT)\receiver.c $(INCLUDES) 
backup.obj:		$(RT)\backup.c $(INCLUDES) 
hlink.obj:		$(RT)\hlink.c $(INCLUDES) 
clientna.obj:		$(RT)\clientna.c $(INCLUDES) 
params.obj:		$(RT)\params.c $(INCLUDES) 
access.obj:		$(RT)\access.c $(INCLUDES) 
connection.obj:		$(RT)\connect.c $(INCLUDES) 

