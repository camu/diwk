X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

INCS = -I${X11INC}
LIBS = -L${X11LIB} -lX11

CPPFLAGS = -D_BSD_SOURCE -D_POSIX_C_SOURCE=2
CFLAGS   = -g -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS  = -g -s ${LIBS}

CC = cc

