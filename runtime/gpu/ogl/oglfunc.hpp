#ifndef OGLFUNC_HPP
#define OGLFUNC_HPP

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

#ifndef GL_VERSION_1_1
#error "GL ERROR: The gl.h version on this computer is quite old."
#endif

#define RUNTIME_BONUS_GL_FNS

#ifndef GL_VERSION_1_2
typedef void (APIENTRYP PFNGLMULTITEXCOORD2FVPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRYP PFNGLMULTITEXCOORD4FVPROC) (GLenum target, const GLfloat *v);
#define RUNTIME_BONUS_GL_FNS RUNTIME_BONUS_GL_FNS \
   XXX(PFNGLMULTITEXCOORD2FVARBPROC,   glMultiTexCoord2fvARB) \
   XXX(PFNGLMULTITEXCOORD4FVARBPROC,   glMultiTexCoord4fvARB)
#endif

#ifndef GL_ARB_multitexture
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
#define GL_TEXTURE0                       0x84C0
#define RUNTIME_BONUS_GL_FNS RUNTIME_BONUS_GL_FNS \
   XXX(PFNGLACTIVETEXTUREARBPROC,      glActiveTextureARB)
#endif

#ifndef GL_ARB_vertex_program
typedef void (APIENTRYP PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRYP PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (APIENTRYP PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (APIENTRYP PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
#define GL_VERTEX_PROGRAM_ARB             0x8620
#define RUNTIME_BONUS_GL_FNS RUNTIME_BONUS_GL_FNS \
   XXX(PFNGLGENPROGRAMSARBPROC,        glGenProgramsARB)               \
   XXX(PFNGLBINDPROGRAMARBPROC,        glBindProgramARB)               \
   XXX(PFNGLPROGRAMSTRINGARBPROC,      glProgramStringARB)             \
   XXX(PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, glProgramLocalParameter4fvARB)
#endif

#ifndef GL_ARB_fragment_program
#define GL_FRAGMENT_PROGRAM_ARB           0x8804
#endif

#define RUNTIME_BONUS_WGL_FNS \
   XXX(PFNWGLCREATEPBUFFERARBPROC,     wglCreatePbufferARB)            \
   XXX(PFNWGLGETPBUFFERDCARBPROC,      wglGetPbufferDCARB)             \
   XXX(PFNWGLRELEASEPBUFFERDCARBPROC,  wglReleasePbufferDCARB)         \
   XXX(PFNWGLDESTROYPBUFFERARBPROC,    wglDestroyPbufferARB)           \
   XXX(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB)        \
   XXX(PFNWGLBINDTEXIMAGEARBPROC,      wglBindTexImageARB)             \
   XXX(PFNWGLRELEASETEXIMAGEARBPROC,   wglReleaseTexImageARB)          \
#endif


/* Declare undefined functions */
#define XXX(type, fn) \
   extern type fn;

#ifdef WIN32
RUNTIME_BONUS_WGL_FNS
#endif

RUNTIME_BONUS_GL_FNS

#undef XXX

namespace brook {

  void initglfunc(void);

}


#endif

