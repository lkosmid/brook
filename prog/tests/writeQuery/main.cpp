/*
 * main.cpp --
 *
 *      C++ side initialization and support for a test of Brook's write
 *      query functionality.
 */

#include <stdlib.h>
#include <string.h>
#include <brook/brook.hpp>
#include "built/writeKernels.hpp"

static int size = 400;

int
main(int argc, char *argv[])
{
   using namespace brook;

   if (getenv("BRT_RUNTIME") == NULL) {
      std::cout << "*Runtime unspecified, using dx9*" << std::endl;
      brook::initialize("dx9", NULL);
   }
   if (argc > 1) {
      size = strtol(argv[1], NULL, 0);
      std::cout << "*Using size of " << size << "x" << size << "*" << std::endl;
   }
   float threshold = (float) (size * size / 2);
   write_query q = write_query::create();
   write_mask m = write_mask::create(size, size);
   stream inStream = stream::create<float>(size, size);
   stream outStream = stream::create<float>(size, size);
   float *data;
   int numCopied, i, j;

   data = (float *) malloc(size * size * sizeof(float));
   for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++) {
         data[i * size + j] = (float) (i * size + j);
      }
   }
   inStream.read(data);

   /*
    * It takes a lot of calls to get all this done.  Such is life.  Here are
    * the rules, as I understand them:
    *
    *   1. The mask must be bound (duh).
    *   2. The mask must be writable even in order to clear it.
    *   3. The mask must be enabled in order for ATI cards to run
    *      write_queries correctly (specifically, if the depth test is
    *      disabled, any occlusion query reports that all fragments drew
    *      even if the shader kills some of them).
    *
    * So, we bind, clear, and enable the write_masking (just with an empty
    * mask) for the entire run so that we can use the write_queries.
    */

   m.bind();
   m.enableSet();
   m.clear();
   m.disableSet();
   m.enableTest();

   q.begin();
   krn_CopyStream(inStream, outStream);
   q.end();
   numCopied = q.count();
   std::cout << numCopied << " elements copied (expecting "
             << size * size << ")" << std::endl;

   m.enableSet();
   q.begin();
   krn_CopyBelowThresholdKill(inStream, threshold, outStream);
   q.end();
   numCopied = q.count();
   m.disableSet();
   std::cout << numCopied << " elements copied (expecting "
             << threshold << ")" << std::endl;

   q.begin();
   krn_CopyStream(inStream, outStream);
   q.end();
   numCopied = q.count();
   std::cout << numCopied << " elements copied (expecting "
             << size * size - threshold << ")" << std::endl;

   m.unbind();
   free(data);
   return 0;
}
