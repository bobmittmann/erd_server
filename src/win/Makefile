
include ../mk/config.mk

LIB_STATIC = win

CFILES = winmain.c callbacks.c winserial.c sleep.c commlist.c  
OBJS = resource.o

include ../mk/lib.mk

$(OUTDIR)/resource.o: resource.rc Application.manifest Application.ico resource.h
	windres -I. -i $< -o $@

