
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

#include "nv30gl.hpp"

using namespace brook;

#ifndef WIN32
void
NV30GLRunTime::createPBufferGLX(int ncomponents)
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
    // with changing the pbuffer size on Linux...
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
	 fprintf(stderr, "NV30GL:  glXChooseFBConfig() failed\n");
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
    fprintf(stderr, "NV30GL: glXCreatePbuffer() failed\n");
    exit(1);
  }

  if (!runOnce) {
     glxContext = glXCreateNewContext(pDisplay,
                                      glxConfig[ncomponents-1][0],
                                      GLX_RGBA_TYPE,
                                      0, true);
     if (!glxConfig) {
        fprintf(stderr, "NV30GL: glXCreateContextWithConfig() failed\n");
        exit (1);
     }
  }

  glXMakeCurrent(pDisplay, glxPbuffer, glxContext);
  runOnce = true;
}

#else

void
NV30GLRunTime::createPBufferWGL (int ncomponents)
{
   static int pixelformat[4];
   static bool runOnce;
   static const int piAttribList[] = {0,0};
   static const float fAttributes[] = {0, 0};

   if (!runOnce) {
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

   if (runOnce) {
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

   if (!runOnce)
      hpbufferglrc = wglCreateContext( hpbufferdc );
   assert (hpbufferglrc);

   if (!wglMakeCurrent( hpbufferdc, hpbufferglrc )) {
      fprintf( stderr, "NV30GL:  MakeCurrent Failed.\n");
      fprintf (stderr, "GetLastError: 0x%x\n", GetLastError());
      assert(0);
   }

   CHECK_GL();
   runOnce = true;
}
#endif


void
NV30GLRunTime::createPBuffer(int ncomponents)
{
#ifdef WIN32
   createPBufferWGL(ncomponents);
#else
   createPBufferGLX(ncomponents);
#endif

   pbuffer_ncomp = ncomponents;
}
