#include "glruntime.hpp"

using namespace brook;

#ifdef WIN32
#define XXX(type, fn)   type fn;
RUNTIME_BONUS_GL_FNS
#undef XXX
#endif

void brook::initglfunc(void) {
#ifdef WIN32
#define   XXX(type, fn) fn = (type) wglGetProcAddress(#fn); assert(fn);
RUNTIME_BONUS_GL_FNS
#undef XXX
#endif
}

