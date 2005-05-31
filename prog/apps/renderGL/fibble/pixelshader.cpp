// pixelshader.cpp

#include "core.hpp"
#include "pixelshader.hpp"


#ifdef __cplusplus
extern "C" {
#endif

#define GL_FRAGMENT_PROGRAM_ARB                     0x8804
#define GL_PROGRAM_FORMAT_ASCII_ARB                 0x8875


typedef void (APIENTRY * PFNGLPROGRAMSTRINGARBPROC)
             (GLenum target, GLenum format, GLsizei len, const void *string);
typedef void (APIENTRY * PFNGLBINDPROGRAMARBPROC)
             (GLenum target, GLuint program);
typedef void (APIENTRY * PFNGLDELETEPROGRAMSARBPROC)
             (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLGENPROGRAMSARBPROC)
             (GLsizei n, GLuint *programs);

#ifdef __cplusplus
}
#endif

#define GETPROCADDR(type, name) {                                          \
      name = (type) wglGetProcAddress(#name);                              \
      if (!name) {std::cerr << "Could Not Load " << #name << std::endl;}   \
   }

#define GL_SYMS                                                 \
   SYM(PFNGLGENPROGRAMSARBPROC,         glGenProgramsARB)       \
   SYM(PFNGLPROGRAMSTRINGARBPROC,       glProgramStringARB)     \
   SYM(PFNGLBINDPROGRAMARBPROC,         glBindProgramARB)       \
   SYM(PFNGLDELETEPROGRAMSARBPROC,      glDeleteProgramsARB)

#define SYM(type, name) static type name;
GL_SYMS
#undef SYM

using namespace fibble;

PixelShader::PixelShader( Context* inContext, const std::string& inSource )
{
   static bool symsLoaded;

   if (!symsLoaded) {
      symsLoaded = true;
#define SYM(type, name) GETPROCADDR(type, name);
      GL_SYMS
#undef SYM
   }

   glGenProgramsARB(1, &_id);
   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _id);
   glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                      inSource.size(), inSource.c_str());
   if (glGetError() != GL_NO_ERROR) {
      std::cerr << "PixelShader: GL error: " << glGetError() << std::endl;
      exit(1);
   }
}

PixelShader::~PixelShader()
{
   glDeleteProgramsARB(1, &_id);
   if (glGetError() != GL_NO_ERROR) {
      std::cerr << "~PixelShader: GL error: " << glGetError() << std::endl;
      exit(1);
   }
   _id = 0;
}

void
PixelShader::bind( void )
{
   glEnable(GL_FRAGMENT_PROGRAM_ARB);
   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, _id);
   if (glGetError() != GL_NO_ERROR) {
      std::cerr << "PixelShader::bind() GL error: " << glGetError() << std::endl;
      exit(1);
   }
}
