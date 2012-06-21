// See LICENSE file for copyright and license details.
#include "diwk.h"
#include "config.h"

int main( int argc, char *argv[] ) {
	diwk_init( );

	printf( "%s\n", diwk_pass_prompt( ) );

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
	if( type == 2 ) {
		dc->x = 0;
		dc->y = 0;
		dc->h = 18;

		col[ColBG] = getcolor( dc, normbg );
		col[ColFG] = getcolor( dc, normfg );

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
	if( type == 1 ) {
		KeySym ks;
		int len = Xutf8LookupString( xic, &e.xkey, buf, BUFLEN, &ks, NULL );
		if( sl % 25 > STRBUFLEN-BUFLEN-1 )
			string = realloc( string, sl+STRBUFLEN+(25-(sl%25))+1 );

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
			} return DIWK_KEY_NOTHING;
		case XK_Down:
			if( listcur < lines-1 ) {
				listcur++;
				return DIWK_DOWN;
			} return DIWK_KEY_NOTHING;
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
