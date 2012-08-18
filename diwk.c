// See LICENSE file for copyright and license details.
#include "diwk.h"
#include "config.h"

int main( int argc, char *argv[] ) {
	diwk_init( );

	printf( "%s\n", diwk_text_prompt( 4 ) );

//	printf( "%s\n", diwk_text_prompt( 0 ) );

//	const char *herp[] = { "herp", "sherp", "derp" };
//	printf( "%s\n", diwk_radio_button( herp, 3 ) );

	diwk_clean( );

	return 0;
}

void diwk_init( ) {
	// gfx
	dc = initdc( );
	col[ColBG] = getcolor( dc, normbg );
	col[ColFG] = getcolor( dc, normfg );
	bw[ColBG] = getcolor( dc, bwbg );
	bw[ColFG] = getcolor( dc, bwfg );
	initfont( dc, "-*-terminus-medium-r-*-*-12-*-*-*-*-*-*-*" );
	lh = 18;
	dw = DisplayWidth( dc->dpy, screen );
	screen = DefaultScreen( dc->dpy );
}

void diwk_clean( ) {
	if( ls ) free( ls );
	if( string ) free( string );
	freedc( dc );
}

void diwk_create_window( char _lines ) {
	XSetWindowAttributes swa;
	swa.override_redirect = True;
	swa.background_pixel = getcolor( dc, "#ffffff" );
	swa.event_mask = ExposureMask | VisibilityChangeMask | KeyPressMask;
	win = XCreateWindow( dc->dpy, RootWindow( dc->dpy, screen ), 0, 0, dw, lh*_lines, 0, DefaultDepth( dc->dpy, screen ), CopyFromParent, DefaultVisual( dc->dpy, 
screen ), CWOverrideRedirect | CWBackPixel | CWEventMask, &swa );
	XMapRaised( dc->dpy, win );
	resizedc( dc, dw, lh*_lines );

	if( XGrabKeyboard( dc->dpy, DefaultRootWindow( dc->dpy ), True, GrabModeAsync, GrabModeAsync, CurrentTime ) != GrabSuccess )
		erhnd( "failed to grab kb" );

	XIM xim = XOpenIM( dc->dpy, NULL, NULL, NULL );
	xic = XCreateIC( xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, win, XNFocusWindow, win, NULL );
}

void draw( int _lines ) {
	for( ; cy( curs ) < view_top; view_top-- );
	for( ; cy( curs ) >= view_top+_lines; view_top++ );

	dc->x = 0; dc->y = 0; dc->w = dw; dc->h = lh*_lines;
	drawrect( dc, 0, 0, dw, lh*_lines, True, BG( dc, bw ) );
	dc->w = 0; dc->h = lh;
	int y = 0;
	int i; for( i = 1; i < ls[0]; i++ ) {
		if( i-1 >= view_top && i-1 < view_top+_lines ) {
			int j = (ls[i+1]-ls[i] < 1 ? 1 : ls[i+1]-ls[i] );
			char tmpstr[j];
			memcpy( tmpstr, &string[ls[i]], j );
			if( tmpstr[j-1] == '\n' ) tmpstr[j-1] = 0;
			else tmpstr[j] = 0;
			dc->w = textw( dc, tmpstr );
			dc->y = y;
			drawtext( dc, tmpstr, col );
			y += lh;
			dc->y = 0;
			if( i-1 == cy( curs ) ) {
				int k; for( k = 0; k < j; k++ ) tmpstr[k] = 'x';
				drawrect( dc, textnw( dc, tmpstr, cx( curs )+1 ), (i-1-view_top)*lh, 1, lh, True, FG( dc, col ) );
			}
		}
	}

	mapdc( dc, win, dw, lh*_lines );
}

// vaatii pientä hiomista kun hypätään kohtaan jossa ei ole mitään (mutta silti riville)
void cvm( Bool _up ) {
	int i = cx( curs ), j = curs;
	for( ; curs > 0 && string[curs] != '\n'; curs-- )
		if( utf8hnd( string[curs] ) == -1 ) i--;

	if( _up ) {
		for( curs--; curs > 0 && string[curs-1] != '\n'; curs-- );
		for( ; cx( curs ) < i; curs++ );
	} else {
		curs = j;
		for( ; string[curs] != '\n'; curs++ );
		for( curs++; curs < sl && cx( curs ) < i; curs++ );
	}
}

char *diwk_text_prompt( int _lines ) {
	diwk_create_window( _lines );
	string = malloc( STRBUFLEN*BUFLEN+1 );
	string[0] = 0;
	ls = malloc( 3*sizeof(int) );
	ls[0] = 2; ls[1] = ls[2] = 0;
	sl = curs = 0;

	draw( _lines );

	char buf[BUFLEN];
	KeySym ks;
	int len, i;
	for( ;; ) {
		XNextEvent( dc->dpy, &e );
		switch( e.type ) {
		case KeyPress:
			len = Xutf8LookupString( xic, &e.xkey, buf, BUFLEN, &ks, NULL );
			if( sl % STRBUFLEN > STRBUFLEN-BUFLEN-1 )
				string = realloc( string, sl+STRBUFLEN+(STRBUFLEN-(sl%STRBUFLEN))+1 );

			// mod1, usually alt
			if( e.xkey.state & Mod1Mask ) {
				switch( ks ) {
				case XK_Escape: return "\0";
				}
			}

			// ctrl
			else if( e.xkey.state & ControlMask ) {
				switch( ks ) {
				case XK_j:
					if( view_top > 0 ) view_top--;
					break;
				case XK_k:
					if( view_top+_lines < ls[0]-1 ) view_top++;
					break;
				}
			}

			// no mod pressed
			else {
				switch( ks ) {
				case XK_Return:
					buf[0] = '\n';
					len = 1; ls[0]++;
					ls = realloc( ls, sizeof(int)*(ls[0]+1) );
					ls[ls[0]] = curs;
					ls[ls[0]-1]++;
				default:
					memmove( &string[curs+len], &string[curs], sl-curs );
					memcpy( &string[curs], buf, len );
					sl += len; curs += len;
					updls( len, cy( curs )+1 );
					moils( );
					string[sl] = 0;
					break;

				case XK_BackSpace:
					for( len = 1; utf8hnd( string[curs-len] ) == -1; len++ );
					if( curs < len ) break;

					if( string[curs-len] == '\n' ) {
						for( i = cy( curs )+1; i < ls[0]; i++ )
							ls[i] = ls[i+1];
						ls[0]--;
					}

					memmove( &string[curs-len], &string[curs], sl-curs );
					curs -= len; sl -= len;
					string[sl] = 0;
					updls( -len, cy( curs )+1 );
					break;

				case XK_Left: if( curs == 0 ) break;
					for( len = 1; utf8hnd( string[curs-len] ) == -1; len++ );
					if( curs >= len ) curs -= len;
					break;
				case XK_Right:
					for( len = 0; utf8hnd( string[curs+len] ) == -1; len++ );
					if( curs+len < sl ) curs += len+1;
					break;
				case XK_Up:
					if( cy( curs ) > 0 ) cvm( True );
					break;
				case XK_Down:
					if( cy( curs ) < ls[0]-2 ) cvm( False );
					break;

				case XK_Escape: return string;
				}
			}

			draw( _lines );
		}
	}
}

char *diwk_radio_button( const char **_str, int _n ) {
	diwk_create_window( _n );

	char listcur = 0;

	dc->x = 0; dc->y = 0; dc->h = lh;
	drawrect( dc, 0, lh*(listcur+1), 4, lh, True, BG( dc, bw ) );
	drawrect( dc, 0, lh*listcur, 4, lh, True, FG( dc, col ) );
	int i; for( i = 0; i < _n; i++ ) {
		dc->y = i*dc->h;
		dc->w = textw( dc, _str[i] );
		dc->x = 4;
		drawtext( dc, _str[i], col );
	}
	mapdc( dc, win, dw, lh*_n );


	KeySym ks;
	for( ;; ) {
		XNextEvent( dc->dpy, &e );
		switch( e.type ) {
		case KeyPress:
			ks = XLookupKeysym( &e.xkey, 0 );

			// mod1, usually alt
			if( e.xkey.state & Mod1Mask ) {
				switch( ks ) {
				case XK_Escape:
				return "\0";
				}
			}

			// no mod pressed
			else
			switch( ks ) {
			case XK_Escape:
				return (char *)_str[listcur];

			case XK_Up:
				if( listcur > 0 ) {
					listcur--;
					dc->x = 0; dc->y = 0;
					drawrect( dc, 0, lh*(listcur+1), 4, lh, True, BG( dc, bw ) );
					drawrect( dc, 0, lh*listcur, 4, lh, True, FG( dc, col ) );
					mapdc( dc, win, dw, lh*_n );
				} break;
			case XK_Down:
				if( listcur < _n-1 ) {
					listcur++;
					dc->x = 0; dc->y = 0;
					drawrect( dc, 0, lh*(listcur-1), 4, lh, True, BG( dc, bw ) );
					drawrect( dc, 0, lh*listcur, 4, lh, True, FG( dc, col ) );
					mapdc( dc, win, dw, lh*_n );
				} break;
			}
		}
	}
}

void moils( ) {
	int i, j; for( i = 1; i < ls[0]; i++ ) {
		if( ls[i] > ls[i+1] ) {
			j = ls[i];
			ls[i] = ls[i+1];
			ls[i+1] = j;
		}
	}
}

void updls( int _len, int _start ) {
	for( _start++; _start <= ls[0]; _start++ )
		ls[_start] += _len;
}

int cx( int _curs ) {
	int i, j;
	for( i = j = 0; i < curs; i++ ) {
		if( utf8hnd( string[i] ) != -1 ) j++;
		if( string[i] == '\n' ) j = 0;
	}
	return j;
}
int cy( int _curs ) {
	int i, j;
	for( i = j = 0; i < curs; i++ )
		if( string[i] == '\n' ) j++;
	return j;
}

char utf8hnd( unsigned char _c ) {
	/* -2 if _c is bad
	   -1 if shud check prev
	    0 if one-byte char
	    1 if two-byte char
	    2 if three-byte char
	    3 if .. and so on */

	if( _c < 0x80 ) return 0;
	else if( _c < 0xc0 ) return -1;
	else if( _c < 0xe0 ) return 1;
	else if( _c < 0xf0 ) return 2;
	else if( _c < 0xf8 ) return 3;
	else if( _c < 0xfc ) return 4;
	else if( _c < 0xfe ) return 5;
	else return -2;
}

