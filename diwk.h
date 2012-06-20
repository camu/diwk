// See LICENSE file for copyright and license details.
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include "draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char POINTERSIZE = sizeof( int * );

Window win;
DC *dc;
int screen;
int dw, lh, lines; // diwk width, line height, number of lines
unsigned long col[ColLast], bw[ColLast];
XEvent e;

char *diwk_radio_button( const char **_str, int _n );

char **list = NULL;
char listcur;

char type;

void diwk_init( int _lines, const char *_label );
void diwk_clean( );
void diwk_draw( );
int diwk_kb_ipret( );
void diwk_run( );



void erhnd( const char *_str ) {
	printf( "%s\n", _str );
	exit( EXIT_FAILURE );
}
