

#ifndef __GPUSANITY_H__
#define __GPUSANITY_H__


int streamCopyTest(int sizex,
		   int sizey,
		   int components,
		   float* inputValues,
		   float* outputValues);


int multiOutTest(int sizex,
		 int sizey,
		 int components,
		 int numOutputs,
		 float* inputValues);


#endif // __GPUSANITY_H__
