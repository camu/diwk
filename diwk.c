// See LICENSE file for copyright and license details.
#include "diwk.h"
#include "config.h"

int main( int argc, char *argv[] ) {
	const char *herp[] = { "herp", "sherp", "derp" };
	printf( "%s\n", diwk_radio_button( herp, 3 ) );

	diwk_clean( );

	return 0;
}

void diwk_init( int _lines, const char *_label ) {
	// gfx
	dc = initdc( );
	col[ColBG] = getcolor( dc, normbg );
	col[ColFG] = getcolor( dc, normfg );
	bw[ColBG] = getcolor( dc, bwbg );
	bw[ColFG] = getcolor( dc, bwfg );
	initfont( dc, NULL );
	lh = 18; lines = _lines;
	dw = DisplayWidth( dc->dpy, screen );
	screen = DefaultScreen( dc->dpy );
	Window root = RootWindow( dc->dpy, screen );
	XSetWindowAttributes swa;
	swa.override_redirect = True;
	swa.background_pixel = getcolor( dc, "#ffffff" );
	swa.event_mask = ExposureMask | VisibilityChangeMask | KeyPressMask;
	win = XCreateWindow( dc->dpy, root, 0, 0, dw, lh*lines, 0, DefaultDepth( dc->dpy, screen ), CopyFromParent, DefaultVisual( dc->dpy, screen ), CWOverrideRedirect | CWBackPixel | CWEventMask, &swa );
	XMapRaised( dc->dpy, win );
	resizedc( dc, dw, lh*lines );
	diwk_draw( );

	// kb
	if( XGrabKeyboard( dc->dpy, DefaultRootWindow( dc->dpy ), True, GrabModeAsync, GrabModeAsync, CurrentTime ) != GrabSuccess )
		erhnd( "failed to grab kb" );
}

void diwk_clean( ) {
	if( list ) free( list );
	freedc( dc );
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
	if( type == 2 ) {
		KeySym ks = XLookupKeysym( &(e.xkey), 0 );

		// mod1, usually alt
		if( e.xkey.state & Mod1Mask ) {
			switch( ks ) {
			case XK_Escape: return 1;
			}
		}

		// no mod pressed
		switch( ks ) {
		case XK_Escape: return 2;
		case XK_Up:
			if( listcur > 0 ) listcur--;
			break;
		case XK_Down:
			if( listcur < lines-1 ) listcur++;
			break;
		}
	}
	return -1;
}

char *diwk_radio_button( const char **_str, int _n ) {
	type = 2;
	listcur = 0;

	list = malloc( POINTERSIZE*_n );
	int i; for( i = 0; i < _n; i++ ) list[i] = _str[i];

	diwk_init( _n, "no label" );
	Bool q = False;
	for( ;!q; ) {
		XNextEvent( dc->dpy, &e );
		switch( e.type ) {
		case KeyPress:
			switch( diwk_kb_ipret( 2 ) ) {
			case 0:
//				printf( "some press\n" );
				break;
			case 1:
//				printf( "discarded\n" );
				list[listcur][0] = '\0';
				q = True;
			case 2:
//				printf( "saved\n" );
				q = True;
			}
		}
		diwk_draw( );
	}

	return list[listcur];
}
