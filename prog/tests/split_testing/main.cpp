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

void main()
{
  int time;

  int size = 512;
  int count = 1000;

  testMatmult( size, count, &time );
//  testFractal( size, count, &time );
//  testParticleCloth( size, count, &time );

  printf( "test size=%d, count=%d, time=%d\n", size, count, time );
}
