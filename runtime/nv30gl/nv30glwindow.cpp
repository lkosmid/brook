
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
#define GLX_FLOAT_COMPONENTS_NV         0x20B0
#endif

#include "nv30glext.h"

#include "nv30gl.hpp"

using namespace brook;

/*  The name of the window.  It is also the
**  window class name.
*/
const char window_name[] = "Brook NV30GL Render Window";

void
NV30GLRunTime::createWindow (void) {
  
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
NV30GLRunTime::createWindowGLContext(void) {
#ifdef WIN32

  assert(hwnd);
  hdc_window = GetDC( hwnd);
  
  assert(bSetupPixelFormat( hdc_window ) );
  
  hglrc_window = wglCreateContext( hdc_window );
  assert(hglrc_window);
 
  assert(wglMakeCurrent( hdc_window, hglrc_window ));
#endif
}


void
NV30GLRunTime::createPBuffer (int ncomponents) {

#ifdef WIN32
   static int pixelformat[4];
   static int first;
   static const int piAttribList[] = {0,0};
   static const float fAttributes[] = {0, 0};

   if (!first) {
      BOOL status;
      int iAttributes[30] = { WGL_RED_BITS_ARB,        0,
                              WGL_GREEN_BITS_ARB,      0,
                              WGL_BLUE_BITS_ARB,       0,
                              WGL_ALPHA_BITS_ARB,      0,
                              WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE,
                              WGL_FLOAT_COMPONENTS_NV, GL_TRUE,
                              WGL_ACCELERATION_ARB,    WGL_FULL_ACCELERATION_ARB,
                              WGL_DEPTH_BITS_ARB,      0,
                              WGL_STENCIL_BITS_ARB,    0, 
                              WGL_DOUBLE_BUFFER_ARB,   GL_FALSE,
                              WGL_SUPPORT_OPENGL_ARB,  GL_TRUE,
                              0,                       0};
      unsigned int numFormats;
      
      for (int i=0; i<4; i++) {
         for (int j=0; j<4; j++)
            iAttributes[1+j*2] = (j<=i)?32:0;
         
         status = wglChoosePixelFormatARB(hdc_window, iAttributes, 
                                          fAttributes, 1,
                                          pixelformat+i, &numFormats);
         
         if ( numFormats == 0  || !status) {
            fprintf(stderr, "NV30GL: ChoosePixelFormat failed to find format\n");
            exit(1);
         }
      }  
   }
   
   if (first) {
      assert(wglMakeCurrent (hpbufferdc, NULL));
      assert(wglReleasePbufferDCARB (hpbuffer, hpbufferdc));
      assert(wglDestroyPbufferARB (hpbuffer));
   }
   
   hpbuffer = wglCreatePbufferARB(hdc_window,
                                  pixelformat[ncomponents-1],
                                  workspace, workspace,
                                  piAttribList);
   
   if ( !hpbuffer ) {
      unsigned int nv_err = GetLastError();
      fprintf (stderr, "NV30GL:  Failed to create pbuffer.\n");
      if (nv_err == ERROR_INVALID_HANDLE)
         fprintf (stderr, "GetLastError: ERROR_INVALID_HANDLE\n");
      else if (nv_err == ERROR_INVALID_DATA)
         fprintf (stderr, "GetLastError: ERROR_INVALID_DATA\n");
      else
         fprintf (stderr, "GetLastError: 0x%x\n", nv_err);
      exit(1);
   }
   
   hpbufferdc = wglGetPbufferDCARB (hpbuffer);
   assert (hpbufferdc);
   
   if (!first)
      hpbufferglrc = wglCreateContext( hpbufferdc );
   assert (hpbufferglrc);
   
   if (!wglMakeCurrent( hpbufferdc, hpbufferglrc )) {
      fprintf( stderr, "NV30GL:  MakeCurrent Failed.\n");
      fprintf (stderr, "GetLastError: 0x%x\n", GetLastError());
      assert(0);
   }

   CHECK_GL();

#else

  /* GLX Pbuffer creation */

  static Display   *pDisplay;
  static int iScreen;
  
  GLXFBConfig *glxConfig[4];

  static const int pbAttribList[] =  {
     GLX_PRESERVED_CONTENTS, true,
     GLX_PBUFFER_WIDTH, workspace,
     GLX_PBUFFER_HEIGHT, workspace,
     0,
  };
  
  static int first;
 
  if (!first) {
     int iConfigCount;   
     int pfAttribList[] = 
        {
           GLX_RED_SIZE,               0,
           GLX_GREEN_SIZE,             0,
           GLX_BLUE_SIZE,              0,
           GLX_ALPHA_SIZE,             0,
           GLX_STENCIL_SIZE,           0,
           GLX_DEPTH_SIZE,             0,
           GLX_FLOAT_COMPONENTS_NV,    true,
           GLX_DRAWABLE_TYPE,          GLX_PBUFFER_BIT,
           0,
        };

     pDisplay = XOpenDisplay(NULL);
     iScreen  = DefaultScreen(pDisplay);

     for (int i=0; i<4; i++) {
         for (int j=0; j<4; j++)
            pfAttribList[1+j*2] = (j<=i)?32:0;

        glxConfig[i] = glXChooseFBConfig(pDisplay, 
                                         iScreen, 
                                         pfAttribList, 
                                         &iConfigCount);

        if (!glxConfig[i]) {
           fprintf(stderr, "NV30GL:  glXChooseFBConfig() failed\n");
           exit(1);
        }        
     }
  }

     
  if (first) {
     glXMakeCurrent(pDisplay, glxPbuffer, NULL);
     glXDeletePbuffer(pDisplay, glxPbuffer);
  }

  glxPbuffer = glXCreatePbuffer(pDisplay, 
				glxConfig[ncomponents-1][0], 
				pbAttribList);
  
  if (!glxPbuffer) {
    fprintf(stderr, "NV30GL: glXCreatePbuffer() failed\n");
    exit(1);
  }
  
  if (!first) {
     glxContext = glXCreateNewContext(pDisplay, 
                                      glxConfig[0], 
                                      GLX_RGBA_TYPE, 
                                      0, true);
     if (!glxConfig) {
        fprintf(stderr, "NV30GL: glXCreateContextWithConfig() failed\n");
        exit (1);
     }
  }
     
  glXMakeCurrent(pDisplay, glxPbuffer, glxContext);


#endif

  pbuffer_ncomp = ncomponents;
  first = 1;
}
