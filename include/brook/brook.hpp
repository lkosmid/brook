#ifndef BROOK_HPP
#define BROOK_HPP

#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#if defined(__GNUC__) &&  __GNUC__==2 && __GNUC_MINOR__<=97
#define __SGI_STL_INTERNAL_RELOPS
// some predefined operator< bugs in stl_relops.h (included by vector) -- they should be inside a namespace, but namespaces don't work well in gcc 2.95.
#endif
#include <vector>
#include <brook/brt.hpp>
#include <brook/brtvector.hpp>
#include <brook/brtintrinsic.hpp>
#include <brook/brtarray.hpp>
#include <brook/kerneldesc.hpp>

#if defined(__GNUC__) &&  __GNUC__==4 && __GNUC_MINOR__==8 && __GNUC_PATCHLEVEL__==4
// gcc 4.8.4 ignores -lpthread if not used, resulting in binaries
// failing at runtime loading due to the absence of pthread linking information
#include <pthread.h>
static void* __dummythread(void*) { return NULL; } 
void static __forcePThreadLink() { pthread_t t1; pthread_create(&t1, NULL, &__dummythread, NULL); }
#endif

//  We are going to skip these for the simpler cpu 
//  versions
//#include "brtscatter.hpp"
//#include "brtgather.hpp"

template <class T> inline ::brook::StreamType __BRTReductionType(const T*e) {
   const float * f = e;
   //error case! will complain about casting an T to a float.
   return ::brook::__BRTFLOAT;
}
template <> inline ::brook::StreamType __BRTReductionType(const ::brook::stream *e) {
   return ::brook::__BRTSTREAM;
}
template <> inline ::brook::StreamType __BRTReductionType(const float4*e) {
   return ::brook::__BRTFLOAT4;
}
template <> inline ::brook::StreamType __BRTReductionType(const float3*e) {
   return ::brook::__BRTFLOAT3;
}
template <> inline ::brook::StreamType __BRTReductionType(const float2*e) {
   return ::brook::__BRTFLOAT2;
}
template <> inline ::brook::StreamType __BRTReductionType(const float*e) {
   return ::brook::__BRTFLOAT;
}

#if defined(BROOK_ENABLE_ADDRESS_TRANSLATION)
namespace {
  class AtStartup {
    static BRTTLS AtStartup atStartup;
    AtStartup() {
      ::brook::Runtime* runtime = ::brook::createRuntime( true );
      if (0)
         runtime=0;
    }
  };
  AtStartup AtStartup::atStartup;
}
#endif


#endif


