#ifndef OGLFUNC_HPP
#define OGLFUNC_HPP

#include "GL/GLee.h"
#ifdef WIN32
#include <windows.h>
#else
typedef void *HGLRC; 
#endif

#ifndef GL_VERSION_1_1
#error GL ERROR: The gl.h version on this computer is very old.
#endif

namespace brook {
  void initglfunc(void);
}

#endif

