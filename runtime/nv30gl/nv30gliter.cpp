#include <stdio.h>

#include "nv30gl.hpp"

using namespace brook;

NV30GLIter::NV30GLIter (NV30GLRunTime *rt,
                        __BRTStreamType type, int dims, 
                        int extents[], float ranges[])
  : Iter(type), runtime(rt) { 
  
  int i;

  // Initialize width and height
  height  = 1;
  switch (dims) {
  case 2:
     width = extents[1];
     height = extents[0];
     break;
  case 1:
     width = extents[0];
     break;
  default:
     fprintf (stderr, 
              "NV30GL Backend only supports 1D and 2D iters");
     exit(1);
  }
  
  // Initialize ncomp
  ncomp = type;
  if (ncomp < 1 || ncomp > 4) {
     fprintf (stderr, "NV30GL: Unsupported iter type created\n");
     exit(1);
  }

  if (dims > 1 && (dims != 2 || ncomp != 2)) {
     fprintf (stderr, "NV30GL: Only 1D or 2D float2 iter streams supported\n");
     exit(1);
  }
  
  // Initialize the cacheptr
  cacheptr = NULL;
  
  // Initialize extents
  for (i=0; i<dims; i++)
    this->extents[i] = extents[i];
  this->dims = dims;

  // Initialize ranges
  min.x = 0.0f; min.y = 0.0f; min.z = 0.0f; min.w = 1.0f;
  max.x = 0.0f; max.y = 0.0f; max.z = 0.0f; max.w = 1.0f;

  switch (ncomp) {
  case 4:
     min.w = ranges[3];
     max.w = ranges[7];
  case 3:
     min.z = ranges[2];
     max.z = ranges[ncomp+2];
  case 2:
     min.y = ranges[1];
     max.y = ranges[ncomp+1];
  case 1:
     min.x = ranges[0]; 
     max.x = ranges[ncomp];
     break;
  default:
     assert(0);
  }
}

void * 
NV30GLIter::getData (unsigned int flags) {
   assert (! (flags & StreamInterface::WRITE));  

   if (cacheptr == NULL) {
      unsigned int i,j;
      float *p;
      cacheptr = (float *) malloc (sizeof(float)*ncomp*width*height);

      if (dims == 1) {
         float4 incr;
         incr.x = (max.x - min.x) / width;
         incr.y = (max.y - min.y) / width;
         incr.z = (max.z - min.z) / width;
         incr.w = (max.w - min.w) / width;
         p = cacheptr;
         for (i=0; i<width; i++) {
            float4 v;
            v.x  = min.x + incr.x*i;
            v.y  = min.y + incr.y*i;
            v.z  = min.z + incr.w*i;
            v.w  = min.w + incr.z*i;
            switch (ncomp) {
            case 4:
               p[3] = v.w;
            case 3:
               p[2] = v.z;
            case 2:
               p[1] = v.y;
            case 1:
               p[0] = v.x;
               break;
            default:
               assert(0);
            }
            p += ncomp;
         }
      } else {
         assert (dims == 2);
         float2 incr;
         incr.x = (max.x - min.x) / width;
         incr.y = (max.y - min.y) / height;
         p = cacheptr;
         for (i=0; i<height; i++) 
            for (j=0; j<width; j++) {
               float2 v;
               v.x = min.x + incr.x*j;
               v.y = min.y + incr.y*i;
               *p++ = v.x;
               *p++ = v.y;
            }
      }
   }

   return (void *) cacheptr;
}

void 
NV30GLIter::releaseData (unsigned int flags) {
   // Do nothing
}


NV30GLIter::~NV30GLIter () {
  if (cacheptr)
     free (cacheptr);
}
