#ifndef BROOK_HPP
#define BROOK_HPP

#include <stdlib.h>
#if defined(__GNUC__) &&  __GNUC__==2 && __GNUC_MINOR__<=97
#define __SGI_STL_INTERNAL_RELOPS
// some predefined operator< bugs in stl_relops.h (included by vector) -- they should be inside a namespace, but namespaces don't work well in gcc 2.95.
#endif
#include <vector>
#include "brt.hpp"
#include "brtvector.hpp"
#include "brtintrinsic.hpp"
#include "brtarray.hpp"
#include "brtscatter.hpp"
#include "brtgather.hpp"

template <class T> inline __BRTStreamType __BRTReductionType(const T*e) {
   const float * f = e;
   //error case! will complain about casting an T to a float.
   return __BRTFLOAT;
}
template <> inline __BRTStreamType __BRTReductionType(const __BRTStream*e) {
   return __BRTSTREAM;
}
template <> inline __BRTStreamType __BRTReductionType(const float4*e) {
   return __BRTFLOAT4;
}
template <> inline __BRTStreamType __BRTReductionType(const float3*e) {
   return __BRTFLOAT3;
}
template <> inline __BRTStreamType __BRTReductionType(const float2*e) {
   return __BRTFLOAT2;
}
template <> inline __BRTStreamType __BRTReductionType(const float*e) {
   return __BRTFLOAT;
}
inline unsigned int getIndexOf(unsigned int i, 
                               const unsigned int *extent,
                               unsigned int dim,
                               const unsigned int *refextent) {
   unsigned int ret = (i%refextent[dim-1])*extent[dim-1]/refextent[dim-1];
   unsigned int accum=extent[dim-1];
   unsigned int refaccum=refextent[dim-1];
   for (int d=dim-2;d>=0;--d) {
      ret+= (((i/refaccum)%refextent[d])*extent[d]/refextent[d])*accum;
      refaccum*=refextent[d];
      accum*=extent[d];
   }
   return ret;
}

inline unsigned int getIndexOf(unsigned int i, 
                               const unsigned int *mapbegin,
                               const unsigned int *mapextent,
                               const unsigned int *extent,
                               unsigned int dim,
                               const unsigned int *refextent) {
   unsigned int ret = mapbegin[dim-1];//+(i%mapextent[dim-1]);
   // you know the above is
   unsigned int accum=refextent[dim-1];
   unsigned int refaccum=mapextent[dim-1];
   for (int d=dim-2;d>=0;--d) {
      ret+= ((i/refaccum)%mapextent[d]+mapbegin[d])*accum;
      refaccum*=mapextent[d];
      accum*=refextent[d];
   }
   return getIndexOf(ret,extent,dim,refextent);
}

#endif


