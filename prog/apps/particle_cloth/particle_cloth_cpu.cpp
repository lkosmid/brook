// particle_cloth_cpu.cpp
// a simple direct implementation
// of the cloth-simulation algorithm
// we run on the GPU...

// particle_cloth.br
// Tests use of structures for input parameters.

#include <stdio.h>
#include <brook/brook.hpp>
#include <timing.hpp>

typedef struct CPUDynamicParticle_t {
	float3 position;
	float3 velocity;
} CPUDynamicParticle;

typedef struct CPUStaticParticle_t {
	bool active;
} CPUStaticParticle;

void cpuStepParticles(
  int count,
  CPUDynamicParticle* dp,
  CPUStaticParticle* sp,
  float deltaT,
  float3* particleForces,
  const float3& gravity )
{
  float3 impulse;
  float halfT = 0.5f*deltaT;
  while(count--)
  {
    if( sp->active )
    {
      impulse.x = deltaT * (particleForces->x + gravity.x);
      impulse.y = deltaT * (particleForces->y + gravity.y);
      impulse.z = deltaT * (particleForces->z + gravity.z);
      dp->position.x += deltaT*dp->velocity.x + halfT*impulse.x;
      dp->position.y += deltaT*dp->velocity.y + halfT*impulse.y;
      dp->position.z += deltaT*dp->velocity.z + halfT*impulse.z;
      dp->velocity.x += impulse.x;
      dp->velocity.y += impulse.y;
      dp->velocity.z += impulse.z;
      particleForces->x = 0;
      particleForces->y = 0;
      particleForces->z = 0;
    }
    dp++;
    sp++;
    particleForces++;
  }
}

void cpuAccumulateWindForces(
  CPUDynamicParticle* p0,
  CPUDynamicParticle* p1,
  CPUDynamicParticle* p2,
  CPUDynamicParticle* begin,
  CPUDynamicParticle* end,
  const float3& windDirection,
  const float3& windVelocity,
  float3* force )
{
  if( p1 < begin || p1 >= end ) return;
  if( p2 < begin || p2 >= end ) return;

  float3 pos0 = p0->position;
  float3 pos1 = p1->position;
  float3 pos2 = p2->position;

  float3 v1;
  float3 v2;

  v1.x = pos1.x - pos0.x;
  v1.y = pos1.y - pos0.y;
  v1.z = pos1.z - pos0.z;

  v2.x = pos2.x - pos0.x;
  v2.y = pos2.y - pos0.y;
  v2.z = pos2.z - pos0.z;

  float3 normalTimesHalfArea;
  normalTimesHalfArea.x = v1.y*v2.z - v1.z*v2.y;
  normalTimesHalfArea.y = v1.z*v2.x - v1.x*v2.z;
  normalTimesHalfArea.z = v1.x*v2.y - v1.y*v2.x;

  float magnitude = normalTimesHalfArea.x*windDirection.x
    + normalTimesHalfArea.y*windDirection.y
    + normalTimesHalfArea.z*windDirection.z;

  force->x += magnitude*windVelocity.x;
  force->y += magnitude*windVelocity.y;
  force->z += magnitude*windVelocity.z;
}

void cpuAccumulateWindForces(
  int countX, int count,
  CPUDynamicParticle* begin,
  const float3& windDirection,
  const float3& windVelocity,
  float3* force )
{
  CPUDynamicParticle* end = begin + count;
  CPUDynamicParticle* p = begin;
  CPUDynamicParticle* left = p - 1;
  CPUDynamicParticle* right = p + 1;
  CPUDynamicParticle* down = p - countX;
  CPUDynamicParticle* up = p + countX;
  for( int y = 0; y < countX; y++ )
  {
    // x = 0
    cpuAccumulateWindForces( p,
      down, right, begin, end,
      windDirection, windVelocity, force );
    cpuAccumulateWindForces( p,
      right, up, begin, end,
      windDirection, windVelocity, force );
    p++;
    left++;
    right++;
    down++;
    up++;
    for( int x = 1; x < countX-1; x++ )
    {
      cpuAccumulateWindForces( p,
        left, down, begin, end,
        windDirection, windVelocity, force );
      cpuAccumulateWindForces( p,
        up, left, begin, end,
        windDirection, windVelocity, force );
      cpuAccumulateWindForces( p,
        down, right, begin, end,
        windDirection, windVelocity, force );
      cpuAccumulateWindForces( p,
        right, up, begin, end,
        windDirection, windVelocity, force );

      p++;
      left++;
      right++;
      down++;
      up++;
    }
    // x = countX-1
    cpuAccumulateWindForces( p,
      left, down, begin, end,
      windDirection, windVelocity, force );
    cpuAccumulateWindForces( p,
      up, left, begin, end,
      windDirection, windVelocity, force );
    p++;
    left++;
    right++;
    down++;
    up++;
  }
}

typedef struct CPUSpringSet_t {
  int offset0;
  int offset1;
  int offsetX0;
  int offsetX1;
  float springConstant;
  float dampingConstant;
  float restLength;
} CPUSpringSet;

void cpuAccumulateSpringForces(
  CPUDynamicParticle* p0,
  CPUDynamicParticle* p1,
  CPUDynamicParticle* begin,
  CPUDynamicParticle* end,
  float3* f0,
  float3* f1,
  const CPUSpringSet& spring )
{
  if( p0 < begin || p0 >= end ) return;
  if( p1 < begin || p1 >= end ) return;

  float3 pos0 = p0->position;
  float3 pos1 = p1->position;
  float3 vel0 = p0->velocity;
  float3 vel1 = p1->velocity;

  float3 relPos;
  float3 relVel;

  relPos.x = pos0.x - pos1.x;
  relPos.y = pos0.y - pos1.y;
  relPos.z = pos0.z - pos1.z;

  relVel.x = vel0.x - vel1.x;
  relVel.y = vel0.y - vel1.y;
  relVel.z = vel0.z - vel1.z;

  float lengthSquared = relPos.x*relPos.x + relPos.y*relPos.y + relPos.z*relPos.z;
  float length = (float) sqrt( (double) lengthSquared );
  float invLength = 1.0f / length;

  float3 springAxis;
  springAxis.x = relPos.x * invLength;
  springAxis.y = relPos.y * invLength;
  springAxis.z = relPos.z * invLength;

  float perturbation = length - spring.restLength;

  float velocityInSpring = springAxis.x*relVel.x + springAxis.y*relVel.y + springAxis.z*relVel.z;

  float strength = spring.springConstant * perturbation
    + spring.dampingConstant * velocityInSpring;

  float3 f;
  f.x = strength * springAxis.x;
  f.y = strength * springAxis.y;
  f.z = strength * springAxis.z;

  f0->x += f.x;
  f0->y += f.y;
  f0->z += f.z;

  f1->x -= f.x;
  f1->y -= f.y;
  f1->z -= f.z;
}

void cpuAccumulateSpringForces(
  int countX, int count,
  CPUDynamicParticle* begin,
  CPUSpringSet springSets[3],
  float3* outForce )
{
  CPUDynamicParticle* end = begin + count;
  CPUDynamicParticle* p = begin;
  float3* f = outForce;

  CPUDynamicParticle* p0[3];
  float3* f0[3];
  CPUDynamicParticle* p1[3];
  float3* f1[3];

  for( int i = 0; i < 3; i++ )
  {
    p0[i] = p + springSets[i].offset0;
    f0[i] = f + springSets[i].offset0;
    p1[i] = p + springSets[i].offset1;
    f1[i] = f + springSets[i].offset1;
  }

  for( int y = 0; y < countX; y++ )
  {
    for( int x = 0; x < countX; x++ )
    {
      for( int i = 0; i < 3; i++ )
      {
        int x0 = x + springSets[i].offsetX0;
        int x1 = x + springSets[i].offsetX1;

        if( x0 < countX )
          cpuAccumulateSpringForces( p, p0[i], begin, end, f, f0[i], springSets[i] );
        if( x1 < countX )
          cpuAccumulateSpringForces( p, p1[i], begin, end, f, f1[i], springSets[i] );

        p0[i]++;
        f0[i]++;
        p1[i]++;
        f1[i]++;
      }
      p++;
      f++;
    }
  }
}

void cpuInitializeParticles(
	int inParticlesPerSide,
	CPUDynamicParticle* outDynamicParticles,
	CPUStaticParticle* outStaticParticles )
{
	int x, y;
	CPUDynamicParticle* dp;
	CPUStaticParticle* sp;
	float scale;

	scale = 1.0f / (float)(inParticlesPerSide-1);

	dp = outDynamicParticles;
	sp = outStaticParticles;
	for( y = 0; y < inParticlesPerSide; y++ )
	{
		for( x = 0; x < inParticlesPerSide; x++ )
		{
			dp->position = float3( scale*(float)x, scale*(float)(-y), 0 );
			dp->velocity = float3( 0, 0, 0 );
			sp->active = true;
			dp++;
			sp++;
		}
	}

	// tweak the corners
	dp = outDynamicParticles + 0;
	sp = outStaticParticles + 0;
	dp->position = float3( 0, 0, 0.1f );
	sp->active = false;
	dp = outDynamicParticles + (inParticlesPerSide-1);
	sp = outStaticParticles + (inParticlesPerSide-1);
	dp->position = float3( 1, 0, -0.1f );
	sp->active = false;
}

void cpuIterate( int inSize, int inIterations, int& outMicroseconds )
{
  int particleCount = inSize*inSize;
  
  float3 gravity;
  float3 windDirection;
  float3 windVelocity;
  
  int i;
//  int j;
  CPUDynamicParticle* dynamicParticles;
  CPUStaticParticle* staticParticles;
  float3* forces;
//  FILE* dump;

  dynamicParticles = (CPUDynamicParticle*)malloc( particleCount*sizeof(CPUDynamicParticle) );
  staticParticles = (CPUStaticParticle*)malloc( particleCount*sizeof(CPUStaticParticle) );
  forces = (float3*)malloc( particleCount*sizeof(float3) );

  CPUSpringSet springSets[3];

  // direct
  springSets[0].offset0 = 1;
  springSets[0].offsetX0 = 1;
  springSets[0].offset1 = inSize;
  springSets[0].offsetX1 = 0;
//  springSets[0].springConstant = -0.5f * (float)(particleCount);
//  springSets[0].dampingConstant = -0.03f * (float)(particleCount);
  springSets[0].springConstant = 0;
  springSets[0].dampingConstant = 0;
  springSets[0].restLength = 1.0f / (float)(inSize-1);

  // diagonal
  springSets[1].offset0 = 1 - inSize;
  springSets[1].offsetX0 = 1;
  springSets[1].offset1 = 1 + inSize;
  springSets[1].offsetX1 = 1;
//  springSets[1].springConstant = springSets[0].springConstant;
//  springSets[1].dampingConstant = springSets[0].dampingConstant;
  springSets[1].springConstant = 0;
  springSets[1].dampingConstant = 0;
  springSets[1].restLength = springSets[0].restLength * (float) sqrt(2.0);

  // bend
  springSets[2].offset0 = 2;
  springSets[2].offsetX0 = 2;
  springSets[2].offset1 = 2*inSize;
  springSets[2].offsetX1 = 0;
//  springSets[2].springConstant = -0.1f * (float)(particleCount);
//  springSets[2].dampingConstant = -0.07f * (float)(particleCount);
  springSets[2].springConstant = 0;
  springSets[2].dampingConstant = 0;
  springSets[2].restLength = springSets[0].restLength * 2.0f;

//  gravity = float3( 0, -0.1f, 0 );
//  windDirection = float3( 0, 0, -1.0f );
//  windVelocity = float3( 0, 0, -1.0f );
    gravity = float3( 0, 0, 0 );
    windDirection = float3( 0, 0, -1.0f );
    windVelocity = float3( 0, 0, 0 );

  // initialize
  cpuInitializeParticles( inSize, dynamicParticles, staticParticles );
  for( i = 0; i < particleCount; i++ )
    forces[i] = float3(0,0,0);

//  dump = fopen("./dump.dump","wb");
  
//  fwrite( &inSize, sizeof(inSize), 1, dump );

  int64 startTime = GetTime();

  for( i = 0; i < inIterations; i++ )
  {
/*	  if( i % 100 == 0 )
	  {
      printf("positions:\n");
		  for( j = 0; j < particleCount; j++ )
		  {
			  p = dynamicParticles[j].position;
        printf( "{%5.3f %5.3f}\n", p.x, p.y );
//			  fwrite( &p, sizeof(p), 1, dump );
		  }
	  }
*/

    cpuAccumulateSpringForces(
      inSize, particleCount,
      dynamicParticles, springSets,
      forces );

    cpuAccumulateWindForces(
      inSize, particleCount,
      dynamicParticles,
      windDirection, windVelocity,
      forces );

    cpuStepParticles(
      particleCount,
      dynamicParticles, staticParticles,
      0.001f, forces, gravity );
  }

  int64 endTime = GetTime();

  outMicroseconds = (int)(endTime - startTime);
  
//  fclose( dump );
}
