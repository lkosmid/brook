// fractal.br
// tests kernel splitting of an iterative construct

#include <stdio.h>

void testFractal( int, int, int* );
void testParticleCloth( int, int, int* );
void testMatmult( int, int, int* );

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm")

int getTime()
{
  return (int)timeGetTime();
}

void main( int argc, char** argv )
{
  int time;

  int size = atoi( argv[1] );
  int count = atoi( argv[2] );

  testMatmult( size, count, &time );
//  testFractal( size, count, &time );
//  testParticleCloth( size, count, &time );

  printf( "test size=%d, count=%d, time=%d\n", size, count, time );
}
