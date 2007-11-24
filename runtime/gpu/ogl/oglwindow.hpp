// oglwindow.hpp
#ifndef OGLWINDOW_HPP
#define OGLWINDOW_HPP

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

#include "oglfunc.hpp"
#ifndef WIN32
typedef void *HGLRC; 
#endif
namespace brook {
  
  class OGLWindow {
    
  public:
    OGLWindow(const char* device);
    ~OGLWindow();
    
    void initFBO();
    
    bool bindFBO();

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
     GLXFBConfig *glxConfig[4];
     GLXContext  glxContext;
#endif
    
  };

}


#endif

