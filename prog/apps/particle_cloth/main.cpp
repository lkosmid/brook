// main.cpp

#include "timing.hpp"

#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <string>
void cpuIterate( int, int, int& );
void gpuIterate( int, int, int* );
void moutIterate( int, int, int* );

int main( int argc, char** argv )
{
  SetupMillisTimer();

  int time;
  bool mout = false;

  if( argc < 4 )
  {
    std::cerr << "bad args" << std::endl;
    return 1;
  }

  int size = atoi( argv[2] );
  int iterations = atoi( argv[3] );

  if( strcmp( argv[1], "cpu" ) == 0 )
  {
    cpuIterate( size, iterations, time );
  }
  else if( strcmp( argv[1], "mout" ) == 0 )
  {
    moutIterate( size, iterations, &time );
	mout = true;
  }
  else
  {
    gpuIterate( size, iterations, &time );
  }

  double totalOps;

  double onewayBandwidth = 0;
  double twowayBandwidth = 0;

  if( mout )
  {
	  double n = (double)size;

	  // 26 ops per spring
	  double springOps = 2*n*(n-1)*26;
	  // 23 ops per step
	  double stepOps = n*n*23;
	  // 9 misc ops
	  double miscOps = n*n*9;
	  totalOps = (springOps + stepOps + miscOps) * iterations;

	  onewayBandwidth = 6*4*4*( n * n * iterations ) / (double)time;
	  twowayBandwidth = 2*5*4*4*( n * n * iterations ) / (double)time;
  }
  else
  {
	double n = (double)size;
	double stepOps = ((n*n - 2)*21+1);
	double windOps = (4*(n-1)*(n-1)*23);
	double springOps = (2*n*(n-1) + 2*(n-1)*(n-1) + 2*n*(n-2))*34;
	totalOps = ( stepOps + windOps + springOps ) * iterations;
  }
  double megaflops = totalOps / (double)time;

  std::cout << iterations << "\t";
  std::cout << time << "\t";
/*  if( mout )
  {
	  std::cout << "1way: " << onewayBandwidth << "\t";
	  std::cout << "2way: " << twowayBandwidth << "\t";
  }*/
  std::cout << megaflops << std::endl;



  CleanupMillisTimer();
  return 0;
}

