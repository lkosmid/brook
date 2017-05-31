#if !defined(__GNUC__) || __GNUC__ != 2
#include <ios>
#endif
#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <stdio.h>

#include "glescheckgl.hpp"
#include "glesfunc.hpp"

static const unsigned int nglErrors = 6;

static const char glError_txt[][32] = {
    "GL_INVALID_ENUM",
    "GL_INVALID_VALUE",
    "GL_INVALID_OPERATION",
    "GL_STACK_OVERFLOW",
    "GL_STACK_UNDERFLOW",
    "GL_OUT_OF_MEMORY",
};


static bool checkFramebufferStatus() {
    GLenum status;
    status=(GLenum)glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            return true;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            printf("Framebuffer incomplete,incomplete attachment\n");
            return false;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("Unsupported framebuffer format\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            printf("Framebuffer incomplete,missing attachment\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            printf("Framebuffer incomplete,attached images must have same dimensions\n");
            return false;
        /*case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
             printf("Framebuffer incomplete,attached images must have same format\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            printf("Framebuffer incomplete,missing draw buffer\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            printf("Framebuffer incomplete,missing read buffer\n");
            return false;*/
    }
    printf("Unknown Framebuffer error\n");
	return false;
}

void brook::__check_gles(int line, const char *file, bool fatal) {
  GLenum r = glGetError();

#if defined(WIN32) && defined(_DEBUG) && 0
  static int lastline;
  static char *lastfile;
  static LARGE_INTEGER lastcount, tickspersec;
  LARGE_INTEGER thiscount;
  if(!tickspersec.QuadPart)
	  QueryPerformanceFrequency(&tickspersec);
  glFinish();
  QueryPerformanceCounter(&thiscount);
  double diff=((thiscount.QuadPart-lastcount.QuadPart)/(double) tickspersec.QuadPart)*1000;
  printf("Time %s:%d => %s:%d = %f ms\n", lastfile, lastline, file, line, diff);
  lastline=line;
  lastfile=file;
  lastcount=thiscount;
#endif

  
  if (r != GL_NO_ERROR) {
    if (r - GL_INVALID_ENUM >= nglErrors)
      std::cerr << "GL: Unknown GL error on line "
                << line << " of " << file << "\n";
    else
      std::cerr << "GL: glGetError returned "
                << glError_txt[r - GL_INVALID_ENUM]
                << " on line "<< line << " of " << file << "\n";
    checkFramebufferStatus();
    if(fatal)
       assert (r==GL_NO_ERROR);
  }
  return;
}
