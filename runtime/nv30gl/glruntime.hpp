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

#ifdef WIN32
/*
 * On Windows we have to dynamically resolve these at runtime.  See
 * glfunc.cpp for the excitement.
 */
extern PFNWGLCREATEPBUFFERARBPROC      wglCreatePbufferARB;
extern PFNWGLGETPBUFFERDCARBPROC       wglGetPbufferDCARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC  wglChoosePixelFormatARB;
extern PFNWGLBINDTEXIMAGEARBPROC       wglBindTexImageARB;
extern PFNWGLRELEASETEXIMAGEARBPROC    wglReleaseTexImageARB;
extern PFNWGLRELEASEPBUFFERDCARBPROC   wglReleasePbufferDCARB;
extern PFNWGLDESTROYPBUFFERARBPROC     wglDestroyPbufferARB;
extern PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4fARB;
extern PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB;
extern PFNGLGENPROGRAMSNVPROC          glGenProgramsNV;
extern PFNGLLOADPROGRAMNVPROC          glLoadProgramNV;
extern PFNGLBINDPROGRAMNVPROC          glBindProgramNV;
extern PFNGLPROGRAMNAMEDPARAMETER4FNVPROC
                            glProgramNamedParameter4fNV;
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

   void initglfunc(void);

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
      void Map() = 0;
      void Reduce() {
         reduceType == __BRTSTREAM ? ReduceStream() : ReduceScalar();
         ResetStateMachine();
      }

   protected:
      void ResetStateMachine();
      virtual void ReduceScalar() = 0;
      virtual void ReduceStream() = 0;

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

      GLStream **outstream;
      bool     *argumentUsesIndexof;
      GLStream *sargs[GL_MAX_TEXCOORDS];
      GLIter   *iargs[GL_MAX_TEXCOORDS];
  };

  class GLStream : public Stream {
  public:
     GLStream (GLRunTime * runtime, int fieldCount,
               const __BRTStreamType type[],
               int dims, const int extents[]);
     virtual ~GLStream ();
     void Read(const void* inData);
     void Write(void* outData);

     void *getData (unsigned int flags);
     void releaseData(unsigned int flags);

     const unsigned int *getExtents() const { return extents; }
     unsigned int getDimension() const { return dims; }
     int getFieldCount() const { return nfields; }

     virtual __BRTStreamType getIndexedFieldType(int i) const {
       return (__BRTStreamType) ncomp[i];
     }

     virtual void GLReadData (void *p) = 0;
     virtual void GLWriteData (const void *p) = 0;

     GLStream *getNext(void) const { return next; }
     unsigned int getNumFields(void) const { return nfields; }

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

     virtual void createPBuffer(int ncomponents) = 0;
     void printMemUsage(void);

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
     HWND hwnd;
     HDC  hdc_window;
     HGLRC hglrc_window;
     HGLRC hpbufferglrc;
     HPBUFFERARB hpbuffer;
     HDC hpbufferdc;
#else
     Display    *pDisplay;
     GLXPbuffer  glxPbuffer;
     GLXContext  glxContext;
#endif

     void createWindow(void);
     void createWindowGLContext(void);
  };
}

#endif
