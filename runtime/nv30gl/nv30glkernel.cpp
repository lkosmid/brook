
#include "nv30gl.hpp"

using namespace brook;

void
NV30GLKernel::BindParameter(const float x,
                            const float y, const float z, const float w)
{
   for (unsigned int i=0; i < npasses; i++) {
      glProgramNamedParameter4fNV(pass_id[i], strlen(constnames[creg]),
                                  (const GLubyte *) constnames[creg],
                                  x, y, z, w);
   }
   CHECK_GL();
}

void
NV30GLKernel::PushScaleBias(Stream *s) {

   /* Scale and bias are not used for FP30 */

   creg++;
}

void
NV30GLKernel::PushShape(Stream *s) {

   /* Shape is not used for FP30 */

   creg++;
}
