#include "arb.hpp"

#include <iostream>

using namespace brook;

void
ARBKernel::BindParameter(const float x, const float y,
                         const float z, const float w)
{
#if 0
   std::cerr << "ARB: Binding (" << x << ", " << y << ", " << z << ", " << w
              << ") to parameter #" << creg << "\n";
#endif
   for (unsigned int i=0; i < npasses; i++) {
      /*
       * This rebinding appears to be mandatory.  At least, without it, we
       * fail the 'constant' regression test. --Jeremy.
       */
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[i]);
      glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, creg, x, y, z, w);
   }
   CHECK_GL();
}

void
ARBKernel::PushScaleBias(Stream *s) {
   BindParameter(1.0f, 1.0f, 0.001f, 0.001f);
   creg++;
}


void
ARBKernel::PushShape(Stream *s) {
   if (runtime->arch == ARCH_NV30) {
      BindParameter(1.0f, 1.0f, 0.0f, 0.0f);
   } else {
      BindParameter(1.0f, 1.0f, -0.5f, -0.5f);
   }
   creg++;
}
