// main.cpp

#include "timing.hpp"

#include <iostream>
#include <stdlib.h>
#include <iostream>

void cpuIterate( int, int, int64& );

int main( int argc, char** argv )
{
  SetupMillisTimer();

  int64 time;

  if( argc < 3 )
  {
    std::cerr << "bad args" << std::endl;
    return 1;
  }

  int size = atoi( argv[1] );
  int iterations = atoi( argv[2] );

  cpuIterate( size, iterations, time );

  double n = (double)size;
  double stepOps = ((n*n - 2)*21+1);
  double windOps = (4*(n-1)*(n-1)*23);
  double springOps = (6*n-8)*34;
  double totalOps = ( stepOps + windOps + springOps ) * iterations;
  double megaflops = totalOps / (double)time;

  std::cout << "particle cloth - cpu" << std::endl;
  std::cout << "size: " << size << std::endl;
  std::cout << "iterations: " << iterations << std::endl;
  std::cout << "time(microseconds): " << time << std::endl;
  std::cout << "megflop/s: " << megaflops << std::endl;

  CleanupMillisTimer();
  return 0;
}