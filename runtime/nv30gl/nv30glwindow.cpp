
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <GL/gl.h>

#include "glext.h"
#include "wglext.h"

#include "nv30gl.hpp"

using namespace brook;

/*  The name of the window.  It is also the
**  window class name.
*/
const char window_name[] = "Brook NV30GL Render Window";

void
NV30GLRunTime::createWindow (void) {
  
  HINSTANCE hinstance;
  WNDCLASS wc;
  DWORD window_style;

  /* These parameters are useless since
  ** the window is never shown nor rendered 
  ** into.
  */
  const int window_width = 100;
  const int window_height = 100;
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
}


// This function creates a default
// GL context which is never really used
// by Brook but is needed to construct
// a pbuffer

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

void
NV30GLRunTime::createWindowGLContext(void) {
  HDC hdc;

  assert(hwnd);
  hdc = GetDC( hwnd);
  
  assert(bSetupPixelFormat( hdc ) );
  
  hglrc_window = wglCreateContext( hdc );
  assert(hglrc_window);
 
  assert(wglMakeCurrent( hdc, hglrc_window ));
}


void
NV30GLRunTime::createPBuffer (void) {

  int pixelformat;
  int piAttribList[] = {0,0};
  float fAttributes[] = {0, 0};
  int iAttributes[30] = { WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE,
			  WGL_FLOAT_COMPONENTS_NV, GL_TRUE,
                          WGL_ACCELERATION_ARB,    WGL_FULL_ACCELERATION_ARB,
			  WGL_COLOR_BITS_ARB,      128,
			  WGL_ALPHA_BITS_ARB,      32,
			  WGL_DEPTH_BITS_ARB,      0,
			  WGL_STENCIL_BITS_ARB,    0, 
			  WGL_DOUBLE_BUFFER_ARB,   GL_FALSE,
                          WGL_SUPPORT_OPENGL_ARB,  GL_TRUE,
			  0,                       0};
  unsigned int numFormats;
  BOOL status;

  HDC hdc = wglGetCurrentDC();
  HDC hpbufferdc;
  
  status = wglChoosePixelFormatARB(hdc, iAttributes, fAttributes, 1,
				   &pixelformat, &numFormats);
  
  if ( numFormats == 0 ) {
     MessageBox( NULL, 
                 "ChoosePixelFormat failed to find a format", 
                 "Error", MB_OK ); 
     exit(1);
  }

  if ( !status ) 
    {
      MessageBox( NULL, "wglChoosePixelFormatARB failed", "Error", MB_OK ); 
      exit(1);
    }
  
  hpbuffer = wglCreatePbufferARB(hdc,
				 pixelformat,
				 workspace, workspace,
				 piAttribList);
  
  if ( !hpbuffer ) 
    {
      fprintf (stderr, "GetLastError: 0x%x\n", GetLastError());
      MessageBox( NULL, "Failed to create pbuffer",
                  "wglCreatePbufferARB Error",
		  MB_OK | MB_ICONINFORMATION );
      exit(1);
    }
  
  hpbufferdc = wglGetPbufferDCARB (hpbuffer);
  assert (hpbufferdc);
  
  hpbufferglrc = wglCreateContext( hpbufferdc );
  assert (hpbufferglrc);

  if (!wglMakeCurrent( hpbufferdc, hpbufferglrc )) {
     fprintf (stderr, "GetLastError: 0x%x\n", GetLastError());
     assert(0);
  }

  CHECK_GL();
}

#if 0

void clearoutput(float x, float y, float z, float w) {
  glClearColor(x, y, z, w);
  glClear(GL_COLOR_BUFFER_BIT);
}

void hidewindow(HWND hwnd) {
  ShowWindow(hwnd, SW_HIDE);
}

void makecurrentwindow(HDC hdc, HGLRC hglrc) {
  assert(wglMakeCurrent(hdc, hglrc ));
}

void makecurrentpbuffer(HDC hpbufferdc, HGLRC hpbufferglrc) {
  assert(wglMakeCurrent( hpbufferdc, hpbufferglrc ));
  glDrawBuffer (GL_FRONT);
  glReadBuffer (GL_FRONT);
  CHECK_GL();
}

static int pbufferbound        = 0;
static int isfront             = 1;
const  int wglfront_back[2]    = {WGL_FRONT_ARB, WGL_BACK_ARB};
const  int front_back[2]       = {GL_FRONT, GL_BACK};

void swappbuffer(void) {
  if (pbufferbound) {
    assert(wglReleaseTexImageARB(hpbuffer, wglfront_back[isfront]));
    CHECK_GL();
  }
  
  glDrawBuffer(front_back[isfront]);
  glReadBuffer(front_back[isfront]);
  CHECK_GL();

  isfront = !isfront;
}

void bindpbuffer(void) {
  assert(wglBindTexImageARB(hpbuffer, wglfront_back[isfront]));
  pbufferbound = 1;
  CHECK_GL();
}

void releasepbuffer(void) {
  assert(wglReleaseTexImageARB(hpbuffer, wglfront_back[isfront]));
  pbufferbound = 0;
}

#endif
