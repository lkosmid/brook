#ifndef NV30GL_H
#define NV30GL_H

#include "glruntime.hpp"

#define NV30GL_MAXCONSTS        GL_MAX_CONSTS
#define NV30GL_MAX_TEXCOORDS    GL_MAX_TEXCOORDS

/*
 * This file is actually pulled in by glruntime.hpp because it has a bunch
 * of core GL definitions in addition to nv30 ones, but is left here too
 * because the multiple inclusion is harmless and this is actually its
 * logical point of inclusion.  --Jeremy.
 */
//#include "nv30glext.h"

namespace brook {

   class NV30GLRunTime;
   class NV30GLKernel;
   class NV30GLStream;
   class NV30GLIter;

   extern const char* NV30GL_RUNTIME_STRING;

   class NV30GLRunTime : public GLRunTime {
   public:
      NV30GLRunTime();

      Kernel *CreateKernel(const void*[]);
      Stream *CreateStream(int fieldCount, const __BRTStreamType fieldTypes[],
                           int dims, const int extents[]);
      Iter *CreateIter(__BRTStreamType type, int dims, int e[],float r[]);

     void createPBuffer(int ncomponents);

   protected:
#ifdef WIN32
     void createPBufferWGL(int ncomponents);
#else
     void createPBufferGLX(int ncomponents);
#endif
   };

   class NV30GLKernel : public GLKernel {
   public:
      NV30GLKernel(NV30GLRunTime *runtime, const void *[]);
      virtual ~NV30GLKernel() { /* Everything is done in ~GLKernel() */ };

      void Map();

   protected:
      void ReduceScalar();
      void ReduceStream();
  };

  class NV30GLStream : public GLStream {
  public:
     NV30GLStream (NV30GLRunTime * runtime, int fieldCount,
                   const __BRTStreamType type[],
                   int dims, const int extents[]);
     virtual ~NV30GLStream() { /* Everything is done in ~GLStream() */ };

     void GLReadData(void *src);
     void GLWriteData(const void *dst);
  };

  class NV30GLIter : public GLIter {
  public:
     NV30GLIter(NV30GLRunTime *runtime, __BRTStreamType type,
                 int dims, int extents[], float ranges[])
        : GLIter(runtime, type, dims, extents, ranges) {};
     virtual ~NV30GLIter() { /* GLIter::GLIter() does all the work */ };
  };
}
#endif
