#include "arb.hpp"

#include <iostream>

using namespace brook;

void
ARBKernel::BindParameter(const float x, const float y,
                         const float z, const float w)
{
   std::cerr << "ARB: Binding (" << x << ", " << y << ", " << z << ", " << w
              << ") to parameter #" << creg << "\n";
   for (unsigned int i=0; i < npasses; i++) {
      //glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, pass_id[i]);
      glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, creg, x, y, z, w);
   }
   CHECK_GL();
}
