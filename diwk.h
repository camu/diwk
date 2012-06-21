// See LICENSE file for copyright and license details.
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include "draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char POINTERSIZE = sizeof( int * );

#define DIWK_RET_DISC 0
#define DIWK_RET_SAVE 1
#define DIWK_UP       2
#define DIWK_DOWN     3

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

void diwk_init( );
void diwk_clean( );
void diwk_draw( );
int diwk_kb_ipret( );
void diwk_run( );



void erhnd( const char *_str ) {
	printf( "%s\n", _str );
	exit( EXIT_FAILURE );
}
