/* benchmark.cpp
Gives a rough idea of the computing power of your machine
(C) 2007 Niall Douglas
Created: 23rd Nov 2007
*/

#include <stdio.h>
#include "brook/brook.hpp"

#define ITERATIONS 16

extern void  benchmark (::brook::stream out,
		::brook::stream in);

static void runBenchmark(float &t1, float &t2, float &t3, const char *runtime)
{
	using namespace brook;
	if(runtime)
		brook::initialize(runtime, NULL);
    {
	    stream out = stream::create<float4>(1024, 1024);
	    stream in = stream::create<float4>(1024, 1024);
        void *outputData=brmalloc(sizeof(float4)*1024*1024);
        brook::int64 start=brook::microseconds();
        for(int n=0; n<ITERATIONS; n++)
        {
            in.read(outputData);
            brook::finish();
        }
        brook::int64 afterwrite=brook::microseconds();
        for(int n=0; n<ITERATIONS; n++)
        {
    	    benchmark(out, in);
       	    brook::finish();
        }
        brook::int64 aftercalc=brook::microseconds();
        for(int n=0; n<ITERATIONS; n++)
        {
    	    out.write(outputData);
            brook::finish();
        }
        brook::int64 afterread=brook::microseconds();
        t2=(afterwrite-start)/1000000.0f/ITERATIONS;
        t1=(aftercalc-afterwrite)/1000000.0f/ITERATIONS;
        t3=(afterread-aftercalc)/1000000.0f/ITERATIONS;
        brfree(outputData);
    }
	brook::finalize();
}

float Go(const char *runtime=0)
{
    float t1, t2, t3;
	{
		printf("\nBenchmarking '%s' runtime ...\n", runtime);
        runBenchmark(t1, t2, t3, runtime);
		printf("Your GPU did around %f GFLOP/sec\n", 1024*1024*512/t1/1000000000);
		printf("CPU to GPU was %f Mb/sec\n", (1024*1024*sizeof(float4)/t2)/(1024*1024));
		printf("GPU to CPU was %f Mb/sec\n", (1024*1024*sizeof(float4)/t3)/(1024*1024));
	}
	return t1+t2;
}

int main(void)
{
    bool hasDX9=false;
	const char **devs=brook::runtimeTargets(), **dev;
	printf("Brook GPU benchmarker (C) 2007 Niall Douglas\n"
		   "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n"
		   "Available devices:\n\n");
	for(dev=devs; *dev; ++dev)
    {
		printf("%s\n", *dev);
        if(!strncmp(*dev, "dx9", 3)) hasDX9=true;
    }
#ifdef _OPENMP
    printf("\nCompiled with OpenMP support! ");
#pragma omp parallel
#pragma omp single nowait
    {
      printf("Using %d threads on %d processors ...\n", omp_get_num_threads(), omp_get_num_procs());
    }
#else
    printf("\nCompiled without OpenMP - single processor usage only\n");
#endif
    float cputime=0;
    printf("Time to write 4Mb of data, perform 2 billion operations and read 4Mb of data ...\n");
    for(dev=devs; *dev; ++dev)
    {
        float time=Go(*dev);
        if(dev==devs)
        {
            cputime=time;
            printf("Total time=%f secs.\n", time);
        }
        else
        {
            printf("Total time=%f secs. This is %f times faster than CPU backend\n", time, cputime/time);
        }
    }

	printf("Press Return to exit!\n");
	getchar();
	return 0;
}
