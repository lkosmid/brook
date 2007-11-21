
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../gpucontext.hpp"

#include "oglfunc.hpp"
#include "oglwindow.hpp"
#include "oglcheckgl.hpp"

using namespace brook;

static const char window_name[] = "Brook GL Render Window";

#ifdef WIN32

static HWND
create_window (int window_x, int window_y) {
  HINSTANCE hinstance;
  WNDCLASS wc;
  DWORD window_style;
  HWND hwnd;

  /*
   * These parameters are useless since the window is never shown nor
   * rendered into.
   */
  const int window_width = 100;
  const int window_height = 100;

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

      ATOM result = RegisterClass( &wc );
      assert (result);
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

  //ShowWindow(hwnd, SW_SHOW);
  //MSG msg;
  //while(PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
  //    DispatchMessage(&msg);
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
  PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1 };
  int nvi, i, pixelformat;
  
  /* Work around driver problems in pixel formats */
  nvi=DescribePixelFormat(hdc,1,sizeof(PIXELFORMATDESCRIPTOR),&pfd);
  GPUAssert(nvi, "No pixel formats available");

  bool accelAvailable=false;
  for(i=1; i<=nvi; i++)
  {
     DescribePixelFormat(hdc,i,sizeof(PIXELFORMATDESCRIPTOR),&pfd);
     pfd.iLayerType=PFD_MAIN_PLANE;
     pixelformat = ChoosePixelFormat( hdc, &pfd );
     //fprintf(stderr, "%d: %d, 0x%x, OpenGL=%d, generic=%d, accel=%d, RGB=%d\n", i, pixelformat, pfd.dwFlags,
     //    !!(pfd.dwFlags&PFD_SUPPORT_OPENGL),
     //    !!(pfd.dwFlags&PFD_GENERIC_FORMAT),
     //    !!(pfd.dwFlags&PFD_GENERIC_ACCELERATED),
     //    !!(pfd.iPixelType==PFD_TYPE_RGBA));
     if(!(pfd.dwFlags&PFD_GENERIC_FORMAT)) accelAvailable=true;
  }
  // This happens when spanning displays over multiple cards
  if(!accelAvailable)
      fprintf(stderr, "Only generic non-accelerated pixel formats are available - are your graphic card drivers working?\n");
  for(i=1; i<=nvi; i++)
  {
     DescribePixelFormat(hdc,i,sizeof(PIXELFORMATDESCRIPTOR),&pfd);
     pfd.iLayerType=PFD_MAIN_PLANE;
     pixelformat = ChoosePixelFormat( hdc, &pfd );
	 if(pixelformat!=i)		// It be broke
		 continue;
     // Draw to window is required
     if(!(pfd.dwFlags&PFD_DRAW_TO_WINDOW)) continue;

     // OpenGL support is required
     if(!(pfd.dwFlags&PFD_SUPPORT_OPENGL)) continue;

     // Hardware acceleration only
     //if((pfd.dwFlags&PFD_GENERIC_FORMAT)) continue;

     // RGBA support is required
     if(pfd.iPixelType!=PFD_TYPE_RGBA) continue;

	 break;
  }
  GPUAssert(i<=nvi, "Couldn't find a suitable pixel format");

  status = SetPixelFormat(hdc, pixelformat, &pfd);
  GPUAssert(status, "SetPixelFormat failed");
 
  return TRUE;
}

static BOOL CALLBACK CollectHMonitors(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
  HMONITOR *monitors=(HMONITOR *) dwData;
  while(*monitors) monitors++;
  *monitors=hMonitor;
  return TRUE;
}

OGLWindow::OGLWindow(const char* device) {
  BOOL status;
  char driver[64]="";
  int winx=CW_USEDEFAULT, winy=CW_USEDEFAULT;

  /* Create a DC for the desired device */
  const char *colon=strchr(device, ':'), *dispvar = getenv("BRT_ADAPTER");
  if(colon || dispvar)
  {
      int adapter=-1;
      if(colon) {
          const char *colon2=strchr(colon+1, ':');
          strncpy(driver, colon+1, colon2-colon-1);
          driver[colon2-colon-1]=0;
      }
      else adapter=atoi(dispvar);
#if 1
      // ned: I just couldn't get CreateDC() to work with current drivers
      // (I think they need a window DC and can't handle a display DC)
      // so non-attached displays are not supported
      HMONITOR monitors[64];
      memset(monitors, 0, sizeof(monitors));
      EnumDisplayMonitors(NULL, NULL, CollectHMonitors, (LPARAM) monitors);

      for(HMONITOR *hmon=monitors; *hmon; hmon++)
      {
          MONITORINFOEX mi; mi.cbSize=sizeof(MONITORINFOEX);
          if(GetMonitorInfo(*hmon, &mi))
          {
              if(!adapter-- || !strcmp(driver, mi.szDevice))
              {
                  winx=mi.rcMonitor.left;
                  winy=mi.rcMonitor.top;
                  if(dispvar)
                  {
                      DISPLAY_DEVICE dd={sizeof(DISPLAY_DEVICE)};
                      for(int n=0; EnumDisplayDevices(NULL, n, &dd, NULL) && strcmp(dd.DeviceName, mi.szDevice); n++);
                      fprintf(stderr, "BRT_ADAPTER chooses adapter ogl:%s:%s\n", mi.szDevice, dd.DeviceString);
                  }
                  break;
              }
          }
      }
      GPUAssert(winx!=CW_USEDEFAULT,
          "Failed to find monitor matching specified display");
  } else winx=winy=0;

  /* Create a window */
  hwnd = create_window(winx, winy);
  hwindowdc = GetDC(hwnd);
#else
      // Niall's alternative which can use independent displays.
      // This code doesn't work properly either - any ideas?
      /*DEVMODE dm={0};
      dm.dmSize=sizeof(DEVMODE);
      // This is convoluted. If it's active, fetch its current settings
      if(!EnumDisplaySettingsEx(driver, ENUM_CURRENT_SETTINGS, &dm, 0))
          if(EnumDisplaySettingsEx(driver, ENUM_REGISTRY_SETTINGS, &dm, 0))

      if(EnumDisplaySettings(driver, 0, &dm))
          hwindowdc = CreateDC(driver, NULL, NULL, &dm);*/
      DISPLAY_DEVICE dd={sizeof(DISPLAY_DEVICE)};
      EnumDisplayDevices(driver, 0, &dd, 0);
      hwindowdc = CreateDC("DISPLAY", driver, NULL, NULL);
      hwnd=NULL;
  } else hwindowdc = CreateDC("DISPLAY", NULL, NULL, NULL);
#endif
  GPUAssert(hwindowdc,
            "Unable to create display context for specified device");

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


  initglfunc();

  fbo = NULL;
}

OGLWindow::~OGLWindow() {
  wglMakeCurrent(hwindowdc, NULL);
  wglDeleteContext(hglrc_window);

  if (fbo)
    glDeleteFramebuffersEXT(1, &fbo);

  DeleteDC(hwindowdc);
  if(hwnd) DestroyWindow(hwnd);
}


void 
OGLWindow::initFBO() {

  glGenFramebuffersEXT(1, &fbo);
  CHECK_GL();
  firstrun=true;
}


bool
OGLWindow::bindFBO() {

  bool switched_contexts=false;
  BOOL status;

  if(firstrun || wglGetCurrentContext()!=hglrc_window) {
    status = wglMakeCurrent(hwindowdc, hglrc_window);
    GPUAssert(status,
              "Unable to make current the window GL context");

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    CHECK_GL();

#if 0
    GLint fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fb);
    assert(fb==fbo);
#endif
	switched_contexts=true;
	firstrun=false;
  }
  
  return switched_contexts;
}

void OGLWindow::makeCurrent()
{
  wglMakeCurrent( hwindowdc, hglrc_window );
}

void OGLWindow::shareLists( HGLRC inContext )
{
  wglShareLists( hglrc_window, inContext );
}

#else

/* Linux version */

#include <X11/Xlib.h>

#define CRGBA(c, r,g,b,a) \
        GLX_RED_SIZE,               r, \
        GLX_GREEN_SIZE,             g, \
        GLX_BLUE_SIZE,              b, \
        GLX_ALPHA_SIZE,             a, \
        GLX_STENCIL_SIZE,           0, \
        GLX_DEPTH_SIZE,             0, \
        GLX_DRAWABLE_TYPE,          GLX_PBUFFER_BIT, \
        GLX_DOUBLEBUFFER,           0

OGLWindow::OGLWindow(const char *device) {
  int attrib[] = { GLX_RGBA, None };
  XSetWindowAttributes swa;
  char displaydevice[256];

  const char *colon=strchr(device, ':'), *endcolon=strrchr(device, ':');
  if(colon){
    if(!endcolon) endcolon=strchr(device, 0);
    strncpy(displaydevice, colon+1, endcolon-colon-1);
    displaydevice[endcolon-colon-1]=0;
  }
  pDisplay = XOpenDisplay(colon ? displaydevice : NULL);
  if (pDisplay == NULL) {
    fprintf (stderr, "Could not connect to X Server at %s.\n", colon ? displaydevice : NULL);
    exit(1);
  }
    
  iScreen  = DefaultScreen(pDisplay);

  visual = glXChooseVisual(pDisplay, iScreen, attrib);
  if (visual == NULL) {
    fprintf (stderr, "Could not create window visual.\n");
    exit(1);
  }

  glxContext = glXCreateContext(pDisplay, 
                                visual, 
                                0, GL_TRUE);  
  if (glxContext == NULL) {
    fprintf (stderr, "Could not create GL Context.\n");
    exit(1);
  }

  cmap = XCreateColormap (pDisplay, 
                          RootWindow(pDisplay, iScreen),
                          visual->visual, AllocNone);
  swa.border_pixel = 0;
  swa.colormap = cmap;

  glxWindow = XCreateWindow(pDisplay,
                            RootWindow(pDisplay, iScreen),
                            0, 0, 1, 1, 0, visual->depth, InputOutput,
                            visual->visual, CWBorderPixel | CWColormap,
                            &swa);
  if (!glXMakeCurrent(pDisplay, glxWindow, glxContext)) {
    fprintf (stderr, "OGLWindow: Could not make current.\n");
    exit(1);
  }

  glFinish();

  initglfunc();
}


void 
OGLWindow::initFBO() {

  glGenFramebuffersEXT(1, &fbo);
  CHECK_GL();
  firstrun=true;
}


bool
OGLWindow::bindFBO() {

  bool switched_contexts=false;

  if(firstrun || glXGetCurrentContext()!=glxContext) {
    glXMakeCurrent(pDisplay, glxWindow, glxContext);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    CHECK_GL();

#if 0
    GLint fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fb);
    assert(fb==fbo);
#endif
	switched_contexts=true;
	firstrun=false;
  }
  
  return switched_contexts;

}

void OGLWindow::makeCurrent()
{
  glXMakeCurrent(pDisplay, glxWindow, glxContext);
}

void shareLists( HGLRC inContext )
{
  GPUAssert( false, "Haven't implemented share lists under glX..." );
}

OGLWindow::~OGLWindow() 
{
  glXDestroyContext(pDisplay, glxContext);
  if (fbo)
    glDeleteFramebuffersEXT(1, &fbo);
  XDestroyWindow(pDisplay, glxWindow);
  XFreeColormap(pDisplay, cmap);
  XFree(visual);
  XCloseDisplay(pDisplay);
}

void OGLWindow::shareLists( HGLRC inContext )
{
//FIXME
}

#endif

