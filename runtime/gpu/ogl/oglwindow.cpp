
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oglcontext"

using namespace brook;

#ifdef WIN32

static const char window_name[] = "Brook GL Render Window";

static HWND
create_window (void) {
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
  
  ppfd = &pfd;

  status = ChoosePixelFormat( hdc, ppfd );
  
  GPUAssert(status, "ChoosePixelFormat failed");
  
  status = SetPixelFormat(hdc, pixelformat, ppfd);
  
  GPUAssert(status, "SetPixelFormat failed");
  
  return TRUE;
}


void
OGLContext::initPbufferWGL(void) {

  unsigned int numFormats;
  BOOL status;

  /* Create a window */
  HWND hwnd = create_window();
  hwindowdc = GetDC(hwnd);

  /* Initialize the initial window GL_context */
  status = bSetupPixelFormat(hdc_window);
  GPUAssert(status,
            "Unable to set window pixel format");

  hglrc_window = wglCreateContext(hwindowdc);
  GPUAssert(hglrc_window,
            "Unable to create window GL context");

  status = wglMakeCurrent(hwindowdc, hglrc_window);
  GPUAssert(status,
            "Unable to make current the window GL context");

  /* Fetch the pixel formats for pbuffers */
  for (int i=0; i<4; i++) {
    status = wglChoosePixelFormatARB(hdc_window, iAttributes[i],
                                     fAttributes[i], 1,
                                     pixelformat+i, &numFormats);
    
    GPUAssert(numFormats > 0 && status,
              "ChoosePixelFormat failed to find a pbuffer format");
  }

  /* Bind a 4 component pbuffer */
  hpbuffer = wglCreatePbufferARB(hwindowdc,
                                 pixelformat[3],
                                 workspace, workspace,
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


  GPUAssert(hbufferdc, "hbufferdc = NULL");
  GPUAssert(numComponents != curretPbufferComponents,
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
  GPUAssert(pixelformat[numCompontents-1], "Invalid pixelformat");
  GPUAssert(piAttribList[numComponts-1], "Invalid piAttribList");
  GPUAssert(workspace, "Bogus workspace");

  /* Create a fresh pbuffer */
  hpbuffer = wglCreatePbufferARB(hwindowdc,
                                 pixelformat[numComponents-1],
                                 workspace, workspace,
                                 piAttribList[numComponents-1]);

  if (!hpbuffer)
    GPUError("Failed to create pbuffer");
  
  hpbufferdc = wglGetPbufferDCARB (hpbuffer);
  
  if (!hpbufferdc)
    GPUError("Failed to get pbuffer dc");
  
  GPUAssert(hpbufferglrc);
  
  if (!wglMakeCurrent( hpbufferdc, hpglrc ))
    GPUError("Failed to make current GL context");
}

#else

void
OGLContext::initPbufferGLX(void) {
  GPUError("Yet to be implemented");
}


void
GLRunTime::bindPBufferGLX(int ncomponents)
{
  GPUError("Yet to be implemented");
}

#endif


void
OGLContext:initPBuffer(void)
{
#ifdef WIN32
   initPBufferWGL();
#else
   initPBufferGLX();
#endif

   currentPbufferComponents = 4;
}


void
OGLContext:bindPBuffer(unsigned int numComponents)
{
  /* If the pbuffer of the right size is already active,
  ** return immediately
  */
  if (currentPBufferComponents == numComponents)
    return;

#ifdef WIN32
   bindPBufferWGL(numComponents);
#else
   bindPBufferGLX(numComponents);
#endif

   currentPbufferComponents = numComponents;
}
