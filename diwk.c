// See LICENSE file for copyright and license details.
#include "diwk.h"
#include "config.h"

int main( int argc, char *argv[] ) {
	diwk_init( );

	printf( "%s\n", diwk_text_prompt( 4, True ) );

//	printf( "%s\n", diwk_text_prompt( 1, False ) );

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
	freedc( dc );
}

void diwk_create_window( char _lines ) {
	XSetWindowAttributes swa;
	swa.override_redirect = True;
	swa.background_pixel = getcolor( dc, "#ffffff" );
	swa.event_mask = ExposureMask | VisibilityChangeMask | KeyPressMask;
	win = XCreateWindow( dc->dpy, RootWindow( dc->dpy, screen ), 0, 0, dw, lh*_lines, 0, DefaultDepth( dc->dpy, screen ), CopyFromParent, DefaultVisual( dc->dpy, screen ), CWOverrideRedirect | CWBackPixel | CWEventMask, &swa );
	XMapRaised( dc->dpy, win );
	resizedc( dc, dw, lh*_lines );

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
		if( curs[1] < view_up ) diwk_view_add( -1 );
		if( curs[1] > view_up+lines-1 ) diwk_view_add( 1 );

		dc->w = 0;
		dc->h = lh;

		int i, j, k; for( i = j = k = 0; i < sl; i++ ) {
			if( string[i] == '\n' ) {
				if( k >= view_up && k <= view_up+lines ) {
					char tmpstr[i-j+1];
					memcpy( tmpstr, &string[j], i-j+1 );
					tmpstr[i-j] = 0;
					dc->w = textw( dc, tmpstr );
					drawtext( dc, tmpstr, col );
					dc->y += lh;
				} k++;
				j = i+1;
			}
		}
		char tmpstr[i-j+1];
		memcpy( tmpstr, &string[j], i-j+1 );
		tmpstr[i-j] = 0;
		dc->w = textw( dc, tmpstr );
		drawtext( dc, tmpstr, col );

		dc->x = 0; dc->y = 0;
		drawrect( dc, textnw( dc, tmpstr, curs[0]+1 ), (curs[1]-view_up)*lh, 1, lh, True, FG( dc, col ) );
//		drawrect( dc, textw( dc, tmpstr )-textw( dc, &tmpstr[curs[0]] )+textw( dc, "" ), curs[1]*lh, 1, lh, True, FG( dc, col ) );

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

		// ctrl
		if( e.xkey.state & ControlMask ) {
			switch( ks ) {
			case XK_p: diwk_view_add( -1 ); diwk_draw( ); return DIWK_KEY_NOTHING;
			case XK_n: diwk_view_add( 1 ); diwk_draw( ); return DIWK_KEY_NOTHING;
			}
		}

		// no mod pressed
		int i, j;
		switch( ks ) {
		case XK_Escape: return DIWK_RET_SAVE;

		case XK_Up:
			if( curs[1] > 0 ) curs[1]--;
			diwk_draw( );
			return DIWK_KEY_NOTHING;
		case XK_Down:
			if( !diwk_is_last_row( ) ) curs[1]++;
			else curs[0] = diwk_row_last_col( );
			diwk_draw( );
			return DIWK_KEY_NOTHING;
		case XK_Left:
			if( curs[0] > 0 ) curs[0]--;
			else {
				curs[1]--;
				curs[0] = diwk_row_last_col( );
			}
			diwk_draw( );
			return DIWK_KEY_NOTHING;
		case XK_Right:
			if( curs[0] != diwk_row_last_col( ) ) curs[0]++;
			else if( !diwk_is_last_row( ) ) {
				curs[0] = 0;
				curs[1]++;
			}
			diwk_draw( );
			return DIWK_KEY_NOTHING;
				
		case XK_Return:
			buf[0] = '\n';
			len = 1;
			break;
		case XK_BackSpace:
			i = diwk_str_curs_pos( );
			if( i == 0 ) return DIWK_KEY_NOTHING;
			for( j = 1; utf8hnd( string[i-j] ) == -1; j++ );

			if( string[i-j] == '\n' ) {
				curs[1]--;
				curs[0] = diwk_row_last_col( );
			} else curs[0] -= j;

			memmove( &string[i-j], &string[i], j );
			sl -= j;

			diwk_draw( );

			return DIWK_KEY_NOTHING;
		}

		i = diwk_str_curs_pos( );
		memmove( &string[i+len], &string[i], sl-i );
		memcpy( &string[i], buf, len );
		if( ks != XK_Return ) curs[0] += len;
		else { curs[0] = 0; curs[1]++; }
		sl += len;
		string[sl] = 0;

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

	return -1;
}

char *diwk_text_prompt( char _lines, Bool _echo ) {
	type = 0;
	lines = (_lines>0?_lines:1);
	diwk_create_window( lines );

	string = malloc( STRBUFLEN*BUFLEN+1 );
	string[0] = 0;
	sl = 0;
	view_up = 0;
	curs[0] = curs[1] = 0;

	if( !_echo ) {
		dc->x = 0; dc->y = 0;
		dc->w = textw( dc, "passwd will not be echo'd" );
		dc->h = lh;
		drawtext( dc, "passwd will not be echo'd", col );
		mapdc( dc, win, dw, lh );
	}

	char buf[BUFLEN];
	KeySym ks;
	int len, i, j;
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
				case XK_p:
					diwk_view_add( -1 );
					break;
				case XK_n:
					diwk_view_add( 1 );
					break;
				}
			}

			// no mod pressed
			else {
				switch( ks ) {
				case XK_Escape: return string;

				case XK_Up:
					if( curs[1] > 0 ) curs[1]--;
					break;
				case XK_Down:
					if( !diwk_is_last_row( ) ) curs[1]++;
					else curs[0] = diwk_row_last_col( );
					break;
				case XK_Left:
					if( curs[0] > 0 ) curs[0]--;
					else {
						curs[1]--;
						curs[0] = diwk_row_last_col( );
					}
					break;
				case XK_Right:
					if( curs[0] != diwk_row_last_col( ) ) curs[0]++;
					else if( !diwk_is_last_row( ) ) {
						curs[0] = 0;
						curs[1]++;
					}
					break;

				case XK_Return:
					buf[0] = '\n';
					len = 1;
				default:
					i = diwk_str_curs_pos( );
					memmove( &string[i+len], &string[i], sl-i );
					memcpy( &string[i], buf, len );
					if( ks != XK_Return ) curs[0] += len;
					else { curs[0] = 0; curs[1]++; }
					sl += len;
					string[sl] = 0;
					break;
					
				case XK_BackSpace:
					i = diwk_str_curs_pos( );
					if( i == 0 ) break;
					for( j = 1; utf8hnd( string[i-j] ) == -1; j++ );
					if( string[i-j] == '\n' ) {
						curs[1]--;
						curs[0] = diwk_row_last_col( );
					} else curs[0] -= j;
					memmove( &string[i-j], &string[i], j );
					sl -= j;
					break;
				}
			}
		 }

		// WARNING: the following section is as ugly as your mom last night
		if( _echo ) {
			if( curs[1] < view_up ) diwk_view_add( -1 );
			if( curs[1] > view_up+lines-1 ) diwk_view_add( 1 );

			dc->x = dc->y = 0; dc->w = dw; dc->h = lh*lines;
			drawrect( dc, 0, 0, dw, lh*lines, True, BG( dc, bw ) );

			dc->w = 0; dc->h = lh;

			int i, j, k; for( i = j = k = 0; i < sl; i++ ) {
				if( string[i] == '\n' ) {
					if( k >= view_up && k <= view_up+lines ) {
						char tmpstr[i-j+1];
						memcpy( tmpstr, &string[j], i-j+1 );
						tmpstr[i-j] = 0;
						dc->w = textw( dc, tmpstr );
						drawtext( dc, tmpstr, col );
						dc->y += lh;
					} k++;
					j = i+1;
				}
			}
			char tmpstr[i-j+1];
			memcpy( tmpstr, &string[j], i-j+1 );
			tmpstr[i-j] = 0;
			dc->w = textw( dc, tmpstr );
			drawtext( dc, tmpstr, col );

			dc->x = dc->y = 0;
			drawrect( dc, textnw( dc, tmpstr, curs[0]+1 ), (curs[1]-view_up)*lh, 1, lh, True, FG( dc, col ) );

			mapdc( dc, win, dw, lh*lines );
		}
	}
}

/*
char *diwk_pass_prompt( ) {
	type = 1;
	lines = 1;
	diwk_create_window( lines );

	string = malloc( STRBUFLEN*BUFLEN+1 );
	sl = 0;


	for( ;; ) {
		XNextEvent( dc->dpy, &e );
		switch( e.type ) {
		case KeyPress:
			switch( diwk_kb_ipret( ) ) {
			case DIWK_RET_DISC:
				if( string ) free( string );
				return "\0";
			case DIWK_RET_SAVE:
				if( string ) free( string );
				return string;
			}
		}
	}
}
*/

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

void diwk_view_add( int _n ) {
	view_up += _n;
	if( view_up < 0 ) view_up -= _n;
	else if( view_up+lines > diwk_last_row( ) ) view_up -= _n;
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

int diwk_row_last_col( ) {
	int i, j; for( i = j = 0; i < sl; i++ ) {
		if( string[i] == '\n' ) j++;
		if( j == curs[1] ) break;
	}
	for( j = i+1; j < sl; j++ )
		if( string[j] == '\n' ) return j-i;
	return 0;
}

int diwk_str_curs_pos( ) {
	int i, j; for( i = j = 0; i < sl; i++ ) {
		if( j == curs[1] ) break;
		if( string[i] == '\n' ) j++;
	}
	return i+curs[0];
}

Bool diwk_is_last_row( ) {
	int i; for( i = diwk_str_curs_pos( ); i < sl; i++ )
		if( string[i] == '\n' ) return False;
	return True;
}

int diwk_last_row( ) {
	int i, j; for( i = j = 0; i < sl; i++ )
		if( string[i] == '\n' ) j++;
	return j+1;
}
