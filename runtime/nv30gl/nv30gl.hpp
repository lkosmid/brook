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

   extern const char *NV30GL_RUNTIME_STRING;
   extern const char *ARB_RUNTIME_STRING;

   class NV30GLRunTime : public GLRunTime {
   public:
      NV30GLRunTime();

      Kernel *CreateKernel(const void*[]);
   };

   class NV30GLKernel : public GLKernel {
   public:
      NV30GLKernel(NV30GLRunTime *runtime, const void *sourcelist[])
         : GLKernel(runtime, sourcelist) { /* All done in GLKernel() */ }
      virtual ~NV30GLKernel() { /* Everything is done in ~GLKernel() */ };


   protected:
      void BindParameter(const float x, const float y,
                         const float z, const float w);
  };
}
#endif
