
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


#include "gpusanity.h"


// chosen to be safely under maximum exact integer representable by a
// 24 bit floating point value
#define MAXIMUM_VALUE  130000



void GenerateTestValues(float* testValues, size_t numEl) {
  for (size_t i=0; i<numEl; i++)
    testValues[i] = (float)(i % MAXIMUM_VALUE);
}


int randint(int maxValue) {
  return (rand() % (maxValue+1));
}



// Performs a number of stream copy tests on streams of varying size
// and number of components.  This is a deterministic set of
// tests. Use PerformRandomCopyTests to randomly test the system.
void PerformCopyTests(size_t maxSize) {
    
  int correct = 1;

  size_t testDataSize = 4 * maxSize * maxSize;
  float* testValues = new float[testDataSize];
  float* outputValues = new float[testDataSize];

  GenerateTestValues(testValues, testDataSize);



  // test various stream sizes for all numbers of components
  for (size_t components=1; components<=4; components++ )
    for (size_t sizey=1; sizey<=maxSize; sizey*=2)
      for (size_t sizex=1; sizex<=maxSize; sizex*=2)
	correct &= streamCopyTest(sizex, sizey, components, testValues, outputValues);
  


  // additional tests to include in the sanity check.  Adding some
  // non-power-of-two sizes here to increase test coverage also adding
  // some small sizes size I've seen those have issues as well due to
  // drivers neglecting to flush cache
  for (size_t components=1; components<=4; components++) {
    correct &= streamCopyTest(1023, 1023, components, testValues, outputValues);
    correct &= streamCopyTest(511, 1023, components, testValues, outputValues);
    correct &= streamCopyTest(511, 513, components, testValues, outputValues);
    correct &= streamCopyTest(255, 255, components, testValues, outputValues);
    correct &= streamCopyTest(255, 257, components, testValues, outputValues);
    correct &= streamCopyTest(31, 33, components, testValues, outputValues);
    correct &= streamCopyTest(3, 5, components, testValues, outputValues);
  }


  if (correct) {
    printf("All stream copy tests passed.\n");
    fflush(stdout);
  }


  delete [] testValues;
  delete [] outputValues;

}


// Performs 'numTests' random stream copy tests.  In each test,
// both the stream size and the number of components in a 
// stream element is randomly selected
void PerformRandomCopyTests(size_t maxSize, size_t numTests) {

  int correct = 1;

  size_t testDataSize = 4 * maxSize * maxSize;
  float* testValues = new float[testDataSize];
  float* outputValues = new float[testDataSize];

  GenerateTestValues(testValues, testDataSize);


  srand( time(NULL) );


  for (size_t i=0; i<numTests; i++) {

    int sizex = randint(maxSize);
    int sizey = randint(maxSize);
    int components = 1 + randint(3);

    correct &= streamCopyTest(sizex, sizey, components, testValues, outputValues);
  }


  if (correct) {
    printf("All random stream copy tests passed.\n");
    fflush(stdout);
  }

  delete [] testValues;
  delete [] outputValues;
}



// Performs a series of deterministic tests of multiple output kernels
// on streams of varying size and with varying number of outputs. For
// random testing, use PerformRandomMultiOutTests
void PerformMultiOutTests(size_t maxSize) {

  int correct = 1;

  size_t testDataSize = 4 * maxSize * maxSize;
  float* testValues = new float[testDataSize];

  GenerateTestValues(testValues, testDataSize);

  // test multiple outputs with float1's (bunch of pow-of-two sizes)
  for (size_t outputs=2; outputs<=4; outputs++)
    for (size_t size=1; size<=maxSize; size*=2)
      correct &= multiOutTest(size, size, 1, outputs, testValues);  

  // and a few non-pow-of-two sizes
  for (size_t outputs=2; outputs<=4; outputs++) {
    correct &= multiOutTest(255, 257, 1, outputs, testValues); 
    correct &= multiOutTest(511, 513, 1, outputs, testValues); 
  }
  
  // test multiple outputs with float4's
  for (size_t outputs=2; outputs<=4; outputs++)
    for (size_t size=1; size<=maxSize; size*=2)
      correct &= multiOutTest(size, size, 4, outputs, testValues);  

  // and again the non-pow-of-two sizes
  for (size_t outputs=2; outputs<=4; outputs++) {
    correct &= multiOutTest(255, 257, 4, outputs, testValues); 
    correct &= multiOutTest(511, 513, 4, outputs, testValues); 
  }
  


  if (correct) {
    printf("All multiple output tests passed.\n");
    fflush(stdout);
  }

  delete [] testValues;

}


// performs 'numTests' random multiple output tests
void PerformRandomMultiOutTests(size_t maxSize, size_t numTests) {

  int correct = 1;

  size_t testDataSize = 4 * maxSize * maxSize;
  float* testValues = new float[testDataSize];

  GenerateTestValues(testValues, testDataSize);

  for (size_t i=0; i<numTests; i++) {
    int sizex = randint(maxSize);
    int sizey = randint(maxSize);
    int components = (randint(1)) ? 1 : 4;
    int numOutputs = 2 + randint(2);

    correct &= multiOutTest(sizex, sizey, components, numOutputs, testValues);
  }


  if (correct) {
    printf("All random multiple output tests passed.\n");
    fflush(stdout);
  }

  delete [] testValues;

}



int main(int argc, char** argv) {

  bool doRandomTesting = false;
  size_t numRandomTests;
  
  size_t maxSize = 1024;


  if (argc>=3  && strcmp(argv[1], "-r")==0) {

    doRandomTesting = true;
    numRandomTests = atoi(argv[2]);

  } else if (argc > 1) {
    
    printf("Usage: %s [-r numTests]\n", argv[0]);
    printf("\n");
    printf("If no arguments are specified, will run a series of deterministic\n"
	   "regression tests.  Use -r for random testing.\n\n");
    return 1;

  }
  
  if (!doRandomTesting) {

    printf("Performing standard regressions... (max size = %d)\n", maxSize);
    fflush(stdout);

    PerformCopyTests(maxSize);
    PerformMultiOutTests(maxSize);

  } else {

    printf("Performing random testing... (%d tests, max size = %d)\n", numRandomTests, maxSize);
    fflush(stdout);

    PerformRandomCopyTests(maxSize, numRandomTests);
    PerformRandomMultiOutTests(maxSize, numRandomTests);

  }


  // <kayvonf> Future notes:  Additional tests to put here might include:
  //    - OGL style buffer ping-ponging with wglBindPbuffer (but how to get this in Brook)
  //    - Occlusion query correctness


  return 0;

}






