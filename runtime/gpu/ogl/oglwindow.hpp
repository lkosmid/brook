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

namespace brook {
  
  class OGLWindow {
    
  public:
    OGLWindow();
    ~OGLWindow();
    
    void initPbuffer( const int   (*viAttribList)[4][64],
                      const float (*vfAttribList)[4][16],
                      const int   (*vpiAttribList)[4][16]);
    
    void bindPbuffer(unsigned int numComponents);
    
  private:
    
#ifdef WIN32
    HGLRC hglrc;
    HGLRC hglrc_window;
    HPBUFFERARB hpbuffer;
    HWND hwnd;
    HDC hwindowdc;
    HDC hpbufferdc;
#else

     Display   *pDisplay;
     int iScreen;
     Window     glxWindow;
     Colormap cmap;
     XVisualInfo *visual;

     GLXFBConfig *glxConfig[4];
     GLXPbuffer  glxPbuffer;
     GLXContext  glxContext;
#endif
    
    int pixelformat[4];
    int piAttribList[4][16];
    unsigned int currentPbufferComponents;


  };

}


#endif

