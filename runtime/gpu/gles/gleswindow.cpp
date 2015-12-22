
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../gpucontext.hpp"

#include "glesfunc.hpp"
#include "gleswindow.hpp"
#include "glescheckgl.hpp"

using namespace brook;

static const char window_name[] = "Brook GL Render Window";

#ifdef WIN32

static HWND
create_window (int window_x, int window_y, bool fullscreen) {
  HINSTANCE hinstance;
  WNDCLASS wc;
  DWORD window_style;
  HWND hwnd;

  /*
   * These parameters are useless since the window is never shown nor
   * rendered into.
   */
  const int window_width = 640;
  const int window_height = 480;

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
  if(fullscreen) window_style |= WS_VISIBLE;

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

GLESWindow::GLESWindow(const char* device) {
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
                      fprintf(stderr, "BRT_ADAPTER chooses adapter gles:%s:%s\n", mi.szDevice, dd.DeviceString);
                      strcpy(driver, mi.szDevice);
                  }
                  break;
              }
          }
      }
      GPUAssert(winx!=CW_USEDEFAULT,
          "Failed to find monitor matching specified display");
  } else winx=winy=0;

  const char *fullscreenstr=getenv("BRT_FULLSCREEN");
  fullscreen=false;
  if(fullscreenstr && '1'==*fullscreenstr)
  {
      settings.dmSize=sizeof(DEVMODE);
      EnumDisplaySettings(*driver ? NULL : driver, ENUM_CURRENT_SETTINGS, &settings);
      settings.dmPelsWidth=640;
      settings.dmPelsHeight=480;
      if(DISP_CHANGE_SUCCESSFUL==ChangeDisplaySettingsEx(*driver ? NULL : driver, &settings,
          NULL, CDS_FULLSCREEN, NULL)) fullscreen=true;
  }

  /* Create a window */
  hwnd = create_window(winx, winy, fullscreen);
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


  initglesfunc();

  fbo = NULL;
}

GLESWindow::~GLESWindow() {
  wglMakeCurrent(hwindowdc, NULL);
  wglDeleteContext(hglrc_window);

  if (fbo)
    glDeleteFramebuffers(1, &fbo);

  DeleteDC(hwindowdc);
  if(hwnd) DestroyWindow(hwnd);
}


void 
GLESWindow::initFBO() {

  glGenFramebuffers(1, &fbo);
  CHECK_GL();
  firstrun=true;
}


bool
GLESWindow::bindFBO() {

  bool switched_contexts=false;
  BOOL status;

  if(firstrun || wglGetCurrentContext()!=hglrc_window) {
    status = wglMakeCurrent(hwindowdc, hglrc_window);
    GPUAssert(status,
              "Unable to make current the window GL context");

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
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

void GLESWindow::makeCurrent()
{
  wglMakeCurrent( hwindowdc, hglrc_window );
}

void GLESWindow::shareLists( HGLRC inContext )
{
  wglShareLists( hglrc_window, inContext );
}

#else

/* Linux version */

#include <X11/Xlib.h>

EGLint aEGLAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

EGLint aEGLContextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };


EGLConfig	aEGLConfigs[1];
EGLint		cEGLConfigs;


GLESWindow::GLESWindow(const char *device) {
//  int attrib[] = { GLX_RGBA, None };
  XSetWindowAttributes wa;
  char displaydevice[256];
  XVisualInfo temp;

  const char *colon=strchr(device, ':'), *endcolon=strrchr(device, ':');
  if(colon){
    if(!endcolon) endcolon=strchr(device, 0);
    strncpy(displaydevice, colon+1, endcolon-colon-1);
    displaydevice[endcolon-colon-1]=0;
  }
  XInitThreads();
  pDisplay = XOpenDisplay(colon ? displaydevice : NULL);
  if (pDisplay == NULL) {
    fprintf (stderr, "Could not connect to X Server at %s.\n", colon ? displaydevice : NULL);
    exit(1);
  }
  
  sEGLDisplay = EGL_CHECK(eglGetDisplay(pDisplay));
  
  EGL_CHECK(eglInitialize(sEGLDisplay, NULL, NULL));
  EGL_CHECK(eglChooseConfig(sEGLDisplay, aEGLAttributes, aEGLConfigs, 1, &cEGLConfigs));
  
  if (cEGLConfigs == 0) {
	  printf("No EGL configurations were returned.\n");
	  exit(-1);
  }

    
  iScreen  = DefaultScreen(pDisplay);

  EGLConfig FBConfig = aEGLConfigs[0];
  int vID,n;

  EGL_CHECK(eglGetConfigAttrib(sEGLDisplay, FBConfig, EGL_NATIVE_VISUAL_ID, &vID));
  temp.visualid = vID;
  visual = XGetVisualInfo(pDisplay, VisualIDMask, &temp, &n);
  if (!visual) {
		printf("Couldn't get X visual info\n");
		exit(-1);
  }

/*  visual = glXChooseVisual(pDisplay, iScreen, attrib);
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
  }*/

  cmap = XCreateColormap (pDisplay, 
                          RootWindow(pDisplay, iScreen),
                          visual->visual, AllocNone);
  wa.border_pixel = 0;
  wa.colormap = cmap;

  glxWindow = XCreateWindow(pDisplay,
                            RootWindow(pDisplay, iScreen),
                            0, 0, 1, 1, 0, visual->depth, InputOutput,
                            visual->visual, CWBorderPixel | CWColormap,
                            &wa);
/*  if (!glXMakeCurrent(pDisplay, glxWindow, glxContext)) {
    fprintf (stderr, "GLESWindow: Could not make current.\n");
    exit(1);
  }*/



//Copied from cube example. Are they required?
/*    XSetStandardProperties(display, window, title, title, None, 0, 0, &sh);
    XMapWindow(display, window);
    XIfEvent(display, &e, wait_for_map, (char*)&window);
    XSetWMColormapWindows(display, window, &window, 1);
    XFlush(display);
*/

  sEGLSurface = EGL_CHECK(eglCreateWindowSurface(sEGLDisplay, aEGLConfigs[0], (EGLNativeWindowType)glxWindow, NULL));
  if (sEGLSurface == EGL_NO_SURFACE) {
	  printf("Failed to create EGL surface.\n");
	  exit(-1);
  }
  
  sEGLContext = EGL_CHECK(eglCreateContext(sEGLDisplay, aEGLConfigs[0], EGL_NO_CONTEXT, aEGLContextAttributes));
  
  if (sEGLContext == EGL_NO_CONTEXT) {
        printf("Failed to create EGL context.\n");
        exit(-1);
  }

  EGL_CHECK(eglMakeCurrent(sEGLDisplay, sEGLSurface, sEGLSurface, sEGLContext));

  glFinish();

  initglesfunc();
}


void 
GLESWindow::initFBO() {

  glGenFramebuffers(1, &fbo);
  CHECK_GL();
  firstrun=true;
printf("Framebuffer created:%d\n", fbo);
}


bool
GLESWindow::bindFBO() {

  bool switched_contexts=false;

  if(firstrun || eglGetCurrentContext()!=sEGLContext) {
    //glXMakeCurrent(pDisplay, glxWindow, glxContext);
    EGL_CHECK(eglMakeCurrent(sEGLDisplay, sEGLSurface, sEGLSurface, sEGLContext));
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    CHECK_GL();
	
/*	GLenum iResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	CHECK_GL();
	if(iResult != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Framebuffer incomplete at %s:%i\n", __FILE__, __LINE__);
		abort();
	}*/

#if 0
    GLint fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fb);
    assert(fb==fbo);
#endif
    switched_contexts=true;
    firstrun=false;
  }
  else
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    CHECK_GL();
  }

printf("Bound Framebuffer:%d\n", fbo);
  
  return switched_contexts;

}

void
GLESWindow::swapBuffers() {
	EGL_CHECK(eglSwapBuffers(sEGLDisplay, sEGLSurface));
}

void GLESWindow::makeCurrent()
{
  EGL_CHECK(eglMakeCurrent(pDisplay, sEGLSurface, sEGLSurface, sEGLContext));
}

void shareLists( HGLRC inContext )
{
  GPUAssert( false, "Haven't implemented share lists under glX..." );
}

GLESWindow::~GLESWindow() 
{
  EGL_CHECK(eglMakeCurrent(sEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
  EGL_CHECK(eglDestroyContext(sEGLDisplay, sEGLContext));
  if (fbo)
    glDeleteFramebuffers(1, &fbo);
  XDestroyWindow(pDisplay, glxWindow);
  XFreeColormap(pDisplay, cmap);
  XFree(visual);
  XCloseDisplay(pDisplay);
}

void GLESWindow::shareLists( HGLRC inContext )
{
//FIXME
}

#endif

