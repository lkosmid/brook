/*
 * glruntime.hpp -
 *
 *      The ARB and NV30GL (and perhaps a future ATI specific) runtimes are
 *      both largely derived from an generic OpenGL runtime infrastructure.
 *      This file is the interface for that infrastructure.
 */

#ifndef _GLRUNTIME_HPP_
#define _GLRUNTIME_HPP_

#include "../runtime.hpp"

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include "wglext.h"
#else
#include <X11/Xlib.h>
#define GL_GLEXT_LEGACY
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#endif

#define GL_MAX_TEXCOORDS        8
#define GL_MAX_CONSTS           128

/*
 * Despite its name, this file includes a whole bunch of non-nv30
 * definitions required for various GL calls.
 */
#include "nv30glext.h"

/*
 * Plug some gaps.  The EXT_texture_rectangle extension hasn't made it into
 * the header files we use yet.
 */
#define GL_TEXTURE_RECTANGLE_EXT   GL_TEXTURE_RECTANGLE_NV

#ifdef WIN32
/*
 * For some inexplicable reason, on Windows hosts, the OpenGL libraries
 * don't actually export some of their symbols.  Instead, you have to use
 * wglGetProcAddress() to pry them out.  This doesn't stop their header
 * files from defining said symbols, but you get awkward compiler complaints
 * if you actually allow them to be defined.  So, we fake it all here
 * instead of defining either WGL_WGLEXT_PROTOTYPES or GL_GLEXT_PROTOTYPES.
 */

#define RUNTIME_BONUS_GL_FNS \
   XXX(PFNWGLCREATEPBUFFERARBPROC,     wglCreatePbufferARB)            \
   XXX(PFNWGLGETPBUFFERDCARBPROC,      wglGetPbufferDCARB)             \
   XXX(PFNWGLRELEASEPBUFFERDCARBPROC,  wglReleasePbufferDCARB)         \
   XXX(PFNWGLDESTROYPBUFFERARBPROC,    wglDestroyPbufferARB)           \
   XXX(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB)        \
   XXX(PFNWGLBINDTEXIMAGEARBPROC,      wglBindTexImageARB)             \
   XXX(PFNWGLRELEASETEXIMAGEARBPROC,   wglReleaseTexImageARB)          \
   XXX(PFNGLMULTITEXCOORD2FARBPROC,    glMultiTexCoord2fARB)           \
   XXX(PFNGLMULTITEXCOORD4FARBPROC,    glMultiTexCoord4fARB)           \
   XXX(PFNGLACTIVETEXTUREARBPROC,      glActiveTextureARB)             \
                                                                       \
   XXX(PFNGLGENPROGRAMSARBPROC,        glGenProgramsARB)               \
   XXX(PFNGLBINDPROGRAMARBPROC,        glBindProgramARB)               \
   XXX(PFNGLPROGRAMSTRINGARBPROC,      glProgramStringARB)             \
   XXX(PFNGLPROGRAMLOCALPARAMETER4FARBPROC, glProgramLocalParameter4fARB) \
                                                                       \
   XXX(PFNGLGENPROGRAMSNVPROC,         glGenProgramsNV)                \
   XXX(PFNGLLOADPROGRAMNVPROC,         glLoadProgramNV)                \
   XXX(PFNGLBINDPROGRAMNVPROC,         glBindProgramNV)                \
   XXX(PFNGLPROGRAMNAMEDPARAMETER4FNVPROC, glProgramNamedParameter4fNV)\

#define XXX(type, fn) \
   extern type fn;

RUNTIME_BONUS_GL_FNS
#undef XXX
#endif

namespace brook {

   class GLRunTime;
   class GLKernel;
   class GLStream;
   class GLIter;

   typedef struct{
      float x, y;
   } glfloat2;

   typedef struct{
      float x, y, z;
   } glfloat3;

   typedef struct{
      float x, y, z, w;
   } glfloat4;

   // Is there a GL_RG?  I'm not sure that LUMINANCE_ALPHA is correct here
   const GLenum GLformat[] = {0, GL_RED, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};

   void __check_gl(int line, char *file);
#define CHECK_GL() __check_gl(__LINE__, __FILE__);

#ifdef WIN32
   void initglfunc(void);
#endif

   class GLKernel : public Kernel {
   public:
      GLKernel(GLRunTime * runtime, const void *[]);
      virtual ~GLKernel();

      void PushStream(Stream *s);
      void PushIter(Iter *s);
      void PushConstant(const float &val);
      void PushConstant(const float2 &val);
      void PushConstant(const float3 &val);
      void PushConstant(const float4 &val);
      void PushReduce(void * val, __BRTStreamType type);
      void PushGatherStream(Stream *s);
      void PushOutput(Stream *s);
      void Map();
      void Reduce() {
         reduceType == __BRTSTREAM ? ReduceStream() : ReduceScalar();
         ResetStateMachine();
      }

   protected:
      virtual void BindParameter(const float x, const float y,
                                 const float z, const float w) = 0;
      void ResetStateMachine();
      void ReduceScalar();
      void ReduceStream();
      void RecomputeTexCoords(unsigned int w, unsigned int h,
                              glfloat4 f1[], glfloat4 f2[]);
      void IssueMapTexCoords(glfloat4 f1[], glfloat4 f2[]);

      GLRunTime *runtime;

      GLuint *pass_id;
      unsigned int *pass_out;
      unsigned int npasses;

      unsigned int nout;
      int sreg, treg, creg;
      int argcount;

      void *reduceVal;
      __BRTStreamType reduceType;
      int  reduceTreg, reduceSreg;
      GLStream *inputReduceStream;
      int  inputReduceStreamTreg, inputReduceStreamSreg;

      GLStream *tmpReduceStream[5];
      GLStream *sreg0;

      char **constnames;

      GLStream **outstreams;
      bool     *argumentUsesIndexof;
      GLStream *sargs[GL_MAX_TEXCOORDS];
      GLIter   *iargs[GL_MAX_TEXCOORDS];
  };

  class GLStream : public Stream {
  public:
     GLStream(GLRunTime * runtime, int fieldCount,
              const __BRTStreamType type[], int dims, const int extents[]);
     virtual ~GLStream ();
     void Read(const void *inData);
     void Write(void *outData);

     void *getData (unsigned int flags);
     void releaseData(unsigned int flags);

     const unsigned int *getExtents() const { return extents; }
     unsigned int getDimension() const { return dims; }
     int getFieldCount() const { return nfields; }

     virtual __BRTStreamType getIndexedFieldType(int i) const {
       return (__BRTStreamType) ncomp[i];
     }

     GLStream *getNext(void) const { return next; }

     int printMemUsage(void);

     unsigned int width, height;
     unsigned int *ncomp;
     GLuint *id;

  protected:
     __BRTStreamType elementType;
     unsigned int extents[3];
     unsigned int dims;
     unsigned int *stride;
     unsigned int nfields;
     unsigned int elemsize;
     void *cacheptr;

     GLStream *prev;
     GLStream *next;

     GLRunTime *runtime;
  };

  class GLIter : public Iter {
  public:
     GLIter (GLRunTime * runtime,
             __BRTStreamType type,
             int dims,
             int extents[],
             float ranges[]);
     ~GLIter();

     void *getData (unsigned int flags);
     void releaseData(unsigned int flags);
     const unsigned int *getExtents() const {return extents;}
     unsigned int getDimension() const {return dims;}

     unsigned int dims;
     float4 min, max;

  protected:
     unsigned int width, height;
     unsigned int ncomp;
     unsigned int extents[3];
     float *cacheptr;
     GLRunTime *runtime;
  };

  class GLRunTime : public RunTime {
  public:
     GLRunTime();

     Stream *CreateStream(int fieldCount, const __BRTStreamType fieldTypes[],
                          int dims, const int extents[]) {
        return new GLStream(this, fieldCount, fieldTypes, dims, extents);
     }

     Iter *CreateIter(__BRTStreamType type, int dims, int extents[],float r[]) {
        return new GLIter(this, type, dims, extents, r);
     }

     void createPBuffer(int ncomponents);
     void printMemUsage();

     enum WORKSPACESIZE {
        workspace = 2048
        //visual C++ 6.0 doesn't know about static const int
     };

     unsigned int pbuffer_ncomp;
     GLuint passthrough_id;

     /*
      * This is only accessible so that the GLStream constructor can update
      * it.  Otherwise, hands off!
      */
     GLStream *streamlist;

  protected:

#ifdef _WIN32
     void createPBufferWGL(int ncomponents);
     void createWindow(void);
     void createWindowGLContext(void);

     HWND hwnd;
     HDC  hdc_window;
     HGLRC hglrc_window;
     HGLRC hpbufferglrc;
     HPBUFFERARB hpbuffer;
     HDC hpbufferdc;
#else
     void createPBufferGLX(int ncomponents);
     Display    *pDisplay;
     GLXPbuffer  glxPbuffer;
     GLXContext  glxContext;
#endif
  };
}

#endif
