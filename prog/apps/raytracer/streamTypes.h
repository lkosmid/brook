/* *************************************************************************
 * Copyright (C) 2004 Jeremy Sugerman
 * All Rights Reserved
 * *************************************************************************/

/*
 * streamTypes.h --
 *
 *      The structs / typedefs for kernel stream arguments.
 *
 *      Note: These can't just be dumped in a .brh file because they need to
 *      be included in both .br and C / C++ files.  That means they can't be
 *      in the same file as kernel declarations since those have to expand
 *      differently in each place.
 */

#ifndef __STREAMTYPES_H__
#define __STREAMTYPES_H__


typedef struct ray_t {
  float3 o;
  float3 d;
  float tmax;
} Ray;

typedef struct raystate_t {
  // an active ray is in either the traversal, intersection, or
  // shading states. so first 3 components are either (1,0,0) (0,1,0)
  // or (0,0,1).  If in isect state, the w component stores index of
  // next triangle to test for intersection
  float4 state; //x=trav, y=isect, z=shade, w=trinum
} RayState;

typedef struct triangle_t {
  float3 v0;
  float3 v1;
  float3 v2;
} Triangle;

typedef struct shadinginfo_t {
  float3 n0;
  float3 n1;
  float3 n2;
  float3 c0;
  float3 c1;
  float3 c2;
} ShadingInfo;

typedef struct hit_t {
  // data is packed as ( ray t, uu, vv, triangle id )
  // Note the third bary coord is not stored, compute
  // it from the first two
  float4 data;
} Hit;

typedef struct pixel_t {
  float4 data;  //rgba
} Pixel;

typedef struct gridtrilistoffset_t {
  float listoffset;  //actually ints
} GridTrilistOffset;

typedef struct gridtrilist_t {
  float trinum;  //actually ints
} GridTrilist;

typedef struct traversaldatadyn_t {
  float3 tMax;     // t when exiting current voxel
  float3 voxno;    //actually ints (current voxel for grid traversal)
  float3 voxtouse; //ints   (current voxel when intersecting tris)
} TraversalDataDyn;

typedef struct traversaldatastatic_t {
  float3 tDelta; // change in t from voxel to voxel
  float3 step;   //actually -1 or 1 only, change in voxel index
  float3 outno;  //actually ints, index of voxel when ray leaves grid
} TraversalDataStatic;
#endif
