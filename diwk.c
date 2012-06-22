// See LICENSE file for copyright and license details.
#include "diwk.h"
#include "config.h"

int main( int argc, char *argv[] ) {
	diwk_init( );

	printf( "%s\n", diwk_text_prompt( 6 ) );

//	printf( "%s\n", diwk_pass_prompt( ) );

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
	initfont( dc, NULL );
	lh = 18;
	dw = DisplayWidth( dc->dpy, screen );
	screen = DefaultScreen( dc->dpy );
}

void diwk_clean( ) {
	if( string ) free( string );
	if( list ) free( list );
	freedc( dc );
}

void diwk_create_window( ) {
	XSetWindowAttributes swa;
	swa.override_redirect = True;
	swa.background_pixel = getcolor( dc, "#ffffff" );
	swa.event_mask = ExposureMask | VisibilityChangeMask | KeyPressMask;
	win = XCreateWindow( dc->dpy, RootWindow( dc->dpy, screen ), 0, 0, dw, lh*lines, 0, DefaultDepth( dc->dpy, screen ), CopyFromParent, DefaultVisual( dc->dpy, screen ), CWOverrideRedirect | CWBackPixel | CWEventMask, &swa );
	XMapRaised( dc->dpy, win );
	resizedc( dc, dw, lh*lines );

	if( XGrabKeyboard( dc->dpy, DefaultRootWindow( dc->dpy ), True, GrabModeAsync, GrabModeAsync, CurrentTime ) != GrabSuccess )
		erhnd( "failed to grab kb" );

	XIM xim = XOpenIM( dc->dpy, NULL, NULL, NULL );
	xic = XCreateIC( xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, win, XNFocusWindow, win, NULL );
}

void diwk_draw( ) {
	dc->x = 0;
	dc->y = 0;
	dc->w = dw;
	dc->h = lh*lines;
	drawrect( dc, 0, 0, dw, lh*lines, True, BG( dc, bw ) );

	if( type == 0 ) {
		dc->x = 0;
		dc->y = 0;
		dc->w = 0;
		dc->h = lh;

		int i, j; for( i = j = 0; i < sl; i++ ) {
			if( string[i] == '\n' ) {
				char tmpstr[i-j+1];
				memcpy( tmpstr, &string[j], i-j+1 );
				tmpstr[i-j] = 0;
				dc->w = textw( dc, tmpstr );
				drawtext( dc, tmpstr, col );
				j = i+1;
				dc->y += lh;
			}
		}
		char tmpstr[i-j+1];
		memcpy( tmpstr, &string[j], i-j+1 );
		tmpstr[i-j] = 0;
		dc->w = textw( dc, tmpstr );
		drawtext( dc, tmpstr, col );

		dc->x = 0; dc->y = 0;
		drawrect( dc, textnw( dc, tmpstr, curs[0]+1 ), curs[1]*lh, 1, lh, True, FG( dc, col ) );
//		drawrect( dc, textw( dc, tmpstr )-textw( dc, &tmpstr[curs[0]] )+textw( dc, "" ), curs[1]*lh, 1, lh, True, FG( dc, col ) );

		mapdc( dc, win, dw, lh*lines );
	}

	if( type == 2 ) {
		dc->x = 0;
		dc->y = 0;
		dc->h = lh;

		drawrect( dc, 0, 0, 4, lh*lines, True, BG( dc, bw ) );
		drawrect( dc, 0, lh*listcur, 4, lh, True, FG( dc, col ) );
		int i; for( i = 0; i < lines; i++ ) {
			dc->y = i*dc->h;
			dc->w = textw( dc, list[i] );
			dc->x = 4;
			drawtext( dc, list[i], col );
		}

		mapdc( dc, win, dw, lh*lines );
	}
}

int diwk_kb_ipret( ) {
	if( type == 0 ) {
		char buf[BUFLEN];
		KeySym ks;
		int len = Xutf8LookupString( xic, &e.xkey, buf, BUFLEN, &ks, NULL );
		if( sl % STRBUFLEN > STRBUFLEN-BUFLEN-1 )
			string = realloc( string, sl+STRBUFLEN+(STRBUFLEN-(sl%STRBUFLEN))+1 );

		// mod1, usually alt
		if( e.xkey.state & Mod1Mask ) {
			switch( ks ) {
			case XK_Escape: return DIWK_RET_DISC;
			}
		}

		// no mod pressed
		int i, j, k; // maybe needed
		switch( ks ) {
		case XK_Escape: return DIWK_RET_SAVE;
// CHECK NOT TO MOVE OUT OF THE TEXT BOX
		case XK_Up:
			if( curs[1] > 0 ) curs[1]--;
			break;

		case XK_Down:
			curs[1]++;
			break;

		case XK_Left:
			if( string[curs[0]-1] != '\n' ) curs[0]--;
			else {
				curs[1]--;
				for( k = curs[0] = 0;; curs[0]++ ) {
					if( string[curs[0]] == '\n' ) k++;
					if( k == curs[1] ) break;
				} for( curs[0] = 0; string[curs[0]] != '\n'; curs[0]++ );
			} break;

		case XK_Right:
			if( string[curs[0]+1] != '\n' ) curs[0]++;
			else {
				curs[0] = 0;
				curs[1]++;
			} break;
				
		case XK_Return:
			curs[0] = 0; curs[1]++;
			buf[0] = '\n';
			len = 1;
			break;
		case XK_BackSpace:
			// this is very stupid when sl is HUUUUUUUUUUGE

			for( i = j = k = 0; i < sl; i++ ) {
				if( string[i] == '\n' ) k++;
				if( k == curs[1] ) {
					j++;
					if( j == curs[2] ) break;
				}
			} // now: i is the cursor position in char *string
printf( "%i\n", i );
			if( i == 0 ) return DIWK_KEY_NOTHING;
			for( j = 1; utf8hnd( string[i-j] ) == -1; j++ );

			if( string[i-j] == '\n' ) {
				curs[1]--;
				for( k = curs[0] = 0;; curs[0]++ ) {
					if( string[curs[0]] == '\n' ) k++;
					if( k == curs[1] ) break;
				} for( curs[0] = 0; string[curs[0]] != '\n'; curs[0]++ );
			} else curs[0] -= len;

			memmove( &string[i-j], &string[i], j );
			sl -= j;

			diwk_draw( );

			return DIWK_KEY_NOTHING;
		}

		if( ks != XK_Return ) curs[0] += len;
		memcpy( &string[sl], buf, BUFLEN );
		sl += len;

		diwk_draw( );

}
	if( type == 1 ) {
		char buf[BUFLEN];
		KeySym ks;
		int len = Xutf8LookupString( xic, &e.xkey, buf, BUFLEN, &ks, NULL );
		if( sl % STRBUFLEN > STRBUFLEN-BUFLEN-1 )
			string = realloc( string, sl+STRBUFLEN+(STRBUFLEN-(sl%STRBUFLEN))+1 );

		// mod1, usually alt
		if( e.xkey.state & Mod1Mask ) {
			switch( ks ) {
			case XK_Escape: return DIWK_RET_DISC;
			}
		}

		// no mod pressed
		switch( ks ) {
		case XK_Escape: return DIWK_RET_SAVE;
		}
		memcpy( &string[sl], buf, BUFLEN );
		sl += len;
	}
	if( type == 2 ) {
		KeySym ks = XLookupKeysym( &(e.xkey), 0 );

		// mod1, usually alt
		if( e.xkey.state & Mod1Mask ) {
			switch( ks ) {
			case XK_Escape: return DIWK_RET_DISC;
			}
		}

		// no mod pressed
		switch( ks ) {
		case XK_Escape: return DIWK_RET_SAVE;
		case XK_Up:
			if( listcur > 0 ) {
				listcur--;
				return DIWK_UP;
			} break;
		case XK_Down:
			if( listcur < lines-1 ) {
				listcur++;
				return DIWK_DOWN;
			} break;
		}
	}
	return -1;
}

char *diwk_text_prompt( char _lines ) {
	type = 0;
	lines = (_lines>0?_lines:1);
	diwk_create_window( );

	string = malloc( STRBUFLEN*BUFLEN+1 );
	string[0] = 0;
	sl = 0;
	view_up = 0;
	curs[0] = curs[1] = 0;

	diwk_draw( );

	for( ;; ) {
		XNextEvent( dc->dpy, &e );
		switch( e.type ) {
		case KeyPress:
			switch( diwk_kb_ipret( ) ) {
			case DIWK_RET_DISC:
				return "\0";
			case DIWK_RET_SAVE:
				return string;
			}
		}
	}
}

char *diwk_pass_prompt( ) {
	type = 1;
	lines = 1;
	diwk_create_window( );

	string = malloc( STRBUFLEN*BUFLEN+1 );
	sl = 0;

	dc->x = 0;
	dc->y = 0;
	dc->w = textw( dc, "passwd will not be echo'd" );
	dc->h = lh;
	drawtext( dc, "passwd will not be echo'd", col );
	mapdc( dc, win, dw, lh );

	for( ;; ) {
		XNextEvent( dc->dpy, &e );
		switch( e.type ) {
		case KeyPress:
			switch( diwk_kb_ipret( ) ) {
			case DIWK_RET_DISC:
				return "\0";
			case DIWK_RET_SAVE:
				return string;
			}
		}
	}
}

char *diwk_radio_button( const char **_str, int _n ) {
	type = 2;
	lines = _n;
	diwk_create_window( );

	listcur = 0;
	list = malloc( POINTERSIZE*_n );
	int i; for( i = 0; i < _n; i++ ) list[i] = _str[i];

	for( i = 0; i < lines; i++ ) {
		dc->y = i*dc->h;
		dc->w = textw( dc, list[i] );
		dc->x = 4;
		drawtext( dc, list[i], col );
	}
	mapdc( dc, win, dw, lh*lines );

	for( ;; ) {
		XNextEvent( dc->dpy, &e );
		switch( e.type ) {
		case KeyPress:
			switch( diwk_kb_ipret( ) ) {
			case DIWK_RET_DISC:
				return "\0";
			case DIWK_RET_SAVE:
				return list[listcur];

			case DIWK_UP:
				drawrect( dc, 0, lh*(listcur+1), 4, lh, True, BG( dc, bw ) );
				drawrect( dc, 0, lh*listcur, 4, lh, True, FG( dc, col ) );
				mapdc( dc, win, dw, lh*lines );
				break;
			case DIWK_DOWN:
				drawrect( dc, 0, lh*(listcur-1), 4, lh, True, BG( dc, bw ) );
				drawrect( dc, 0, lh*listcur, 4, lh, True, FG( dc, col ) );
				mapdc( dc, win, dw, lh*lines );
				break;
			}
		}
		diwk_draw( );
	}
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
