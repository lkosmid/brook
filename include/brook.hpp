#ifndef BROOK_HPP
#define BROOK_HPP

#include <stdlib.h>
#include <vector>
#include "brt.hpp"
#include "brtvector.hpp"
#include "brtarray.hpp"
#include "brtscatter.hpp"

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
   unsigned int ret=mapbegin[0]/*+i%mapextent[dim-1]*/;
   //commented out region above is zero whenever this function is invoked   
   for (unsigned int j=1;j<dim;++j) {
      ret*=refextent[j];
      unsigned ij = i;
      for (unsigned int k=dim-1;k>j;--k) {
         //the above may be unsigned as j>=1
         ij/=mapextent[k];
      }
      ret+=mapbegin[j]+ij%refextent[j];
   }
   return getIndexOf(ret,extent,dim,refextent);
}

#endif


