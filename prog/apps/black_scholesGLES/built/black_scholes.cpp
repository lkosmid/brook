
////////////////////////////////////////////
// Generated by BRCC v0.1
// BRCC Compiled on: Mar 19 2017 23:54:49
////////////////////////////////////////////

#include <brook/brook.hpp>
#include <math.h>

#include "common.h"

#include "Timer.h"

#include <stdio.h>

#include <assert.h>

static int  retval = 0;
#define S_LOWER_LIMIT 10.0f

#define S_UPPER_LIMIT 100.0f

#define K_LOWER_LIMIT 10.0f

#define K_UPPER_LIMIT 100.0f

#define T_LOWER_LIMIT 1.0f

#define T_UPPER_LIMIT 10.0f

#define r_LOWER_LIMIT 0.01f

#define r_UPPER_LIMIT 0.05f

#define sigma_LOWER_LIMIT 0.01f

#define sigma_UPPER_LIMIT 0.10f

static const char *__PHI_ps20= NULL;
static const char *__PHI_ps2b= NULL;
static const char *__PHI_ps2a= NULL;
static const char *__PHI_ps30= NULL;
static const char *__PHI_ctm= NULL;
static const char *__PHI_fp30= NULL;
static const char *__PHI_fp40= NULL;
static const char *__PHI_arb= NULL;
static const char *__PHI_glsl= NULL;

static __BrtFloat2  __PHI_cpu_inner(const __BrtFloat2  &X)
{
  __BrtFloat2  y;
  __BrtFloat2  absX;
  __BrtFloat2  t;
  __BrtFloat2  result;
  const __BrtFloat1  c1 = __BrtFloat1(0.319382f);
  const __BrtFloat1  c2 = -__BrtFloat1(0.356564f);
  const __BrtFloat1  c3 = __BrtFloat1(1.781478f);
  const __BrtFloat1  c4 = -__BrtFloat1(1.821256f);
  const __BrtFloat1  c5 = __BrtFloat1(1.330274f);
  const __BrtFloat1  zero = __BrtFloat1(0.000000f);
  const __BrtFloat1  one = __BrtFloat1(1.000000f);
  const __BrtFloat1  two = __BrtFloat1(2.000000f);
  const __BrtFloat1  temp4 = __BrtFloat1(0.231642f);
  const __BrtFloat1  one_by_sqrt_2_pi = __BrtFloat1(0.398942f);

  absX = __abs_cpu_inner(X);
  t = one / (one + temp4 * absX);
  y = one - one_by_sqrt_2_pi * __exp_cpu_inner(-X * X / two) * t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * c5))));
  result.mask1((X.swizzle1(maskX) < zero).questioncolon(one - y.swizzle1(maskX),y.swizzle1(maskX)),maskX);
  result.mask1((X.swizzle1(maskY) < zero).questioncolon(one - y.swizzle1(maskY),y.swizzle1(maskY)),maskY);
  return result;
}


static const char *__black_scholes_call_ps20= NULL;
static const char *__black_scholes_call_ps2b= NULL;
static const char *__black_scholes_call_ps2a= NULL;
static const char *__black_scholes_call_ps30= NULL;
static const char *__black_scholes_call_ctm= NULL;
static const char *__black_scholes_call_fp30= NULL;
static const char *__black_scholes_call_fp40= NULL;
static const char *__black_scholes_call_arb= NULL;
static const char *__black_scholes_call_glsl= NULL;

namespace {
	using namespace ::brook::desc;
	static const gpu_kernel_desc __black_scholes_call_gles_desc = gpu_kernel_desc()
		.technique( gpu_technique_desc()
			.pass( gpu_pass_desc(
				"// glesf output by Cg compiler\n"
				"// cgc version 3.1.0013, build date Apr 18 2012\n"
				"// command line args: -quiet -DCGC=1 -profile glesf -DGL_ES\n"
				"// source file: /tmp/brookHF95fE\n"
				"//vendor NVIDIA Corporation\n"
				"//version 3.1.0.13\n"
				"//profile glesf\n"
				"//program main\n"
				"//semantic main._tex_S : TEXUNIT0\n"
				"//semantic main._tex_sigma : TEXUNIT1\n"
				"//semantic main._tex_K : TEXUNIT2\n"
				"//semantic main._tex_T : TEXUNIT3\n"
				"//semantic main._tex_R : TEXUNIT4\n"
				"//semantic main.__workspace : C0\n"
				"//var sampler2D _tex_S : TEXUNIT0 : _tex_S1 0 : 0 : 1\n"
				"//var sampler2D _tex_sigma : TEXUNIT1 : _tex_sigma1 1 : 2 : 1\n"
				"//var sampler2D _tex_K : TEXUNIT2 : _tex_K1 2 : 4 : 1\n"
				"//var sampler2D _tex_T : TEXUNIT3 : _tex_T1 3 : 6 : 1\n"
				"//var sampler2D _tex_R : TEXUNIT4 : _tex_R1 4 : 8 : 1\n"
				"//var float2 _tex_S_pos : $vin.TEXCOORD0 : TEXCOORD0 : 1 : 1\n"
				"//var float2 _tex_sigma_pos : $vin.TEXCOORD1 : TEXCOORD1 : 3 : 1\n"
				"//var float2 _tex_K_pos : $vin.TEXCOORD2 : TEXCOORD2 : 5 : 1\n"
				"//var float2 _tex_T_pos : $vin.TEXCOORD3 : TEXCOORD3 : 7 : 1\n"
				"//var float2 _tex_R_pos : $vin.TEXCOORD4 : TEXCOORD4 : 9 : 1\n"
				"//var float __output_0 : $vout.COLOR0 : COL : 10 : 1\n"
				"\n"
				"precision highp float;\n"
				"\n"
				"struct double_struct {\n"
				"    vec2 _x;\n"
				"};\n"
				"\n"
				"struct double2_struct {\n"
				"    vec4 _xy;\n"
				"};\n"
				"\n"
				"float _TMP2;\n"
				"vec4 _TMP0;\n"
				"uniform sampler2D _tex_S1;\n"
				"uniform sampler2D _tex_sigma1;\n"
				"uniform sampler2D _tex_K1;\n"
				"uniform sampler2D _tex_T1;\n"
				"uniform sampler2D _tex_R1;\n"
				"vec3 _temp30038;\n"
				"vec3 _temp40038;\n"
				"vec3 _temp50038;\n"
				"vec3 _temp60038;\n"
				"vec2 _d0038;\n"
				"float _K_exp_minus_r_T0038;\n"
				"vec2 _temp0038;\n"
				"vec2 _temp20038;\n"
				"float _call0038;\n"
				"vec3 _TMP47;\n"
				"vec2 _t0052;\n"
				"vec2 _y0052;\n"
				"vec2 _result0052;\n"
				"vec2 _x0056;\n"
				"varying vec4 TEX0;\n"
				"varying vec4 TEX1;\n"
				"varying vec4 TEX2;\n"
				"varying vec4 TEX3;\n"
				"varying vec4 TEX4;\n"
				"\n"
				" // main procedure, the original name was main\n"
				"void main()\n"
				"{\n"
				"\n"
				"    float _S1;\n"
				"    float _sigma1;\n"
				"    float _K1;\n"
				"    float _T1;\n"
				"\n"
				"    reconstruct_float(_TMP0.x, _tex_S1, TEX0.xy);\n"
				"    _S1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_sigma1, TEX1.xy);\n"
				"    _sigma1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_K1, TEX2.xy);\n"
				"    _K1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_T1, TEX3.xy);\n"
				"    _T1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_R1, TEX4.xy);\n"
				"    _temp40038.x = sqrt(_T1);\n"
				"    _temp30038 = vec3(_sigma1, _sigma1, -_TMP0.x);\n"
				"    _temp40038.yz = vec2(_sigma1, _T1);\n"
				"    _temp50038 = _temp30038*_temp40038;\n"
				"    _temp40038.xy = vec2(_K1, 2.0);\n"
				"    _temp30038.xy = vec2(_S1, _temp50038.y);\n"
				"    _temp60038.xy = _temp30038.xy/_temp40038.xy;\n"
				"    _TMP2 = log(_temp60038.x);\n"
				"    _d0038.x = (_TMP2 + (_TMP0.x + _temp60038.y)*_T1)/_temp50038.x;\n"
				"    _d0038.y = _d0038.x - _temp50038.x;\n"
				"    _t0052 = 1.0/(1.0 + 2.31641993E-01*abs(_d0038));\n"
				"    _x0056 = (-_d0038*_d0038)/2.0;\n"
				"    _temp50038.xy = _x0056;\n"
				"    _TMP47 = exp(_temp50038.xyz);\n"
				"    _K_exp_minus_r_T0038 = _K1*_TMP47.z;\n"
				"    _temp0038 = vec2(_S1, _K_exp_minus_r_T0038);\n"
				"    _y0052 = 1.0 - (3.98941994E-01*_TMP47.xy)*_t0052*(3.19382012E-01 + _t0052*(-3.56563985E-01 + _t0052*(1.78147805 + _t0052*(-1.82125604 + _t0052*1.33027399))));\n"
				"    _result0052=abs(step(_d0038, vec2(0.0))-_y0052);\n"
				"    _temp20038 = _temp0038*_result0052;\n"
				"    _call0038 = _temp20038.x - _temp20038.y;\n"
				"    encode_output_float( _call0038);\n"
				"} //  \n"
				"//!!BRCC\n"
				"//narg:6\n"
				"//s:1:S\n"
				"//s:1:sigma\n"
				"//s:1:K\n"
				"//s:1:T\n"
				"//s:1:R\n"
				"//o:1:call\n"
				"//workspace:1024\n"
				"//!!multipleOutputInfo:0:1:\n"
				"//!!fullAddressTrans:0:\n"
				"//!!reductionFactor:0:\n"
				"")
				.sampler(1, 0)
				.sampler(2, 0)
				.sampler(3, 0)
				.sampler(4, 0)
				.sampler(5, 0)
				.interpolant(1, kStreamInterpolant_Position)
				.interpolant(2, kStreamInterpolant_Position)
				.interpolant(3, kStreamInterpolant_Position)
				.interpolant(4, kStreamInterpolant_Position)
				.interpolant(5, kStreamInterpolant_Position)
				.output(6, 0)
			)
		);
	static const void* __black_scholes_call_gles = &__black_scholes_call_gles_desc;
}

static void  __black_scholes_call_cpu_inner(const __BrtFloat1  &S,
                                           const __BrtFloat1  &sigma,
                                           const __BrtFloat1  &K,
                                           const __BrtFloat1  &T,
                                           const __BrtFloat1  &R,
                                           __BrtFloat1  &call)
{
  __BrtFloat2  d;
  __BrtFloat1  K_exp_minus_r_T;
  __BrtFloat1  two = __BrtFloat1(2.000000f);
  __BrtFloat2  temp;
  __BrtFloat2  temp2;
  __BrtFloat3  temp3;
  __BrtFloat3  temp4;
  __BrtFloat3  temp5;
  __BrtFloat3  temp6;

  temp3 = __BrtFloat3 (sigma,sigma,-R);
  temp4 = __BrtFloat3 (__sqrt_cpu_inner(T),sigma,T);
  temp5 = temp3 * temp4;
  temp4.mask2(__BrtFloat2 (K,two),maskX,maskY);
  temp3.mask2(__BrtFloat2 (S,temp5.swizzle1(maskY)),maskX,maskY);
  temp6.mask2(temp3.swizzle2(maskX, maskY) / temp4.swizzle2(maskX, maskY),maskX,maskY);
  d.mask1((__log_cpu_inner(temp6.swizzle1(maskX)) + (R + temp6.swizzle1(maskY)) * T) / temp5.swizzle1(maskX),maskX);
  d.mask1(d.swizzle1(maskX) - temp5.swizzle1(maskX),maskY);
  K_exp_minus_r_T = K * __exp_cpu_inner(temp5.swizzle1(maskZ));
  temp = __BrtFloat2 (S,K_exp_minus_r_T);
  temp2 = temp * __PHI_cpu_inner(d);
  call = temp2.swizzle1(maskX) - temp2.swizzle1(maskY);
}
void  __black_scholes_call_cpu(::brook::Kernel *__k, const std::vector<void *>&args, int __brt_idxstart, int __brt_idxend, bool __brt_isreduce)
{
  ::brook::StreamInterface *arg_S = (::brook::StreamInterface *) args[0];
  ::brook::StreamInterface *arg_sigma = (::brook::StreamInterface *) args[1];
  ::brook::StreamInterface *arg_K = (::brook::StreamInterface *) args[2];
  ::brook::StreamInterface *arg_T = (::brook::StreamInterface *) args[3];
  ::brook::StreamInterface *arg_R = (::brook::StreamInterface *) args[4];
  ::brook::StreamInterface *arg_call = (::brook::StreamInterface *) args[5];
  
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 16) if(!__brt_isreduce)
#endif
  for(int __brt_idx=__brt_idxstart; __brt_idx<__brt_idxend; __brt_idx++) {
    Addressable <__BrtFloat1  > __out_arg_call((__BrtFloat1 *) __k->FetchElem(arg_call, __brt_idx));
    __black_scholes_call_cpu_inner (Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_S, __brt_idx)),
                                    Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_sigma, __brt_idx)),
                                    Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_K, __brt_idx)),
                                    Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_T, __brt_idx)),
                                    Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_R, __brt_idx)),
                                    __out_arg_call);
    *reinterpret_cast<__BrtFloat1 *>(__out_arg_call.address) = __out_arg_call.castToArg(*reinterpret_cast<__BrtFloat1 *>(__out_arg_call.address));
  }
}

extern void  black_scholes_call (::brook::stream S,
		::brook::stream sigma,
		::brook::stream K,
		::brook::stream T,
		::brook::stream R,
		::brook::stream call) {
  static const void *__black_scholes_call_fp[] = {
     "fp30", __black_scholes_call_fp30,
     "fp40", __black_scholes_call_fp40,
     "arb", __black_scholes_call_arb,
     "glsl", __black_scholes_call_glsl,
     "gles", __black_scholes_call_gles,
     "ps20", __black_scholes_call_ps20,
     "ps2b", __black_scholes_call_ps2b,
     "ps2a", __black_scholes_call_ps2a,
     "ps30", __black_scholes_call_ps30,
     "ctm", __black_scholes_call_ctm,
     "cpu", (void *) __black_scholes_call_cpu,
     NULL, NULL };
  static BRTTLS ::brook::kernel* __pk;
  if(!__pk) __pk = new ::brook::kernel;
  __pk->initialize(__black_scholes_call_fp);
  ::brook::kernel& __k = *__pk;

  __k->PushStream(S);
  __k->PushStream(sigma);
  __k->PushStream(K);
  __k->PushStream(T);
  __k->PushStream(R);
  __k->PushOutput(call);
  __k->Map();

}


static const char *__black_scholes_put_ps20= NULL;
static const char *__black_scholes_put_ps2b= NULL;
static const char *__black_scholes_put_ps2a= NULL;
static const char *__black_scholes_put_ps30= NULL;
static const char *__black_scholes_put_ctm= NULL;
static const char *__black_scholes_put_fp30= NULL;
static const char *__black_scholes_put_fp40= NULL;
static const char *__black_scholes_put_arb= NULL;
static const char *__black_scholes_put_glsl= NULL;

namespace {
	using namespace ::brook::desc;
	static const gpu_kernel_desc __black_scholes_put_gles_desc = gpu_kernel_desc()
		.technique( gpu_technique_desc()
			.pass( gpu_pass_desc(
				"// glesf output by Cg compiler\n"
				"// cgc version 3.1.0013, build date Apr 18 2012\n"
				"// command line args: -quiet -DCGC=1 -profile glesf -DGL_ES\n"
				"// source file: /tmp/brookAzh8Om\n"
				"//vendor NVIDIA Corporation\n"
				"//version 3.1.0.13\n"
				"//profile glesf\n"
				"//program main\n"
				"//semantic main._tex_S : TEXUNIT0\n"
				"//semantic main._tex_sigma : TEXUNIT1\n"
				"//semantic main._tex_K : TEXUNIT2\n"
				"//semantic main._tex_T : TEXUNIT3\n"
				"//semantic main._tex_R : TEXUNIT4\n"
				"//semantic main.__workspace : C0\n"
				"//var sampler2D _tex_S : TEXUNIT0 : _tex_S1 0 : 0 : 1\n"
				"//var sampler2D _tex_sigma : TEXUNIT1 : _tex_sigma1 1 : 2 : 1\n"
				"//var sampler2D _tex_K : TEXUNIT2 : _tex_K1 2 : 4 : 1\n"
				"//var sampler2D _tex_T : TEXUNIT3 : _tex_T1 3 : 6 : 1\n"
				"//var sampler2D _tex_R : TEXUNIT4 : _tex_R1 4 : 8 : 1\n"
				"//var float2 _tex_S_pos : $vin.TEXCOORD0 : TEXCOORD0 : 1 : 1\n"
				"//var float2 _tex_sigma_pos : $vin.TEXCOORD1 : TEXCOORD1 : 3 : 1\n"
				"//var float2 _tex_K_pos : $vin.TEXCOORD2 : TEXCOORD2 : 5 : 1\n"
				"//var float2 _tex_T_pos : $vin.TEXCOORD3 : TEXCOORD3 : 7 : 1\n"
				"//var float2 _tex_R_pos : $vin.TEXCOORD4 : TEXCOORD4 : 9 : 1\n"
				"//var float __output_0 : $vout.COLOR0 : COL : 10 : 1\n"
				"\n"
				"precision highp float;\n"
				"\n"
				"struct double_struct {\n"
				"    vec2 _x;\n"
				"};\n"
				"\n"
				"struct double2_struct {\n"
				"    vec4 _xy;\n"
				"};\n"
				"\n"
				"float _TMP2;\n"
				"vec4 _TMP0;\n"
				"uniform sampler2D _tex_S1;\n"
				"uniform sampler2D _tex_sigma1;\n"
				"uniform sampler2D _tex_K1;\n"
				"uniform sampler2D _tex_T1;\n"
				"uniform sampler2D _tex_R1;\n"
				"vec3 _temp30038;\n"
				"vec3 _temp40038;\n"
				"vec3 _temp50038;\n"
				"vec3 _temp60038;\n"
				"vec2 _d0038;\n"
				"float _K_exp_minus_r_T0038;\n"
				"vec2 _temp0038;\n"
				"vec2 _temp20038;\n"
				"float _put0038;\n"
				"vec3 _TMP47;\n"
				"vec2 _X0052;\n"
				"vec2 _t0052;\n"
				"vec2 _y0052;\n"
				"vec2 _result0052;\n"
				"vec2 _x0056;\n"
				"varying vec4 TEX0;\n"
				"varying vec4 TEX1;\n"
				"varying vec4 TEX2;\n"
				"varying vec4 TEX3;\n"
				"varying vec4 TEX4;\n"
				"\n"
				" // main procedure, the original name was main\n"
				"void main()\n"
				"{\n"
				"\n"
				"    float _S1;\n"
				"    float _sigma1;\n"
				"    float _K1;\n"
				"    float _T1;\n"
				"\n"
				"    reconstruct_float(_TMP0.x, _tex_S1, TEX0.xy);\n"
				"    _S1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_sigma1, TEX1.xy);\n"
				"    _sigma1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_K1, TEX2.xy);\n"
				"    _K1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_T1, TEX3.xy);\n"
				"    _T1 = _TMP0.x;\n"
				"    reconstruct_float(_TMP0.x, _tex_R1, TEX4.xy);\n"
				"    _temp40038.x = sqrt(_T1);\n"
				"    _temp30038 = vec3(_sigma1, _sigma1, -_TMP0.x);\n"
				"    _temp40038.yz = vec2(_sigma1, _T1);\n"
				"    _temp50038 = _temp30038*_temp40038;\n"
				"    _temp40038.xy = vec2(_K1, 2.0);\n"
				"    _temp30038.xy = vec2(_S1, _temp50038.y);\n"
				"    _temp60038.xy = _temp30038.xy/_temp40038.xy;\n"
				"    _TMP2 = log(_temp60038.x);\n"
				"    _d0038.y = (_TMP2 + (_TMP0.x + _temp60038.y)*_T1)/_temp50038.x;\n"
				"    _d0038.x = _d0038.y - _temp50038.x;\n"
				"    _X0052 = -_d0038;\n"
				"    _t0052 = 1.0/(1.0 + 2.31641993E-01*abs(_X0052));\n"
				"    _x0056 = (-_X0052*_X0052)/2.0;\n"
				"    _temp50038.xy = _x0056;\n"
				"    _TMP47 = exp(_temp50038.xyz);\n"
				"    _K_exp_minus_r_T0038 = _K1*_TMP47.z;\n"
				"    _temp0038 = vec2(_K_exp_minus_r_T0038, _S1);\n"
				"    _y0052 = 1.0 - (3.98941994E-01*_TMP47.xy)*_t0052*(3.19382012E-01 + _t0052*(-3.56563985E-01 + _t0052*(1.78147805 + _t0052*(-1.82125604 + _t0052*1.33027399))));\n"
				"    _result0052=abs(step(_X0052, vec2(0.0))-_y0052);\n"
				"    _temp20038 = _temp0038*_result0052;\n"
				"    _put0038 = _temp20038.x - _temp20038.y;\n"
				"    encode_output_float( _put0038);\n"
				"} //  \n"
				"//!!BRCC\n"
				"//narg:6\n"
				"//s:1:S\n"
				"//s:1:sigma\n"
				"//s:1:K\n"
				"//s:1:T\n"
				"//s:1:R\n"
				"//o:1:put\n"
				"//workspace:1024\n"
				"//!!multipleOutputInfo:0:1:\n"
				"//!!fullAddressTrans:0:\n"
				"//!!reductionFactor:0:\n"
				"")
				.sampler(1, 0)
				.sampler(2, 0)
				.sampler(3, 0)
				.sampler(4, 0)
				.sampler(5, 0)
				.interpolant(1, kStreamInterpolant_Position)
				.interpolant(2, kStreamInterpolant_Position)
				.interpolant(3, kStreamInterpolant_Position)
				.interpolant(4, kStreamInterpolant_Position)
				.interpolant(5, kStreamInterpolant_Position)
				.output(6, 0)
			)
		);
	static const void* __black_scholes_put_gles = &__black_scholes_put_gles_desc;
}

static void  __black_scholes_put_cpu_inner(const __BrtFloat1  &S,
                                          const __BrtFloat1  &sigma,
                                          const __BrtFloat1  &K,
                                          const __BrtFloat1  &T,
                                          const __BrtFloat1  &R,
                                          __BrtFloat1  &put)
{
  __BrtFloat2  d;
  __BrtFloat1  K_exp_minus_r_T;
  __BrtFloat1  two = __BrtFloat1(2.000000f);
  __BrtFloat2  temp;
  __BrtFloat2  temp2;
  __BrtFloat3  temp3;
  __BrtFloat3  temp4;
  __BrtFloat3  temp5;
  __BrtFloat3  temp6;

  temp3 = __BrtFloat3 (sigma,sigma,-R);
  temp4 = __BrtFloat3 (__sqrt_cpu_inner(T),sigma,T);
  temp5 = temp3 * temp4;
  temp4.mask2(__BrtFloat2 (K,two),maskX,maskY);
  temp3.mask2(__BrtFloat2 (S,temp5.swizzle1(maskY)),maskX,maskY);
  temp6.mask2(temp3.swizzle2(maskX, maskY) / temp4.swizzle2(maskX, maskY),maskX,maskY);
  d.mask1((__log_cpu_inner(temp6.swizzle1(maskX)) + (R + temp6.swizzle1(maskY)) * T) / temp5.swizzle1(maskX),maskY);
  d.mask1(d.swizzle1(maskY) - temp5.swizzle1(maskX),maskX);
  K_exp_minus_r_T = K * __exp_cpu_inner(temp5.swizzle1(maskZ));
  temp = __BrtFloat2 (K_exp_minus_r_T,S);
  temp2 = temp * __PHI_cpu_inner(-d);
  put = temp2.swizzle1(maskX) - temp2.swizzle1(maskY);
}
void  __black_scholes_put_cpu(::brook::Kernel *__k, const std::vector<void *>&args, int __brt_idxstart, int __brt_idxend, bool __brt_isreduce)
{
  ::brook::StreamInterface *arg_S = (::brook::StreamInterface *) args[0];
  ::brook::StreamInterface *arg_sigma = (::brook::StreamInterface *) args[1];
  ::brook::StreamInterface *arg_K = (::brook::StreamInterface *) args[2];
  ::brook::StreamInterface *arg_T = (::brook::StreamInterface *) args[3];
  ::brook::StreamInterface *arg_R = (::brook::StreamInterface *) args[4];
  ::brook::StreamInterface *arg_put = (::brook::StreamInterface *) args[5];
  
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 16) if(!__brt_isreduce)
#endif
  for(int __brt_idx=__brt_idxstart; __brt_idx<__brt_idxend; __brt_idx++) {
    Addressable <__BrtFloat1  > __out_arg_put((__BrtFloat1 *) __k->FetchElem(arg_put, __brt_idx));
    __black_scholes_put_cpu_inner (Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_S, __brt_idx)),
                                   Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_sigma, __brt_idx)),
                                   Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_K, __brt_idx)),
                                   Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_T, __brt_idx)),
                                   Addressable <__BrtFloat1 >((__BrtFloat1 *) __k->FetchElem(arg_R, __brt_idx)),
                                   __out_arg_put);
    *reinterpret_cast<__BrtFloat1 *>(__out_arg_put.address) = __out_arg_put.castToArg(*reinterpret_cast<__BrtFloat1 *>(__out_arg_put.address));
  }
}

extern void  black_scholes_put (::brook::stream S,
		::brook::stream sigma,
		::brook::stream K,
		::brook::stream T,
		::brook::stream R,
		::brook::stream put) {
  static const void *__black_scholes_put_fp[] = {
     "fp30", __black_scholes_put_fp30,
     "fp40", __black_scholes_put_fp40,
     "arb", __black_scholes_put_arb,
     "glsl", __black_scholes_put_glsl,
     "gles", __black_scholes_put_gles,
     "ps20", __black_scholes_put_ps20,
     "ps2b", __black_scholes_put_ps2b,
     "ps2a", __black_scholes_put_ps2a,
     "ps30", __black_scholes_put_ps30,
     "ctm", __black_scholes_put_ctm,
     "cpu", (void *) __black_scholes_put_cpu,
     NULL, NULL };
  static BRTTLS ::brook::kernel* __pk;
  if(!__pk) __pk = new ::brook::kernel;
  __pk->initialize(__black_scholes_put_fp);
  ::brook::kernel& __k = *__pk;

  __k->PushStream(S);
  __k->PushStream(sigma);
  __k->PushStream(K);
  __k->PushStream(T);
  __k->PushStream(R);
  __k->PushOutput(put);
  __k->Map();

}


float  PHI_cpu(float  X)
{
  float  y;
  float  absX;
  float  t;
  const float  c1 = 0.319382f;
  const float  c2 = -0.356564f;
  const float  c3 = 1.781478f;
  const float  c4 = -1.821256f;
  const float  c5 = 1.330274f;
  const float  one_by_sqrt_2_pi = 0.398942f;

  absX = fabs(X);
  t = 1.000000f / (1.000000f + 0.231642f * absX);
  y = 1.000000f - one_by_sqrt_2_pi * exp(-X * X / 2.000000f) * t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * c5))));
  return (X < 0) ? (1.000000f - y) : (y);
}

void  black_scholes_cpu(float  *S, float  *K, float  *T, float  *r, float  *sigma, float  *call, float  *put, unsigned int  Width)
{
  unsigned int  y;

  for (y = 0; y < Width; ++y)
  {
    float  d1;
    float  d2;
    float  sigma_sqrt_T;
    float  K_exp_minus_r_T;

    sigma_sqrt_T = sigma[y] * sqrt(T[y]);
    d1 = (log(S[y] / K[y]) + (r[y] + sigma[y] * sigma[y] / 2.000000f) * T[y]) / sigma_sqrt_T;
    d2 = d1 - sigma_sqrt_T;
    K_exp_minus_r_T = K[y] * exp(-r[y] * T[y]);
    call[y] = S[y] * PHI_cpu(d1) - K_exp_minus_r_T * PHI_cpu(d2);
    put[y] = K_exp_minus_r_T * PHI_cpu(-d2) - S[y] * PHI_cpu(-d1);
  }

}

float  frand(float  lowerLimit, float  upperLimit)
{
  unsigned int  intRand;
  float  t;

  assert(upperLimit > lowerLimit);
  intRand = rand();
  t = (float ) (intRand) / (float ) (RAND_MAX);
  return lowerLimit * t + upperLimit * (1.000000f - t);
}

int  main(int  argc, char  **argv)
{
  unsigned int  i;
  BRTALIGNED float  *inputArray[5] = { NULL };
  float  *callArray = NULL;
  float  *putArray = NULL;
  struct infoRec  cmd;
  unsigned int  Width;
  unsigned int  dim1;
  unsigned int  dim2;

  Setup(0);
  Setup(1);
  ParseCommandLine(argc,argv,&cmd);
  srand(cmd.Seed);
  Width = cmd.Width;
  if (Width < 4)
  {
    Width = 1;
    dim1 = 1;
    dim2 = 1;
  }

  else
  {
    unsigned int  tempVar1 = (unsigned int ) (sqrt((double ) (Width)));

    dim1 = tempVar1;
    dim2 = tempVar1;
  }

  Width = dim1 * dim2;
  cmd.Width = Width;
  if (Width != cmd.Width)
  {
    printf("The modified width is : %u\n",Width);
  }

  if (cmd.Height != DEFAULT_HEIGHT)
  {
    fprintf(stderr,"Warning: \n\t'Height' is not taken from the commandline.\n");
    fprintf(stderr,"\tIt is not used in this Brook sample.\n");
    fprintf(stderr,"\tPlease use 'Width' if you wish to change the number of options.\n\n");
  }

  callArray = allocate_mat_f(1,Width);
  putArray = allocate_mat_f(1,Width);
  if (!putArray || !callArray)
  {
    fprintf(stderr,"Error: Memory Allocation.\n");
    retval = -1;
    goto cleanup;
  }

  for (i = 0; i < 5; ++i)
  {
    if (!(inputArray[i] = allocate_mat_f(1,Width)))
    {
      fprintf(stderr,"Error: Memory Allocation.\n");
      retval = -1;
      goto cleanup;
    }

  }

  for (i = 0; i < Width; ++i)
  {
    inputArray[0][i] = frand(S_LOWER_LIMIT,S_UPPER_LIMIT);
    inputArray[1][i] = frand(K_LOWER_LIMIT,K_UPPER_LIMIT);
    inputArray[2][i] = frand(T_LOWER_LIMIT,T_UPPER_LIMIT);
    inputArray[3][i] = frand(r_LOWER_LIMIT,r_UPPER_LIMIT);
    inputArray[4][i] = frand(sigma_LOWER_LIMIT,sigma_UPPER_LIMIT);
  }

  if (cmd.Verbose)
  {
    print_mat_f("S Array: ","%lf ",inputArray[0],1,Width);
    print_mat_f("K Array: ","%lf ",inputArray[1],1,Width);
    print_mat_f("T Array: ","%lf ",inputArray[2],1,Width);
    print_mat_f("R Array: ","%lf ",inputArray[3],1,Width);
    print_mat_f("Sigma Array: ","%lf ",inputArray[4],1,Width);
  }

  else
    if (!cmd.Quiet)
    {
      printf("Printing first row of data, for more use -v\n");
      print_mat_f("S Array: ","%3.2lf ",inputArray[0],1,Width);
      print_mat_f("K Array: ","%3.2lf ",inputArray[1],1,Width);
      print_mat_f("T Array: ","%3.2lf ",inputArray[2],1,Width);
      print_mat_f("R Array: ","%3.2lf ",inputArray[3],1,Width);
      print_mat_f("Sigma Array: ","%3.2lf ",inputArray[4],1,Width);
    }

  {
    ::brook::stream S_stream(::brook::getStreamType(( float  *)0), dim1 , dim2,-1);
    ::brook::stream K_stream(::brook::getStreamType(( float  *)0), dim1 , dim2,-1);
    ::brook::stream T_stream(::brook::getStreamType(( float  *)0), dim1 , dim2,-1);
    ::brook::stream r_stream(::brook::getStreamType(( float  *)0), dim1 , dim2,-1);
    ::brook::stream sigma_stream(::brook::getStreamType(( float  *)0), dim1 , dim2,-1);
    ::brook::stream call_stream(::brook::getStreamType(( float  *)0), dim1 , dim2,-1);
    ::brook::stream put_stream(::brook::getStreamType(( float  *)0), dim1 , dim2,-1);

    Start(0);
    for (i = 0; i < cmd.Iterations; ++i)
    {
      streamRead(S_stream,inputArray[0]);
      streamRead(K_stream,inputArray[1]);
      streamRead(T_stream,inputArray[2]);
      streamRead(r_stream,inputArray[3]);
      streamRead(sigma_stream,inputArray[4]);
      black_scholes_call(S_stream,sigma_stream,K_stream,T_stream,r_stream,call_stream);
      black_scholes_put(S_stream,sigma_stream,K_stream,T_stream,r_stream,put_stream);
      streamWrite(call_stream,callArray);
      streamWrite(put_stream,putArray);
    }

    Stop(0);
  }

  if (cmd.Verbose)
  {
    print_mat_f("Call: ","%lf ",callArray,1,Width);
    print_mat_f("Put: ","%lf ",putArray,1,Width);
  }

  else
    if (!cmd.Quiet)
    {
      printf("Printing first row of data, for more use -v\n");
      print_mat_f("Call: ","%3.2lf ",callArray,1,Width);
      print_mat_f("Put: ","%3.2lf ",putArray,1,Width);
    }

  if (cmd.Timing)
  {
    double  time = GetElapsedTime(0);

    printf("%-8s%-8s%-16s%-16s\n","Width","Height","Iterations","GPU Total Time");
    printf("%-8d%-8d%-16d%-16lf",cmd.Width,cmd.Height,cmd.Iterations,time);
    printf("\n\n");
  }

  if (cmd.Verify)
  {
    int  res = 0;
    float  *call;
    float  *put;

    call = allocate_mat_f(1,Width);
    put = allocate_mat_f(1,Width);
    if (!call || !put)
    {
      fprintf(stderr,"Error: Memory Allocation.\n");
      retval = -1;
      goto cpu_cleanup;
    }

    printf("-e Verify correct output.\n");
    printf("Performing Black Scholes Pricing on CPU ... ");
    black_scholes_cpu(inputArray[0],inputArray[1],inputArray[2],inputArray[3],inputArray[4],call,put,Width);
    printf("Done\n");
    res += compare_mat_f(callArray,call,1,Width);
    res += compare_mat_f(putArray,put,1,Width);
    if (res)
    {
      printf("%s: Failed!\n\n",argv[0]);
      retval = -1;
    }

    else
    {
      printf("%s: Passed!\n\n",argv[0]);
      retval = 0;
    }

    if (cmd.Performance)
    {
      double  cpu_time = 0.000000f;
      double  gpu_time = 0.000000f;
      float  *call;
      float  *put;

      call = allocate_mat_f(1,Width);
      put = allocate_mat_f(1,Width);
      if (!call || !put)
      {
        fprintf(stderr,"Error: Memory Allocation.\n");
        retval = -1;
        goto cpu_cleanup;
      }

      printf("-p Compare performance with CPU.\n");
      Start(1);
      for (i = 0; i < cmd.Iterations; ++i)
      {
        black_scholes_cpu(inputArray[0],inputArray[1],inputArray[2],inputArray[3],inputArray[4],call,put,Width);
      }

      Stop(1);
      cpu_time = GetElapsedTime(1);
      gpu_time = GetElapsedTime(0);
      printf("%-8s%-8s%-16s%-16s%-16s%-16s\n","Width","Height","Iterations","CPU Total Time","GPU Total Time","Speedup");
      printf("%-8d%-8d%-16d%-16lf%-16lf%-16lf",cmd.Width,cmd.Height,cmd.Iterations,cpu_time,gpu_time,cpu_time / gpu_time);
      printf("\n\n");
    }

  cpu_cleanup:
    if (call)
    {
      free(call);
    }

    if (put)
    {
      free(put);
    }

  }

cleanup:
  if (callArray)
  {
    free(callArray);
  }

  if (putArray)
  {
    free(putArray);
  }

  for (i = 0; i < 5; ++i)
  {
    if (inputArray[i])
    {
      free(inputArray[i]);
    }

  }

  if (!cmd.Verify)
  {
    printf("\nPress enter to exit...\n");
    getchar();
  }

  return retval;
}

