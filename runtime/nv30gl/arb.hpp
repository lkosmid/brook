#ifndef __ARB_HPP__
#define __ARB_HPP__

#include "glruntime.hpp"

namespace brook {
   extern const char *ARB_RUNTIME_STRING;

   class ARBRunTime : public GLRunTime {
   public:
      ARBRunTime() : GLRunTime() { /* All done in GLRunTime() */ }
      virtual ~ARBRunTime() { /* All done in GLRunTime() */ }

      Kernel *CreateKernel(const void*[]);
   };

   class ARBKernel : public GLKernel {
   public:
      ARBKernel(ARBRunTime *runtime, const void *sourcelist[])
         : GLKernel(runtime) { Initialize(runtime, sourcelist, "arb"); }
      virtual ~ARBKernel() { /* Everything is done in ~GLKernel() */ };


   protected:
      void BindParameter(const float x, const float y,
                         const float z, const float w);
  };
}
#endif
