/* *************************************************************************
 * Copyright (C) 2004 Jeremy Sugerman
 * All Rights Reserved
 * *************************************************************************/

/*
 * main.h --
 *
 *      Core structs used in the ray tracer
 */

#ifndef __MAIN_H__
#define __MAIN_H__


typedef struct Camera {
   float3 u, v, w;
   float tx, ty;
} Camera;


typedef struct Scene {
   char *fileName;

   float3 lookFrom, lookAt;
   float3 up;
   float fov;

   float3 pointLight;
} Scene;


/*
 * Normalize --
 *
 *      Simple routine to normalize a float3 (presumed to be a vector)
 *
 * Returns:
 *      A vector of unit length parallel to the given one.
 */

static inline float3
Normalize(const float3& vec) {
   float x, y, z, l;

   l = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
   x = vec.x / l;
   y = vec.y / l;
   z = vec.z / l;
   return float3(x, y, z);
}


/*
 * Cross --
 *
 *      Simple routine to return the cross product of two float3s (presumed
 *      to be vectors).
 *
 * Returns:
 *      A vector orthogonal to both given vectors.
 */

static inline float3
Cross(const float3& v1, const float3& v2)
{
   return float3(v1.y * v2.z - v1.z * v2.y,
                 v1.z * v2.x - v1.x * v2.z,
                 v1.z * v2.y - v1.y * v2.x);
}
#endif
