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
PFNGLPROGRAMNAMEDPARAMETER4FNVPROC  brook::glProgramNamedParameter4fNV;
PFNGLMULTITEXCOORD2FARBPROC     brook::glMultiTexCoord2fARB;

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
   XXX(PFNGLPROGRAMNAMEDPARAMETER4FNVPROC, glProgramNamedParameter4fNV);
   XXX(PFNGLMULTITEXCOORD2FARBPROC,    glMultiTexCoord2fARB);
}

