// See LICENSE file for copyright and license details.
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include "draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char POINTERSIZE = sizeof( int * );
#define BUFLEN 6
#define STRBUFLEN 25

#define DIWK_KEY_NOTHING 0
#define DIWK_RET_DISC    1
#define DIWK_RET_SAVE    2

Window win;
DC *dc;
int screen;
int dw, lh, lines; // diwk width, line height, number of lines
unsigned long col[ColLast], bw[ColLast];
XEvent e;
XIC xic;

char type;

char *string;
int *ls;
int sl;
int curs;
int view_top;

void updls( int _len, int _start );
int cx( int _curs );
int cy( int _curs );

char *diwk_text_prompt( int _lines );
char *diwk_radio_button( const char **_str, int _n );

void diwk_init( );
void diwk_clean( );
void diwk_create_window( char _lines );

char utf8hnd( unsigned char _c );

void erhnd( const char *_str ) {
	printf( "%s\n", _str );
	exit( EXIT_FAILURE );
}
