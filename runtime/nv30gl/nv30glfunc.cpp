#include "nv30gl.hpp"

using namespace brook;

PFNWGLCREATEPBUFFERARBPROC      brook::wglCreatePbufferARB;
PFNWGLGETPBUFFERDCARBPROC       brook::wglGetPbufferDCARB;
PFNWGLCHOOSEPIXELFORMATARBPROC  brook::wglChoosePixelFormatARB;
PFNWGLBINDTEXIMAGEARBPROC       brook::wglBindTexImageARB;
PFNWGLRELEASETEXIMAGEARBPROC    brook::wglReleaseTexImageARB;
PFNGLACTIVETEXTUREARBPROC       brook::glActiveTextureARB;
PFNGLGENPROGRAMSNVPROC          brook::glGenProgramsNV;
PFNGLLOADPROGRAMNVPROC          brook::glLoadProgramNV;
PFNGLBINDPROGRAMNVPROC          brook::glBindProgramNV;

#define   XXX(B,A) A = (B) wglGetProcAddress(#A); assert(A);

void brook::initglfunc(void) {
   XXX(PFNWGLCREATEPBUFFERARBPROC,     wglCreatePbufferARB);
   XXX(PFNWGLGETPBUFFERDCARBPROC,      wglGetPbufferDCARB);
   XXX(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB);
   XXX(PFNWGLBINDTEXIMAGEARBPROC,      wglBindTexImageARB);
   XXX(PFNWGLRELEASETEXIMAGEARBPROC,   wglReleaseTexImageARB);
   XXX(PFNGLACTIVETEXTUREARBPROC,      glActiveTextureARB);
   XXX(PFNGLGENPROGRAMSNVPROC,         glGenProgramsNV);
   XXX(PFNGLLOADPROGRAMNVPROC,         glLoadProgramNV);
   XXX(PFNGLBINDPROGRAMNVPROC,         glBindProgramNV);
}

