#ifndef GLESFUNC_HPP
#define GLESFUNC_HPP

#ifdef WIN32
#include <windows.h>
#else
typedef void *HGLRC; 
#endif

#include <GLES2/gl2.h>

#ifndef GL_ES_VERSION_2_0
#error GL ERROR: OpenGL ES 2.0 is not supported on this system
#endif

namespace brook {
  void initglesfunc(void);
}

#endif

