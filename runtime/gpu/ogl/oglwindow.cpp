
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"
#include "wglext.h"

#include "oglcontext.hpp"

using namespace brook;

#ifdef WIN32

static const char window_name[] = "Brook GL Render Window";

static HWND
create_window (void) {
  HINSTANCE hinstance;
  WNDCLASS wc;
  DWORD window_style;
  HWND hwnd;

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

  if (!hwnd)
    GPUError ("Failed to create window");

  return hwnd;
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
  int status;
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
  
  GPUAssert(pixelformat, "ChoosePixelFormat failed");
  
  status = SetPixelFormat(hdc, pixelformat, ppfd);
  
  GPUAssert(status, "SetPixelFormat failed");
  
  return TRUE;
}

#define CRGBA(c,r,g,b,a) \
  WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE, \
  WGL_ACCELERATION_ARB,    WGL_FULL_ACCELERATION_ARB, \
  WGL_DEPTH_BITS_ARB,      0,\
  WGL_STENCIL_BITS_ARB,    0,\
  WGL_DOUBLE_BUFFER_ARB,   GL_FALSE, \
  WGL_SUPPORT_OPENGL_ARB,  GL_TRUE, \
  WGL_AUX_BUFFERS_ARB,     0, \
  WGL_FLOAT_COMPONENTS_NV, GL_TRUE, \
  WGL_COLOR_BITS_ARB,      c, \
  WGL_RED_BITS_ARB,        r, \
  WGL_GREEN_BITS_ARB,      g, \
  WGL_BLUE_BITS_ARB,       b, \
  WGL_ALPHA_BITS_ARB,      a


/* These require vendor specific strings to append */
static const int 
baseiAttribList[4][64] = { {CRGBA(32,32,0,0,0), 0, 0},
                           {CRGBA(64,32,32,0,0), 0, 0},
                           {CRGBA(96,32,32,32,0), 0, 0},
                           {CRGBA(128,32,32,32,32), 0, 0} };

static const float
basefAttribList[4][16] = { {0.0f,0.0f}, 
                           {0.0f,0.0f}, 
                           {0.0f,0.0f},
                           {0.0f,0.0f}};

static int
basepiAttribList[4][16] = { {0, 0},
                            {0, 0},
                            {0, 0},
                            {0, 0}};

void 
OGLContext::appendVendorAttribs() {

  int i,j,k;
  int nattrib = 0;

  const int   (*viAttribList)[4][64];
  const float (*vfAttribList)[4][16];
  const int   (*vpiAttribList)[4][16];

  getVendorAttribs(&viAttribList, &vfAttribList, &vpiAttribList);

  for (i=0; i<4; i++) {

    for (j=0; j<63; j+=2) {
      GPUAssert(j<64, "Error: no room for base attribs");
      iAttribList[i][j]   = baseiAttribList[i][j];
      iAttribList[i][j+1] = baseiAttribList[i][j];
      if (iAttribList[i][j]   == 0 && 
          iAttribList[i][j+1] == 0)
        break;
    }

    if (viAttribList) {
      for (k=0; k<63; k+=2) { 
        GPUAssert(j<64, "Error: no room for vendor attribs");
        
        iAttribList[i][j++]  = (*viAttribList)[i][k];
        iAttribList[i][j++]  = (*viAttribList)[i][k+1];
        
      if ((*viAttribList)[i][k]   == 0 && 
          (*viAttribList)[i][k+1] == 0)
        break;
      }
    }


    for (j=0; j<16; j+=2) {
      GPUAssert(j<16, "Error: no room for base attribs");
      fAttribList[i][j]   = basefAttribList[i][j];
      fAttribList[i][j+1] = basefAttribList[i][j];
      if (fAttribList[i][j]   == 0.0f && 
          fAttribList[i][j+1] == 0.0f)
        break;
    }
    

    if (vfAttribList) {
      for (k=0; k<16; k+=2) { 
        GPUAssert(j<16, "Error: no room for vendor attribs");
        
        fAttribList[i][j++]  = (*vfAttribList)[i][k];
        fAttribList[i][j++]  = (*vfAttribList)[i][k+1];
        
        if ((*vfAttribList)[i][k]   == 0.0f && 
            (*vfAttribList)[i][k+1] == 0.0f)
          break;
      }
    }


    for (j=0; j<16; j+=2) {
      GPUAssert(j<16, "Error: no room for base attribs");
      piAttribList[i][j]   = basepiAttribList[i][j];
      piAttribList[i][j+1] = basepiAttribList[i][j];
      if (piAttribList[i][j]   == 0 && 
          piAttribList[i][j+1] == 0)
        break;
    }

    if (vpiAttribList) {
      for (k=0; k<16; k+=2) { 
        GPUAssert(j<16, "Error: no room for vendor attribs");
        
        piAttribList[i][j++]  = (*vpiAttribList)[i][k];
        piAttribList[i][j++]  = (*vpiAttribList)[i][k+1];
        
        if ((*vpiAttribList)[i][k]   == 0 && 
            (*vpiAttribList)[i][k+1] == 0)
          break;
      }
    }
  }
}

void
OGLContext::initPbufferWGL(void) {

  unsigned int numFormats;
  BOOL status;
  HGLRC hglrc_window;

  /* Create a window */
  HWND hwnd = create_window();
  hwindowdc = GetDC(hwnd);

  /* Initialize the initial window GL_context */
  status = bSetupPixelFormat(hwindowdc);
  GPUAssert(status,
            "Unable to set window pixel format");

  hglrc_window = wglCreateContext(hwindowdc);
  GPUAssert(hglrc_window,
            "Unable to create window GL context");

  status = wglMakeCurrent(hwindowdc, hglrc_window);
  GPUAssert(status,
            "Unable to make current the window GL context");

  /* Initialize gl functions */
  initglfunc();

  /* Append vendor specific attribs */
  appendVendorAttribs();

  /* Fetch the pixel formats for pbuffers */
  for (int i=0; i<4; i++) {
    status = wglChoosePixelFormatARB(hwindowdc, iAttribList[i],
                                     fAttribList[i], 1,
                                     pixelformat+i, &numFormats);
    
    GPUAssert(numFormats > 0 && status,
              "ChoosePixelFormat failed to find a pbuffer format");
  }

  /* Bind a 4 component pbuffer */
  hpbuffer = wglCreatePbufferARB(hwindowdc,
                                 pixelformat[3],
                                 2048, 2048,
                                 piAttribList[3]);

  if (!hpbuffer)
    GPUError("Failed to create float pbuffer");

  hpbufferdc = wglGetPbufferDCARB (hpbuffer);

  if (!hpbufferdc)
    GPUError("Failed to get pbuffer DC");

  hglrc = wglCreateContext( hpbufferdc );

  if (!hglrc)
    GPUError("Failed to create GL context");

  if (!wglMakeCurrent( hpbufferdc, hglrc ))
    GPUError("Failed to bind GL context");
}


void
OGLContext::bindPbufferWGL(unsigned int numComponents) {


  GPUAssert(hpbufferdc, "hpbufferdc = NULL");
  GPUAssert(numComponents != currentPbufferComponents,
            "Unnessesary call to bindPbuffer");
  GPUAssert(numComponents > 0 &&
            numComponents <= 4,
            "Cannot hand pbuffers other than 1-4 components");

  /* Tear down the old pbuffer */
  if (!wglMakeCurrent (hpbufferdc, NULL))
    GPUError("MakeCurrent Failed");

  if (!wglReleasePbufferDCARB (hpbuffer, hpbufferdc))
    GPUError("ReleasePbufferDC Failed");

  if (!wglDestroyPbufferARB (hpbuffer))
    GPUError("DestroyPbufferARB Failed");

  GPUAssert(hwindowdc, "hwindowdc = NULL");
  GPUAssert(pixelformat[numComponents-1], "Invalid pixelformat");
  GPUAssert(piAttribList[numComponents-1], "Invalid piAttribList");
  GPUAssert(2048, "Bogus 2048");

  /* Create a fresh pbuffer */
  hpbuffer = wglCreatePbufferARB(hwindowdc,
                                 pixelformat[numComponents-1],
                                 2048, 2048,
                                 piAttribList[numComponents-1]);

  if (!hpbuffer)
    GPUError("Failed to create pbuffer");
  
  hpbufferdc = wglGetPbufferDCARB (hpbuffer);
  
  if (!hpbufferdc)
    GPUError("Failed to get pbuffer dc");
  
  GPUAssert(hglrc, "Invalid glrc");
  
  if (!wglMakeCurrent( hpbufferdc, hglrc ))
    GPUError("Failed to make current GL context");
}

#else

void
OGLContext::initPbufferGLX(void) {
  GPUError("Yet to be implemented");
}


void
OGLContext::bindPbufferGLX(int ncomponents)
{
  GPUError("Yet to be implemented");
}

#endif


void
OGLContext::initPbuffer(void)
{
#ifdef WIN32
   initPbufferWGL();
#else
   initPbufferGLX();
#endif

   currentPbufferComponents = 4;
}


void
OGLContext::bindPbuffer(unsigned int numComponents)
{
  /* If the pbuffer of the right size is already active,
  ** return immediately
  */
  if (currentPbufferComponents == numComponents)
    return;

#ifdef WIN32
   bindPbufferWGL(numComponents);
#else
   bindPbufferGLX(numComponents);
#endif

   currentPbufferComponents = numComponents;
}
