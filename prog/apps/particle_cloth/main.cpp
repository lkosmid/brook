// main.cpp

#include "timing.hpp"

#include <iostream>
#include <stdlib.h>
#include <iostream>

void cpuIterate( int, int, int& );
void gpuIterate( int, int, int* );

int main( int argc, char** argv )
{
  SetupMillisTimer();

  int time;

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
  else
  {
    gpuIterate( size, iterations, &time );
  }


  double n = (double)size;
  double stepOps = ((n*n - 2)*21+1);
  double windOps = (4*(n-1)*(n-1)*23);
  double springOps = (2*n*(n-1) + 2*(n-1)*(n-1) + 2*n*(n-2))*34;
  double totalOps = ( stepOps + windOps + springOps ) * iterations;
  double megaflops = totalOps / (double)time;

  std::cout << "particle cloth - " << argv[1] << std::endl;
  std::cout << "size: " << size << std::endl;
  std::cout << "iterations: " << iterations << std::endl;
  std::cout << "time(microseconds): " << time << std::endl;
  std::cout << "megflop/s: " << megaflops << std::endl;

  CleanupMillisTimer();
  return 0;
}