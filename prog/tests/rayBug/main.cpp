/*
 * main.cpp --
 *
 *      C++ side initialization and support for a test of Brook's write
 *      query functionality.
 */

#include <stdlib.h>
#include <string.h>
#include <brook/brook.hpp>
#include "built/kernels.hpp"

static float4* gDebugCoreStates = NULL;
static float4* gDebugIntersectStates = NULL;

static void debugDump( int w, int h, brook::stream& coreStateStream, brook::stream& intersectStateStream )
{
   if( gDebugCoreStates == NULL )
   {
      gDebugCoreStates = new float4[ w * h ];
      gDebugIntersectStates = new float4[ w * h ];
   }

   coreStateStream.write( gDebugCoreStates );
   intersectStateStream.write( gDebugIntersectStates );

   printf( "core=<%f, %f, %f, %f> isect=<%f, %f, %f, %f>\n",
      gDebugCoreStates[0].x,
      gDebugCoreStates[0].y,
      gDebugCoreStates[0].z,
      gDebugCoreStates[0].w,
      gDebugIntersectStates[0].x,
      gDebugIntersectStates[0].y,
      gDebugIntersectStates[0].z,
      gDebugIntersectStates[0].w );
}

//KdTreeConditionalAccelerator::KdTreeConditionalAccelerator()
int main( int argc, char** argv )
{
   int doBroken = 1;

   if( argc > 1 )
      doBroken = atoi( argv[1] );

   using namespace brook;

   int height = 256;
   int width = 256;

   brook::initialize("dx9", NULL);

   brook::stream shadingHits =
      brook::stream::create<ShadingHit>(height, width);

   iter filmPos(::brook::__BRTFLOAT2,
                height, width, -1, 1.0f, 1.0f, -1.0f, -1.0f, -1);

   stream rays = stream::create<Ray>(height, width);
   stream hits = stream::create<float4>(height, width);

if( doBroken )
{
   krnGenEyeRays( filmPos, rays );
}

   brook::stream coreStateStream =
      stream::create<float4>(height, width);
   brook::stream intersectStateStream =
      stream::create<float4>(height, width);

   printf( "initialize\n" );
   krnKD_Initialize( coreStateStream, intersectStateStream );
   debugDump( width, height, coreStateStream, intersectStateStream );


   printf( "leaf\n" );
   krnKD_Leaf(
      coreStateStream, intersectStateStream,
      coreStateStream, intersectStateStream );
   debugDump( width, height, coreStateStream, intersectStateStream );

   return 0;
}