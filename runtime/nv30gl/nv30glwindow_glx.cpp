
#include <assert.h>
#include <stdio.h>
#include <GL/gl.h>

#include "glext.h"
#include "glxext.h"

#include "nv30gl.hpp"

using namespace brook;

/*  The name of the window.  It is also the
**  window class name.
*/
const char window_name[] = "Brook NV30GL Render Window";

void
NV30GLRunTime::createWindow (void) {  
}


void
NV30GLRunTime::createWindowGLContext(void) {
}


void
NV30GLRunTime::createPBuffer (void) {

  Display   *pDisplay = XOpenDiplay(NULL); 
  int iScreen = DefaultScreen(pOldDisplay);
  
  GLXFBConfig *glxConfig;
  int iConfigCount;   
  
  int pfAttribList[] = 
    {
      GLX_RED_SIZE,               32,
      GLX_GREEN_SIZE,             32,
      GLX_BLUE_SIZE,              32,
      GLX_ALPHA_SIZE,             32,
      GLX_STENCIL_SIZE,           0,
      GLX_DEPTH_SIZE,             0,
      GLX_FLOAT_COMPONENTS_NV,    true,
      GLX_DRAWABLE_TYPE,          GLX_PBUFFER_BIT,
      0,
    };
  
  glxConfig = glXChooseFBConfigSGIX(pDisplay, 
                                    iScreen, 
                                    pfAttribList, 
                                    &iConfigCount);
  if (!glxConfig) {
    fprintf(stderr, "NV30GL:  glXChooseFBConfigSGIX() failed\n");
    exit(1);
  }
  
  int pbAttribList[] =  {
    GLX_LARGEST_PBUFFER, true,
    GLX_PRESERVED_CONTENTS, true,
    0,
  };
  
  glxPbuffer = glXCreateGLXPbufferSGIX(pDisplay, 
                                       glxConfig[0], 
                                       workspace, workspace,
                                       pbAttribList);
  
  if (!glxPbuffer) {
    fprintf(stderr, "NV30GL: glXCreatePbufferSGIX() failed\n");
    exit(1);
  }
  
  glxContext = glXCreateContextWithConfigSGIX(pDisplay, 
                                              glxConfig[0], 
                                              GLX_RGBA_TYPE, 
                                              0, 
                                              true);
  if (!glxConfig) {
    fprintf(stderr, "NV30GL: glXCreateContextWithConfigSGIX() failed\n");
    exit (1);
  }
  
  glXMakeCurrent(pDisplay, glxPbuffer, glxContext);
  
}

