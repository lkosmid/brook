#ifndef NV30GL_H
#define NV30GL_H

#include "glruntime.hpp"

/*
 * This file is actually pulled in by glruntime.hpp because it has a bunch
 * of core GL definitions in addition to nv30 ones, but is left here too
 * because the multiple inclusion is harmless and this is actually its
 * logical point of inclusion.  --Jeremy.
 */
//#include "nv30glext.h"

namespace brook {
   extern const char *NV30GL_RUNTIME_STRING;

   class NV30GLRunTime : public GLRunTime {
   public:
      NV30GLRunTime() : GLRunTime() { /* All done in GLRunTime() */ }
      virtual ~NV30GLRunTime() { /* All done in GLRunTime() */ }

      Kernel *CreateKernel(const void *sourcelist[]);
   };

   class NV30GLKernel : public GLKernel {
   public:
      NV30GLKernel(NV30GLRunTime *runtime, const void *sourcelist[])
         : GLKernel(runtime) { Initialize(runtime, sourcelist, "fp30"); }
      virtual ~NV30GLKernel() { /* Everything is done in ~GLKernel() */ };


   protected:
      void BindParameter(const float x, const float y,
                         const float z, const float w);
  };
}
#endif
