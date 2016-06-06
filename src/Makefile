# File:     Makefile
# Author:   Robinson Mittmann (bobmittmann@gmail.com)
# Target:
# Comment:
# Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 

include mk/config.mk

PROG = trdp_proxy

CFILES = chat.c match.c trdp_proxy.c

# Compile ANSI build only if CHARSET=ANSI
ifeq (${CHARSET}, ANSI)
  CFLAGS= -O3 -std=c99 -D _WIN32_IE=0x0500 -D WINVER=0x500
else
  CFLAGS= -O3 -std=c99 -D UNICODE -D _UNICODE -D _WIN32_IE=0x0500 -D WINVER=0x500
endif

LDFLAGS = -s -Wl,--subsystem,windows

LIBDIRS = win

LIBS = pthread m win comctl32 gdi32
OBJS = win/resource.o

INCPATH = . include 

include mk/prog.mk

