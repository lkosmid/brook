#include "glruntime.hpp"

using namespace brook;

#ifdef WIN32
#define XXX(type, fn)   type fn;
RUNTIME_BONUS_GL_FNS
RUNTIME_BONUS_NV_FNS
#undef XXX
#endif

namespace brook {
   void initglfunc(void) {
#ifdef WIN32
#define   XXX(type, fn) fn = (type) wglGetProcAddress(#fn); assert(fn);
      RUNTIME_BONUS_GL_FNS;
      if (strstr((const char *)glGetString(GL_EXTENSIONS),
                 "NV_fragment_program")) {
         RUNTIME_BONUS_NV_FNS;
      }
#undef XXX   
#endif
   }
}

