# diwk – dynamic input widget kit
# See LICENSE for copyright and license details

include config.mk

SRC = diwk.c draw.c
OBJ = ${SRC:.c=.o}

all: options diwk

options:
	@echo -e "diwk build options\nCFLAGS\t=${CFLAGS}\nLDFLAGS\t${LDFLAGS}\nCC\t${CC}"

${OBJ}: diwk.h draw.h

diwk: diwk.o draw.o
	@echo CC -o $@
	@${CC} -o $@ diwk.o draw.o ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f diwk ${OBJ}
