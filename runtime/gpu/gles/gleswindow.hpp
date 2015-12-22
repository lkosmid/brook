// gleswindow.hpp
#ifndef GLESWINDOW_HPP
#define GLESWINDOW_HPP

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
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
    
  private:
    unsigned int fbo;
	bool firstrun, fullscreen;
#ifdef WIN32
    HGLRC hglrc_window;
    HWND hwnd;
    HDC hwindowdc;
    DEVMODE settings;

#else
     Display   *pDisplay;
     int iScreen;
     Window     glxWindow;
     Colormap cmap;
     XVisualInfo *visual;
     //GLXFBConfig *glxConfig[4];
     //GLXContext  glxContext;
     EGLSurface sEGLSurface;
     EGLContext sEGLContext;
     EGLDisplay sEGLDisplay;
#endif
    
  };

}


#endif

