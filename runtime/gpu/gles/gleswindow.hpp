// gleswindow.hpp
#ifndef GLESWINDOW_HPP
#define GLESWINDOW_HPP

#ifdef WIN32
#include <windows.h>
#else
#ifdef RPI_NO_X
#include  "bcm_host.h"
#else
#include <X11/Xlib.h>
#endif
#endif

#include <EGL/egl.h>

#include "glesfunc.hpp"
#ifndef WIN32
typedef void *HGLRC; 
#endif
namespace brook {
  
  class GLESWindow {
    
  public:
    GLESWindow(const char* device);
    ~GLESWindow();
    
    void initFBO();
    
    bool bindFBO();

void swapBuffers();

    void makeCurrent();
    void shareLists(HGLRC inContext );
    unsigned int framebuffer()                    const { return fbo;       }

#ifdef RPI_NO_X
    EGLNativeWindowType RaspberryWinCreate(const char *title);
#endif
    
  private:
    unsigned int fbo;
	bool firstrun, fullscreen;
#ifdef WIN32
    HGLRC hglrc_window;
    HWND hwnd;
    HDC hwindowdc;
    DEVMODE settings;

#else

#ifndef RPI_NO_X
     Display   *pDisplay;
     int iScreen;
     Window     window;
     Colormap cmap;
     XVisualInfo *visual;
#else
     EGLNativeWindowType window;
#endif
     //GLXFBConfig *glxConfig[4];
     //GLXContext  glxContext;
     EGLSurface sEGLSurface;
     EGLContext sEGLContext;
     EGLDisplay sEGLDisplay;
     EGLConfig aEGLConfigs[1];
     EGLint cEGLConfigs;
#endif
    
  };

}


#endif

