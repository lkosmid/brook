
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glruntime.hpp"
#ifndef WIN32
#define GLX_FLOAT_COMPONENTS_NV         0x20B0
#endif

using namespace brook;

#ifdef WIN32
/*
 * The name of the window.  It is also the window class name.
 */
static const char window_name[] = "Brook GL Render Window";

void
GLRunTime::createWindow(void) {
  HINSTANCE hinstance;
  WNDCLASS wc;
  DWORD window_style;

  /*
   * These parameters are useless since the window is never shown nor
   * rendered into.
   */
  const int window_width = 10;
  const int window_height = 10;
  const int window_x = 0;
  const int window_y = 0;

  hinstance = GetModuleHandle( NULL );

  // Create the window class
  if (!GetClassInfo(hinstance, window_name, &wc)) {
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


/*
 * bSetupPixelFormat --
 *
 * This function creates a default GL context which is never really used by
 * Brook but is needed to construct a pbuffer
 */

static BOOL
bSetupPixelFormat(HDC hdc)
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

    if (pixelformat == 0) {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }
    if (!SetPixelFormat(hdc, pixelformat, ppfd)) {
        MessageBox( NULL, "SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }
    return TRUE;
}


void
GLRunTime::createWindowGLContext(void)
{
  assert(hwnd);
  hdc_window = GetDC(hwnd);

  assert(bSetupPixelFormat(hdc_window));

  hglrc_window = wglCreateContext(hdc_window);
  assert(hglrc_window);

  assert(wglMakeCurrent(hdc_window, hglrc_window));
}


// Ugh.  Should we put this into NVIDIA's wglext.h?
#define WGL_TYPE_RGBA_FLOAT_ATI             0x21A0

void
GLRunTime::createPBufferWGL(int ncomponents)
{
   static int pixelformat[4];
   static bool runOnce;

   static const int piAttribList[] = {0,0,
			 WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_R_NV,
			 WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
			 0,0,
			 WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_RG_NV,
			 WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
			 0,0,
			 WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_RGB_NV,
			 WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
			 0,0,
			 WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_RGBA_NV,
			 WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV,
			 0,0};

   static bool ati_not_nv;

   if (!runOnce) {
      BOOL status;
      int iAttributes[30];
      int nAttrib = 0;
      static const float fAttributes[] = {0,0};

      unsigned int numFormats;
      
      const char *(*wglGetExtensionsString)(void) = 
        (const char *(*)(void)) wglGetProcAddress("wglGetExtensionsStringEXT");

      if (!wglGetExtensionsString) {
        fprintf(stderr, "Graphics adaptor %s does not support "
                "wglGetExtensionsString.\n", glGetString(GL_RENDERER));
        exit(1);
      }

      /* Figure out which pbuffer to create */
      if (strstr (wglGetExtensionsString(), 
                  "WGL_ATI_pixel_format_float")) {                  
        ati_not_nv = true;
      } else {
        if (strstr (wglGetExtensionsString(),
                    "WGL_NV_float_buffer")) {
          ati_not_nv = false;
        } else {
          fprintf (stderr, "WARNING: Graphics adaptor %s does not support\n"
                   "known floating point render targets. Assuming ATI.\n",
                   glGetString(GL_RENDERER));
          ati_not_nv = true;
          
        }
      }   


      for (int i=0; i<4; i++) {

#define PUSH_ATTRIB(a, b) \
iAttributes[nAttrib++] = a; iAttributes[nAttrib++] = b; 
	 
	 nAttrib = 0;
	 PUSH_ATTRIB (WGL_RED_BITS_ARB,        32);
	 PUSH_ATTRIB (WGL_GREEN_BITS_ARB,      i>0?32:0);
	 PUSH_ATTRIB (WGL_BLUE_BITS_ARB,       i>1?32:0);
	 PUSH_ATTRIB (WGL_ALPHA_BITS_ARB,      i>3?32:0);
	 PUSH_ATTRIB (WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE);
	 PUSH_ATTRIB (WGL_ACCELERATION_ARB,    WGL_FULL_ACCELERATION_ARB);
	 PUSH_ATTRIB (WGL_DEPTH_BITS_ARB,      0);
	 PUSH_ATTRIB (WGL_STENCIL_BITS_ARB,    0);
	 PUSH_ATTRIB (WGL_DOUBLE_BUFFER_ARB,   GL_FALSE);
	 PUSH_ATTRIB (WGL_SUPPORT_OPENGL_ARB,  GL_TRUE);
	 PUSH_ATTRIB (WGL_AUX_BUFFERS_ARB,     0);

	 if (ati_not_nv) {
	   PUSH_ATTRIB (WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_FLOAT_ATI);
	 } else {
	   int mode[] = { WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV,
			  WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RG_NV,
			  WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGB_NV,
			  WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV
	   };			  
	   PUSH_ATTRIB (WGL_FLOAT_COMPONENTS_NV, GL_TRUE);
	   PUSH_ATTRIB (mode[i], GL_TRUE);
        
	   PUSH_ATTRIB (0, 0);
	 }

         status = wglChoosePixelFormatARB(hdc_window, iAttributes,
                                          fAttributes, 1,
                                          pixelformat+i, &numFormats);

         if ( numFormats == 0  || !status) {
            fprintf(stderr, "GL: ChoosePixelFormat failed to find format\n");
            exit(1);
         }
      }
   }

   if (runOnce) {
      assert(wglMakeCurrent (hpbufferdc, NULL));
      assert(wglReleasePbufferDCARB (hpbuffer, hpbufferdc));
      assert(wglDestroyPbufferARB (hpbuffer));
   }

   hpbuffer = wglCreatePbufferARB(hdc_window,
                                  pixelformat[ncomponents-1],
                                  workspace, workspace,
                                  ati_not_nv?
				    piAttribList:
				  (piAttribList+(ncomponents-1)*6+1)
				  );

   if (!hpbuffer) {
      unsigned int gl_err = GetLastError();

      fprintf (stderr, "GL:  Failed to create pbuffer.\n");
      if (gl_err == ERROR_INVALID_HANDLE)
         fprintf (stderr, "GetLastError: ERROR_INVALID_HANDLE\n");
      else if (gl_err == ERROR_INVALID_DATA)
         fprintf (stderr, "GetLastError: ERROR_INVALID_DATA\n");
      else
         fprintf (stderr, "GetLastError: 0x%x\n", gl_err);
      exit(1);
   }

   hpbufferdc = wglGetPbufferDCARB (hpbuffer);
   assert (hpbufferdc);

   if (!runOnce)
      hpbufferglrc = wglCreateContext( hpbufferdc );
   assert (hpbufferglrc);

   if (!wglMakeCurrent( hpbufferdc, hpbufferglrc )) {
      fprintf( stderr, "GL:  MakeCurrent Failed.\n");
      fprintf (stderr, "GetLastError: 0x%x\n", GetLastError());
      assert(0);
   }

   CHECK_GL();
   runOnce = true;
}

#else

void
GLRunTime::createPBufferGLX(int ncomponents)
{
  static Display   *pDisplay;
  static int iScreen;
  static GLXFBConfig *glxConfig[4];

  static const int pbAttribList[] =  {
     GLX_PRESERVED_CONTENTS, true,
     GLX_PBUFFER_WIDTH, workspace,
     GLX_PBUFFER_HEIGHT, workspace,
     0,
  };

  static bool runOnce;

  if (runOnce) {
    // XXX: For some reson, I can't deal
    // with changing the pbuffer size on Linux... (Ian)
    pbuffer_ncomp = ncomponents;
    return;
  }

  if (!runOnce) {
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

       if (!glxConfig[i][0]) {
	 fprintf(stderr, "GL:  glXChooseFBConfig() failed\n");
	 exit(1);
       }
     }
  }


  if (runOnce) {
    glXMakeCurrent(pDisplay, None, NULL);
    glXDestroyPbuffer(pDisplay, glxPbuffer);
  }

  glxPbuffer = glXCreatePbuffer(pDisplay,
				glxConfig[ncomponents-1][0],
				pbAttribList);

  if (!glxPbuffer) {
    fprintf(stderr, "GL: glXCreatePbuffer() failed\n");
    exit(1);
  }

  if (!runOnce) {
     glxContext = glXCreateNewContext(pDisplay,
                                      glxConfig[ncomponents-1][0],
                                      GLX_RGBA_TYPE,
                                      0, true);
     if (!glxConfig) {
        fprintf(stderr, "GL: glXCreateContextWithConfig() failed\n");
        exit (1);
     }
  }

  glXMakeCurrent(pDisplay, glxPbuffer, glxContext);
  runOnce = true;
}
#endif


void
GLRunTime::createPBuffer(int ncomponents)
{
#ifdef WIN32
   createPBufferWGL(ncomponents);
#else
   createPBufferGLX(ncomponents);
#endif

   pbuffer_ncomp = ncomponents;
}
