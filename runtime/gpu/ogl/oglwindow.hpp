// oglwindow.hpp

#pragma once

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include "wglext.h"
#endif



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
#endif
    
    int pixelformat[4];
    int piAttribList[4][16];
    unsigned int currentPbufferComponents;
  };

}
