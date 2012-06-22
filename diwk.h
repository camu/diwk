// See LICENSE file for copyright and license details.
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include "draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char POINTERSIZE = sizeof( int * );
#define BUFLEN 4
#define STRBUFLEN 25

#define DIWK_KEY_NOTHING 0
#define DIWK_RET_DISC    1
#define DIWK_RET_SAVE    2
#define DIWK_UP          3
#define DIWK_DOWN        4

Window win;
DC *dc;
int screen;
int dw, lh, lines; // diwk width, line height, number of lines
unsigned long col[ColLast], bw[ColLast];
XEvent e;
XIC xic;

char type;

//char buf[BUFLEN];
char *string;
unsigned long sl;
unsigned long view_up;
unsigned long curs[2];

char *diwk_text_prompt( char _lines );

char *diwk_pass_prompt( );

char *diwk_radio_button( const char **_str, int _n );
char **list;
char listcur;

void diwk_init( );
void diwk_clean( );
void diwk_draw( );
int diwk_kb_ipret( );
void diwk_create_window( );

char utf8hnd( unsigned char _c );

void erhnd( const char *_str ) {
	printf( "%s\n", _str );
	exit( EXIT_FAILURE );
}
