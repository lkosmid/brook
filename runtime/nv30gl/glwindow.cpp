
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include "wglext.h"
#else
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#endif

#include "glruntime.hpp"

using namespace brook;

/*  The name of the window.  It is also the
**  window class name.
*/
static const char window_name[] = "Brook GL Render Window";

void
GLRunTime::createWindow (void) {
#ifdef WIN32
  HINSTANCE hinstance;
  WNDCLASS wc;
  DWORD window_style;

  /* These parameters are useless since
  ** the window is never shown nor rendered
  ** into.
  */
  const int window_width = 10;
  const int window_height = 10;
  const int window_x = 0;
  const int window_y = 0;

  hinstance = GetModuleHandle( NULL );

  // Create the window class
  if ( !GetClassInfo(hinstance, window_name, &wc) )
    {
      wc.style = CS_OWNDC;
      wc.lpfnWndProc = (WNDPROC) DefWindowProc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = hinstance;
      wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
      wc.hCursor = LoadCursor( NULL, IDC_ARROW );
      wc.hbrBackground = NULL;
      wc.lpszMenuName = NULL;
      wc.lpszClassName = window_name;

      assert (RegisterClass( &wc ));
    }

  window_style = ( WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
  window_style |= WS_POPUP;

  // Create the window
  hwnd = CreateWindow( window_name, window_name,
		       window_style,
		       window_x,
		       window_y,
		       window_width,
		       window_height,
		       NULL, NULL, hinstance, NULL );

  assert (hwnd);
#endif
}



// This function creates a default
// GL context which is never really used
// by Brook but is needed to construct
// a pbuffer

#ifdef WIN32
static BOOL
bSetupPixelFormat( HDC hdc )
{
    PIXELFORMATDESCRIPTOR *ppfd;
    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
      1,                              // version number
      PFD_DRAW_TO_WINDOW |            // support window
      PFD_SUPPORT_OPENGL |            // support OpenGL
      PFD_DOUBLEBUFFER,               // double buffered
      PFD_TYPE_RGBA,                  // RGBA type
      0,                              // 24-bit color depth
      0, 0, 0, 0, 0, 0,               // color bits ignored
      0,                              // no alpha buffer
      0,                              // shift bit ignored
      0,                              // no accumulation buffer
      0, 0, 0, 0,                     // accum bits ignored
      0,                              // set depth buffer
      0,                              // set stencil buffer
      0,                              // no auxiliary buffer
      PFD_MAIN_PLANE,                 // main layer
      0,                              // reserved
      0, 0, 0                         // layer masks ignored
    };
    int pixelformat;

    ppfd = &pfd;
    pixelformat = ChoosePixelFormat( hdc, ppfd );

    if ( pixelformat == 0 )
    {
        MessageBox( NULL, "ChoosePixelFormat failed", "Error", MB_OK );
        return FALSE;
    }
    if ( !SetPixelFormat( hdc, pixelformat, ppfd ) )
    {
        MessageBox( NULL, "SetPixelFormat failed", "Error", MB_OK );
        return FALSE;
    }
    return TRUE;
}
#endif

void
GLRunTime::createWindowGLContext(void) {
#ifdef WIN32
  assert(hwnd);
  hdc_window = GetDC( hwnd);

  assert(bSetupPixelFormat( hdc_window ) );

  hglrc_window = wglCreateContext( hdc_window );
  assert(hglrc_window);

  assert(wglMakeCurrent( hdc_window, hglrc_window ));
#endif
}
