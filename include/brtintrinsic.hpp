#include <math.h>
#ifdef _WIN32
#include <float.h>
#endif
#ifndef BRTINTRINSIC_HPP
#define BRTINTRINSIC_HPP

#define UNINTRINSICMEMBER(FUNC,CALLFUNC,RET) \
inline RET FUNC (const __BrtFloat4 &f) { \
  return f.CALLFUNC(); \
} \
inline RET FUNC (const __BrtFloat3 &f) { \
  return f.CALLFUNC(); \
} \
inline RET FUNC (const __BrtFloat2 &f) { \
  return f.CALLFUNC(); \
} \
inline RET FUNC (const __BrtFloat1 &f) { \
  return f.CALLFUNC(); \
}


#define UNINTRINSIC(FUNC,CALLFUNC) \
inline __BrtFloat4 FUNC (const __BrtFloat4 &f) { \
  return __BrtFloat4 (CALLFUNC (f.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1)), \
                      CALLFUNC (f.unsafeGetAt(2)), \
                      CALLFUNC (f.unsafeGetAt(3))); \
} \
inline __BrtFloat3 FUNC (const __BrtFloat3 &f) { \
  return __BrtFloat3 (CALLFUNC (f.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1)), \
                      CALLFUNC (f.unsafeGetAt(2))); \
} \
inline __BrtFloat2 FUNC (const __BrtFloat2 &f) { \
  return __BrtFloat2 (CALLFUNC (f.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1))); \
} \
inline __BrtFloat1 FUNC (const __BrtFloat1 &f) { \
  return __BrtFloat1 (CALLFUNC (f.unsafeGetAt(0))); \
}


#define UNINTRINSICINOUT(FUNC,CALLFUNC) \
inline __BrtFloat4 FUNC (const __BrtFloat4 &f, __BrtFloat4 &out) { \
  return __BrtFloat4 (CALLFUNC (f.unsafeGetAt(0),out.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1),out.unsafeGetAt(1)), \
                      CALLFUNC (f.unsafeGetAt(2),out.unsafeGetAt(2)), \
                      CALLFUNC (f.unsafeGetAt(3),out.unsafeGetAt(3))); \
} \
inline __BrtFloat3 FUNC (const __BrtFloat3 &f, __BrtFloat3 &out) { \
  return __BrtFloat3 (CALLFUNC (f.unsafeGetAt(0),out.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1),out.unsafeGetAt(1)), \
                      CALLFUNC (f.unsafeGetAt(2),out.unsafeGetAt(2))); \
} \
inline __BrtFloat2 FUNC (const __BrtFloat2 &f, __BrtFloat2 &out) { \
  return __BrtFloat2 (CALLFUNC (f.unsafeGetAt(0),out.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1),out.unsafeGetAt(1))); \
} \
inline __BrtFloat1 FUNC (const __BrtFloat1 &f, __BrtFloat1 & out) { \
  return __BrtFloat1 (CALLFUNC (f.unsafeGetAt(0),out.unsafeGetAt(0))); \
}

#if defined (_MSC_VER) && (_MSC_VER <= 1200)
#define TEMPL_TYPESIZE sizeof(BRT_TYPE)/sizeof(BRT_TYPE::TYPE)
#else
#define TEMPL_TYPESIZE BRT_TYPE::size
#endif

#define BININTRINSIC(FUNC,CALLFUNC) \
template <class BRT_TYPE> vec<GCCTYPENAME LCM<GCCTYPENAME BRT_TYPE::TYPE,float>::type, \
       LUB<TEMPL_TYPESIZE,4>::size> FUNC (const __BrtFloat4 &f, const BRT_TYPE &g) { \
  return f.CALLFUNC(g); \
} \
template <class BRT_TYPE> vec<GCCTYPENAME LCM<GCCTYPENAME BRT_TYPE::TYPE,float>::type, \
       LUB<TEMPL_TYPESIZE,3>::size> FUNC (const __BrtFloat3 &f, const BRT_TYPE &g) { \
  return f.CALLFUNC(g); \
} \
template <class BRT_TYPE> vec<GCCTYPENAME LCM<GCCTYPENAME BRT_TYPE::TYPE,float>::type, \
       LUB<TEMPL_TYPESIZE,2>::size> FUNC (const __BrtFloat2 &f, const BRT_TYPE &g) { \
  return f.CALLFUNC(g); \
} \
template <class BRT_TYPE> vec<GCCTYPENAME LCM<GCCTYPENAME BRT_TYPE::TYPE,float>::type, \
       LUB<TEMPL_TYPESIZE,1>::size> FUNC (const __BrtFloat1 &f, const BRT_TYPE &g) { \
  return f.CALLFUNC(g); \
}

#define TRIINTRINSIC(FUNC,CALLFUNC) \
inline __BrtFloat4 FUNC (const __BrtFloat4 &f, \
             const __BrtFloat4 &g, \
             const __BrtFloat4 &h) { \
  return __BrtFloat4 (CALLFUNC (f.unsafeGetAt(0), \
                                       g.unsafeGetAt(0), \
                                       h.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1), \
                                       g.unsafeGetAt(1), \
                                       h.unsafeGetAt(1)), \
                      CALLFUNC (f.unsafeGetAt(2), \
                                       g.unsafeGetAt(2), \
                                       h.unsafeGetAt(2)), \
                      CALLFUNC (f.unsafeGetAt(3), \
                                       g.unsafeGetAt(3), \
                                       h.unsafeGetAt(3))); \
} \
inline __BrtFloat3 FUNC (const __BrtFloat3 &f, \
                         const __BrtFloat3 &g, \
                         const __BrtFloat3 &h) { \
  return __BrtFloat3 (CALLFUNC (f.unsafeGetAt(0), \
                                       g.unsafeGetAt(0), \
                                       h.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1), \
                                       g.unsafeGetAt(1), \
                                       h.unsafeGetAt(1)), \
                      CALLFUNC (f.unsafeGetAt(2), \
                                       g.unsafeGetAt(2), \
                                       h.unsafeGetAt(2))); \
} \
inline __BrtFloat2 FUNC (const __BrtFloat2 &f, \
                         const __BrtFloat2 &g, \
                         const __BrtFloat2 &h) { \
  return __BrtFloat2 (CALLFUNC (f.unsafeGetAt(0), \
                                       g.unsafeGetAt(0), \
                                       h.unsafeGetAt(0)), \
                      CALLFUNC (f.unsafeGetAt(1), \
                                       g.unsafeGetAt(1), \
                                       h.unsafeGetAt(1))); \
} \
inline __BrtFloat1 FUNC (const __BrtFloat1 &f, \
                         const __BrtFloat1 &g, \
                         const __BrtFloat1 &h) { \
  return __BrtFloat1 (CALLFUNC (f.unsafeGetAt(0), \
                                       g.unsafeGetAt(0), \
                                       h.unsafeGetAt(0))); \
}


inline float degrees_float (float x) {
   return x*180.0f/3.1415926536f;
}
inline float radians_float (float x) {
   return x*3.1415926536f/180.0f;
}
inline float saturate_float (float x) {
   return x>1.0f?1.0f:x<0.0f?0.0f:x;
}
inline float clamp_float(float x, float l, float u) {
   return x>u?u:x<l?l:x;
}
inline float sign_float (float x) {
   return x==0.0f?0.0f:x<0.0f?-1.0f:1.0f;
}
inline float exp2_float (float x) {
   return (float)pow(2.0f,x);
}
static const float _const_log2 = log(2.0f);
inline float log2_float (float x) {
   return (float)log (x)/_const_log2;
}
inline float round_float (float x) {
   float f = x-floor(x);
   float g = ceil(x)-x;
   return f==g?(x<0.0f?floor(x):ceil(x)):f<g?floor(x):ceil(x);
}
inline float lerp_float (float a, float b, float s) {
   return a + s*(b - a);
}
inline float rsqrt_float (float x) {
#ifdef __APPLE__
	return 1.0f/sqrt(x);
#else
   return 1.0f/sqrtf(x);
#endif
}
inline float frac_float (float x) {
   float y = x-floor(x);
   return x<0.0f?1.0f-y:y;
}
inline float frc_float (float x) {
   return frac_float(x);
}
inline float frexp_float (float x, float & oout) {
   int exp;
   x = frexp(x,&exp);
   oout=(float)exp;
   return x;
}
inline float modf_float (float x, float & oout) {
   double exp;
   x = (float)modf(x,&exp);
   oout=(float)exp;
   return x;
}
inline float finite_float (float x) {
#ifdef _WIN32
   return _finite(x);
#else
#ifdef __APPLE__
   return __isfinitef(x);
#else
   return finite(x);
#endif
#endif
}
inline float isnan_float (float x) {
#ifdef _WIN32
   return _isnan(x);
#else
#ifdef __APPLE__
   return __isnanf(x);
#else
   return isnan(x);
#endif
#endif
}
inline float isinf_float (float x) {
#ifdef _WIN32
   return (!finite_float(x))&&(!isnan_float(x));
#else
#ifdef __APPLE__
   return __isinff(x);
#else
   return isinf(x);
#endif
#endif
}
  BININTRINSIC(atan2,atan2)
  BININTRINSIC(fmod,fmodf)
  BININTRINSIC(pow,pow)
  UNINTRINSICMEMBER(any,any,__BrtFloat1);
  UNINTRINSICMEMBER(all,all,__BrtFloat1);
  UNINTRINSICMEMBER(length,length,__BrtFloat1);
  UNINTRINSICMEMBER(len,length,__BrtFloat1);
  UNINTRINSICINOUT(frexp,frexp_float);
  UNINTRINSIC(degrees,degrees_float)
  UNINTRINSIC(radians,radians_float)
  UNINTRINSIC(saturate,saturate_float)
  UNINTRINSIC(abs,fabsf)
  TRIINTRINSIC(clamp,clamp_float)
  UNINTRINSIC(isfinite,finite_float)
  UNINTRINSIC(isnan,isnan_float)
  BININTRINSIC(max,max_float)
  BININTRINSIC(min,min_float)
  UNINTRINSIC(sign,sign_float)
  UNINTRINSIC(acos,acos)
  UNINTRINSIC(asin,asin)
  UNINTRINSIC(atan,atan)
  UNINTRINSIC(ceil,ceil)
  UNINTRINSIC(cos,cos)
  UNINTRINSIC(cosh,cosh)
  UNINTRINSIC(exp,exp)
  UNINTRINSIC(exp2,exp2_float)
  UNINTRINSIC(floor,floor)
  UNINTRINSIC(frac,frac_float)
  UNINTRINSIC(frc,frc_float)
  UNINTRINSIC(isinf,isinf_float)
  BININTRINSIC(ldexp,ldexp_float)
  BININTRINSIC(dot,dot)
  BININTRINSIC(distance,distance)
  UNINTRINSIC(log,log)
  UNINTRINSIC(log2,log2_float)
  UNINTRINSIC(log10,log10)
  UNINTRINSICINOUT(modf,modf_float)
  UNINTRINSIC(round,round_float)
  UNINTRINSIC(rsqrt,rsqrt_float)
  UNINTRINSIC(sin,sin)  
  //UNINTRINSIC(sincos)
  UNINTRINSIC(sinh,sinh)
  UNINTRINSIC(sqrt,sqrt)
  BININTRINSIC(step,step)
  UNINTRINSIC(tan,tan)
  UNINTRINSIC(tanh,tanh)
     //  TRIINTRINSIC(smoothstep)
  TRIINTRINSIC(lerp,lerp_float)

#undef UNINTRINSIC
#undef BININTRINSIC
#undef TRIINTRINSIC
inline __BrtFloat3 cross (const __BrtFloat3 &u, const __BrtFloat3 v) {
     return __BrtFloat3( u.unsafeGetAt(1)*v.unsafeGetAt(2)
                         -u.unsafeGetAt(2)*v.unsafeGetAt(1),
                         u.unsafeGetAt(2)*v.unsafeGetAt(0)
                         -u.unsafeGetAt(0)*v.unsafeGetAt(2),
                         
                         u.unsafeGetAt(0)*v.unsafeGetAt(1)
                         -u.unsafeGetAt(1)*v.unsafeGetAt(0));
}

#endif
