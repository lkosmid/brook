/*
 * main.cpp --
 *
 *      C++ side initialization and support for a test of Brook's write
 *      query functionality.
 */

#include <brook/brook.hpp>
#include "built/writeKernels.hpp"

static const int size = 100;
static const float threshold = 100.0f;

int
main(int argc, char *argv[])
{
   using namespace brook;

   write_query q = write_query::create();
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

   q.begin();
   krn_CopyStream(inStream, outStream);
   q.end();
   numCopied = q.count();
   std::cout << numCopied << " elements copied" << std::endl;

   q.begin();
   krn_CopyBelowThreshold(inStream, threshold, outStream);
   q.end();
   numCopied = q.count();
   std::cout << numCopied << " elements copied" << std::endl;
   return 0;
}
