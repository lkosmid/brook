
////////////////////////////////////////////
// Generated by BRCC v0.1
// BRCC Compiled on: Mar 19 2017 23:54:49
////////////////////////////////////////////

#include <brook/brook.hpp>
#include "common.h"

#include "Timer.h"

#include <stdio.h>

static int  retval = 0;
static const char *__cholesky_ps20= NULL;
static const char *__cholesky_ps2b= NULL;
static const char *__cholesky_ps2a= NULL;
static const char *__cholesky_ps30= NULL;
static const char *__cholesky_ctm= NULL;
static const char *__cholesky_fp30= NULL;
static const char *__cholesky_fp40= NULL;
static const char *__cholesky_arb= NULL;
static const char *__cholesky_glsl= NULL;

namespace {
	using namespace ::brook::desc;
	static const gpu_kernel_desc __cholesky_gles_desc = gpu_kernel_desc()
		.technique( gpu_technique_desc()
			.pass( gpu_pass_desc(
				"// glesf output by Cg compiler\n"
				"// cgc version 3.1.0013, build date Apr 18 2012\n"
				"// command line args: -quiet -DCGC=1 -profile glesf -DGL_ES\n"
				"// source file: /tmp/brookTBzij0\n"
				"//vendor NVIDIA Corporation\n"
				"//version 3.1.0.13\n"
				"//profile glesf\n"
				"//program main\n"
				"//semantic main.A : TEXUNIT0\n"
				"//semantic main.__gatherconst_A : C0\n"
				"//semantic main.L : TEXUNIT1\n"
				"//semantic main.__gatherconst_L : C1\n"
				"//semantic main._const_output_invscalebias : C2\n"
				"//semantic main.StreamDim : C3\n"
				"//semantic main.column : C4\n"
				"//semantic main.__workspace : C5\n"
				"//var sampler2D A[0] : TEXUNIT0 : _A2[0] 0 : 0 : 1\n"
				"//var float4 __gatherconst_A : C0 : _gatherconst_A1 : 1 : 1\n"
				"//var sampler2D L[0] : TEXUNIT1 : _L2[0] 1 : 2 : 1\n"
				"//var float4 __gatherconst_L : C1 : _gatherconst_L1 : 3 : 1\n"
				"//var float4 _const_output_invscalebias : C2 : _const_output_invscalebias1 : 5 : 1\n"
				"//var float4 StreamDim : C3 : _StreamDim1 : 7 : 1\n"
				"//var float column : C4 : _column2 : 8 : 1\n"
				"//var float __output_0 : $vout.COLOR0 : COL : 4 : 1\n"
				"//var float2 _tex_output_pos : $vin.TEXCOORD0 : TEXCOORD0 : 6 : 1\n"
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
				"vec2 _TMP4;\n"
				"vec4 _TMP5;\n"
				"vec2 _TMP3;\n"
				"vec2 _TMP2;\n"
				"float _TMP6;\n"
				"vec2 _TMP1;\n"
				"vec2 _TMP0;\n"
				"uniform sampler2D _A2[1];\n"
				"uniform vec4 _gatherconst_A1;\n"
				"uniform sampler2D _L2[1];\n"
				"uniform vec4 _gatherconst_L1;\n"
				"uniform vec4 _const_output_invscalebias1;\n"
				"uniform vec4 _StreamDim1;\n"
				"uniform float _column2;\n"
				"vec4 _x0017;\n"
				"float _output0018;\n"
				"float _Akk0018;\n"
				"float _sum0018;\n"
				"float _i0018;\n"
				"float _temp0018;\n"
				"float _Lkk0018;\n"
				"vec2 _index0020;\n"
				"vec2 _index0026;\n"
				"float _a0032;\n"
				"vec2 _index0036;\n"
				"vec2 _index0042;\n"
				"vec2 _index0048;\n"
				"float _TMP53;\n"
				"varying vec4 TEX0;\n"
				"\n"
				" // main procedure, the original name was main\n"
				"void main()\n"
				"{\n"
				"\n"
				"    vec4 _indexofoutput;\n"
				"    vec2 _TMP15;\n"
				"\n"
				"    _TMP15 = TEX0.xy*_const_output_invscalebias1.xy + _const_output_invscalebias1.zw;\n"
				"    _x0017 = _StreamDim1*vec4(_TMP15.x, _TMP15.y, 0.00000000E+00, 0.00000000E+00);\n"
				"    _indexofoutput = floor(_x0017);\n"
				"    if (_column2 == _indexofoutput.x) { // if begin\n"
				"        if (_indexofoutput.x > _indexofoutput.y) { // if begin\n"
				"            _output0018 = 0.00000000E+00;\n"
				"        } else {\n"
				"            _index0020 = vec2(_indexofoutput.x, _indexofoutput.x);\n"
				"            _TMP0 = _index0020*_gatherconst_A1.xy + _gatherconst_A1.zw;\n"
				"            reconstruct_float(_TMP5.x, _A2[0], _TMP0);\n"
				"            _Akk0018 = _TMP5.x;\n"
				"            _sum0018 = 0.00000000E+00;\n"
				"            _i0018 = 0.00000000E+00;\n"
				"            _TMP53 = 0.00000000E+00;\n"
				"            if (0.00000000E+00 > _indexofoutput.x - 1.00000000E+00) { // if begin\n"
				"                _TMP53 = -1.00000000E+00;\n"
				"            } // end if\n"
				"            if (_TMP53 == 0.00000000E+00) { // if begin\n"
				"                _index0026 = vec2(0.00000000E+00, _indexofoutput.x);\n"
				"                _TMP1 = _index0026*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                reconstruct_float(_TMP5.x, _L2[0], _TMP1);\n"
				"                _sum0018 = _TMP5.x*_TMP5.x;\n"
				"            } // end if\n"
				"            if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                _i0018 = 1.00000000E+00;\n"
				"            } // end if\n"
				"            if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                if (_i0018 > _indexofoutput.x - 1.00000000E+00) { // if begin\n"
				"                    _TMP53 = -1.00000000E+00;\n"
				"                } // end if\n"
				"                if (_TMP53 == 0.00000000E+00) { // if begin\n"
				"                    _index0026 = vec2(_i0018, _indexofoutput.x);\n"
				"                    _TMP1 = _index0026*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                    reconstruct_float(_TMP5.x, _L2[0], _TMP1);\n"
				"                    _sum0018 = _sum0018 + _TMP5.x*_TMP5.x;\n"
				"                } // end if\n"
				"                if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                    _i0018 = _i0018 + 1.00000000E+00;\n"
				"                } // end if\n"
				"                if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                    if (_i0018 > _indexofoutput.x - 1.00000000E+00) { // if begin\n"
				"                        _TMP53 = -1.00000000E+00;\n"
				"                    } // end if\n"
				"                    if (_TMP53 == 0.00000000E+00) { // if begin\n"
				"                        _index0026 = vec2(_i0018, _indexofoutput.x);\n"
				"                        _TMP1 = _index0026*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                        reconstruct_float(_TMP5.x, _L2[0], _TMP1);\n"
				"                        _sum0018 = _sum0018 + _TMP5.x*_TMP5.x;\n"
				"                    } // end if\n"
				"                    if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                        _i0018 = _i0018 + 1.00000000E+00;\n"
				"                    } // end if\n"
				"                    if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                        if (_i0018 > _indexofoutput.x - 1.00000000E+00) { // if begin\n"
				"                            _TMP53 = -1.00000000E+00;\n"
				"                        } // end if\n"
				"                        if (_TMP53 == 0.00000000E+00) { // if begin\n"
				"                            _index0026 = vec2(_i0018, _indexofoutput.x);\n"
				"                            _TMP1 = _index0026*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                            reconstruct_float(_TMP5.x, _L2[0], _TMP1);\n"
				"                            _sum0018 = _sum0018 + _TMP5.x*_TMP5.x;\n"
				"                        } // end if\n"
				"                        if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                            _i0018 = _i0018 + 1.00000000E+00;\n"
				"                        } // end if\n"
				"                        if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                            if (_i0018 > _indexofoutput.x - 1.00000000E+00) { // if begin\n"
				"                                _TMP53 = -1.00000000E+00;\n"
				"                            } // end if\n"
				"                            if (_TMP53 == 0.00000000E+00) { // if begin\n"
				"                                _index0026 = vec2(_i0018, _indexofoutput.x);\n"
				"                                _TMP1 = _index0026*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                                reconstruct_float(_TMP5.x, _L2[0], _TMP1);\n"
				"                                _sum0018 = _sum0018 + _TMP5.x*_TMP5.x;\n"
				"                            } // end if\n"
				"                            if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                                _i0018 = _i0018 + 1.00000000E+00;\n"
				"                            } // end if\n"
				"                            if (_TMP53 >= 0.00000000E+00) { // if begin\n"
				"                                if (_i0018 > _indexofoutput.x - 1.00000000E+00) { // if begin\n"
				"                                    _TMP53 = -1.00000000E+00;\n"
				"                                } // end if\n"
				"                                if (_TMP53 == 0.00000000E+00) { // if begin\n"
				"                                    _index0026 = vec2(_i0018, _indexofoutput.x);\n"
				"                                    _TMP1 = _index0026*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                                    reconstruct_float(_TMP5.x, _L2[0], _TMP1);\n"
				"                                    _sum0018 = _sum0018 + _TMP5.x*_TMP5.x;\n"
				"                                } // end if\n"
				"                            } // end if\n"
				"                        } // end if\n"
				"                    } // end if\n"
				"                } // end if\n"
				"            } // end if\n"
				"            _a0032 = _Akk0018 - _sum0018;\n"
				"            _TMP6 = inversesqrt(_a0032);\n"
				"            _Lkk0018 = 1.00000000E+00/_TMP6;\n"
				"            if (_indexofoutput.x == _indexofoutput.y) { // if begin\n"
				"                _output0018 = _Lkk0018;\n"
				"            } else {\n"
				"                _sum0018 = 0.00000000E+00;\n"
				"                for (float _i0019 = 0.0; _i0019 < 6.00000000E+00; _i0019 = _i0019 + 1.00000000E+00) { // for begin\n"
				"                    if (_i0019 > _indexofoutput.x - 1.00000000E+00) { // if begin\n"
				"                        { // break begin\n"
				"                            break;\n"
				"                        } // end break\n"
				"                    } // end if\n"
				"                    _index0036 = vec2(_i0019, _indexofoutput.y);\n"
				"                    _TMP2 = _index0036*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                    reconstruct_float(_TMP5.x, _L2[0], _TMP2);\n"
				"                    _temp0018 = _TMP5.x;\n"
				"                    _index0042 = vec2(_i0019, _indexofoutput.y);\n"
				"                    _TMP3 = _index0042*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"                    reconstruct_float(_TMP5.x, _L2[0], _TMP3);\n"
				"                    _sum0018 = _sum0018 + _temp0018*_TMP5.x;\n"
				"                } // end for\n"
				"                _output0018 = (1.00000000E+00/_Lkk0018)*(_Akk0018 - _sum0018);\n"
				"            } // end if\n"
				"        } // end if\n"
				"        encode_output_float( _output0018);\n"
				"    } else {\n"
				"        if (_indexofoutput.x > _indexofoutput.y) { // if begin\n"
				"            _output0018 = 0.00000000E+00;\n"
				"            encode_output_float( _output0018);\n"
				"        } else {\n"
				"            _index0048 = vec2(_indexofoutput.x, _indexofoutput.y);\n"
				"            _TMP4 = _index0048*_gatherconst_L1.xy + _gatherconst_L1.zw;\n"
				"            gl_FragColor.xyzw = texture2D(_L2[0], _TMP4);\n"
				"        } // end if\n"
				"    } // end if\n"
				"} //  \n"
				"//!!BRCC\n"
				"//narg:4\n"
				"//c:1:A\n"
				"//c:1:L\n"
				"//oi:1:output\n"
				"//c:1:column\n"
				"//workspace:1024\n"
				"//!!multipleOutputInfo:0:1:\n"
				"//!!fullAddressTrans:0:\n"
				"//!!reductionFactor:0:\n"
				"")
				.constant(1, kGatherConstant_Shape)
				.constant(2, kGatherConstant_Shape)
				.constant(3, kOutputConstant_Indexof)
				.constant(3, StreamDim)
				.constant(4, 0)
				.sampler(1, 0)
				.sampler(2, 0)
				.interpolant(3, kOutputInterpolant_Position)
				.output(3, 0)
			)
		);
	static const void* __cholesky_gles = &__cholesky_gles_desc;
}

static void  __cholesky_cpu_inner(const __BrtArray<__BrtFloat1  > &A,
                                 const __BrtArray<__BrtFloat1  > &L,
                                 __BrtFloat1  &output,
                                 const __BrtFloat1  &column)
{
  __BrtFloat2  index = (indexof(output)).swizzle2(maskX, maskY);
  __BrtFloat1  i;
  __BrtFloat1  sum;
  __BrtFloat1  temp;
  __BrtFloat1  temp2;
  __BrtFloat1  Akk;
  __BrtFloat1  Lkk;
  const __BrtFloat1  maxIter = __BrtFloat1(6.000000f);

  if (column == index.swizzle1(maskX))
  {
    if (index.swizzle1(maskX) > index.swizzle1(maskY))
      output = __BrtFloat1(0.000000f);
    else
    {
      Akk = A[index.swizzle1(maskX)][index.swizzle1(maskX)];
      sum = __BrtFloat1(0.000000f);
      for (i = __BrtFloat1(0.000000f); i < maxIter; i += __BrtFloat1(1.000000f))
      {
        if (i > index.swizzle1(maskX) - __BrtFloat1(1.000000f))
          break;
        temp = L[index.swizzle1(maskX)][i];
        sum += temp * temp;
      }

      Lkk = __sqrt_cpu_inner(Akk - sum);
      if (index.swizzle1(maskX) == index.swizzle1(maskY))
        output = Lkk;
      else
      {
        sum = __BrtFloat1(0.000000f);
        for (i = __BrtFloat1(0.000000f); i < maxIter; i += __BrtFloat1(1.000000f))
        {
          if (i > index.swizzle1(maskX) - __BrtFloat1(1.000000f))
            break;
          temp = L[index.swizzle1(maskY)][i];
          temp2 = L[index.swizzle1(maskY)][i];
          sum += temp * temp2;
        }

        output = __BrtFloat1(1.000000f) / Lkk * (Akk - sum);
      }

    }

  }

  else
  {
    if (index.swizzle1(maskX) > index.swizzle1(maskY))
      output = __BrtFloat1(0.000000f);
    else
      output = L[index.swizzle1(maskY)][index.swizzle1(maskX)];
  }

}
void  __cholesky_cpu(::brook::Kernel *__k, const std::vector<void *>&args, int __brt_idxstart, int __brt_idxend, bool __brt_isreduce)
{
  __BrtArray<__BrtFloat1  > *arg_A = (__BrtArray<__BrtFloat1  > *) args[0];
  __BrtArray<__BrtFloat1  > *arg_L = (__BrtArray<__BrtFloat1  > *) args[1];
  ::brook::StreamInterface *arg_output = (::brook::StreamInterface *) args[2];
  __BrtFloat1 *arg_column = (__BrtFloat1 *) args[3];
  
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 16) if(!__brt_isreduce)
#endif
  for(int __brt_idx=__brt_idxstart; __brt_idx<__brt_idxend; __brt_idx++) {
    Addressable <__BrtFloat1  > __out_arg_output((__BrtFloat1 *) __k->FetchElem(arg_output, __brt_idx));
    __cholesky_cpu_inner (*arg_A,
                          *arg_L,
                          __out_arg_output,
                          *arg_column);
    *reinterpret_cast<__BrtFloat1 *>(__out_arg_output.address) = __out_arg_output.castToArg(*reinterpret_cast<__BrtFloat1 *>(__out_arg_output.address));
  }
}

extern void  cholesky (::brook::stream A,
		::brook::stream L,
		::brook::stream output,
		const float  & __input_column) {
  float  column(__input_column);
  static const void *__cholesky_fp[] = {
     "fp30", __cholesky_fp30,
     "fp40", __cholesky_fp40,
     "arb", __cholesky_arb,
     "glsl", __cholesky_glsl,
     "gles", __cholesky_gles,
     "ps20", __cholesky_ps20,
     "ps2b", __cholesky_ps2b,
     "ps2a", __cholesky_ps2a,
     "ps30", __cholesky_ps30,
     "ctm", __cholesky_ctm,
     "cpu", (void *) __cholesky_cpu,
     NULL, NULL };
  static BRTTLS ::brook::kernel* __pk;
  if(!__pk) __pk = new ::brook::kernel;
  __pk->initialize(__cholesky_fp);
  ::brook::kernel& __k = *__pk;

  __k->PushGatherStream(A);
  __k->PushGatherStream(L);
  __k->PushOutput(output);
  __k->PushConstant(column);
  __k->Map();

}


int  GenerateInputMatrix(unsigned int  w, unsigned int  h, float  *I, float  *M)
{
  unsigned int  index = 0;
  unsigned int  srcindex = 0;
  unsigned int  dstindex = 0;
  unsigned int  k = 0;
  unsigned int  j = 0;
  unsigned int  i = 0;
  float  value = 0.000000f;

  if (!I || !M)
  {
    printf("Null pointer!\n");
    return 0;
  }

  for (i = 0; i < h; i++)
  {
    for (j = 0; j < w; j++)
    {
      if (j <= i)
      {
        value = 0.000000f;
        for (k = 0; k <= j; k++)
        {
          value += (k + 1) * (k + 1);
        }

      }

      index = i * w + j;
      I[index] = value;
    }

  }

  for (i = 0; i < h; i++)
  {
    for (j = 0; j < w; j++)
    {
      srcindex = i * w + j;
      M[srcindex] = I[srcindex];
    }

  }

  return 1;
}

unsigned int  AdjustSampleSize(unsigned int  w, unsigned int  h)
{
  unsigned int  final_dimension = w;

  if (w != h)
  {
    final_dimension = (w > h) ? (w) : (h);
    printf("Only square matrices allowed. Setting dimension = %dx%d\n",final_dimension,final_dimension);
  }

  printf("Adjusted Matrix Dimensions: %dx%d\n",final_dimension,final_dimension);
  return final_dimension;
}

void  CholeskyCPU(unsigned int  width, unsigned int  height, float  *L)
{
  float  dot_product;
  float  sqrt_Akk;
  unsigned int  k = 0;
  unsigned int  i = 0;
  unsigned int  j = 0;
  unsigned int  p = 0;

  for (k = 0; k < width - 1; k++)
  {
    sqrt_Akk = sqrt(L[k * width + k]);
    L[k * width + k] /= sqrt_Akk;
    for (i = k + 1; i < width; i++)
    {
      L[i * width + k] /= sqrt_Akk;
    }

    for (j = k + 1; j < width; j++)
    {
      float  temp = L[j * width + k + 1];

      dot_product = 0.000000f;
      for (p = 0; p <= k; p++)
      {
        dot_product += L[j * width + p] * L[(k + 1) * width + p];
      }

      temp -= dot_product;
      L[j * width + k + 1] = temp;
    }

  }

  L[(height - 1) * width + height - 1] /= sqrt(L[(height - 1) * width + height - 1]);
  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      if (j > i)
      {
        L[i * width + j] = 0.000000f;
      }

    }

  }

}

int  main(int  argc, char  **argv)
{
  float  *L = NULL;
  float  *input_matrix = NULL;
  unsigned int  itr = 0;
  unsigned int  i = 0;
  unsigned int  j = 0;
  unsigned int  Height;
  unsigned int  Width;
  unsigned int  dim;
  struct infoRec  cmd;
  BRTALIGNED char  buffer[40];

  Setup(0);
  Setup(1);
  Setup(2);
  ParseCommandLine(argc,argv,&cmd);
  srand(cmd.Seed);
  dim = AdjustSampleSize(cmd.Width,cmd.Height);
  Height = dim;
  Width = dim;
  L = allocate_mat_f(Height,Width);
  input_matrix = allocate_mat_f(Height,Width);
  if (!L)
  {
    retval = -1;
    goto cleanup;
  }

  GenerateInputMatrix(Width,Height,input_matrix,L);
  if (cmd.Verbose)
  {
    sprintf(buffer,"Input Matrix M:\n");
    print_mat_f(buffer,"%3.2lf ",(float *) (input_matrix),Height,Width);
  }

  else
    if (!cmd.Quiet)
    {
      printf("Printing the first row, use -v for more.\n");
      sprintf(buffer,"Input Matrix M:\n");
      print_mat_f(buffer,"%3.2lf ",(float *) (L),1,Width);
    }

  {
    ::brook::stream Astream(::brook::getStreamType(( float  *)0), Height , Width,-1);
    ::brook::stream Lstream(::brook::getStreamType(( float  *)0), Height , Width,-1);
    ::brook::stream Lstream2(::brook::getStreamType(( float  *)0), Height , Width,-1);
    float  *empty = (float *) (calloc(Height * Width,sizeof(float ) ));

    Start(0);
    for (itr = 0; itr < cmd.Iterations; itr++)
    {
      streamRead(Astream,L);
      streamRead(Lstream,empty);
      streamRead(Lstream2,empty);
      for (i = 0; i < Width; i++)
      {
        cholesky(Astream,(i % 2) ? (Lstream2) : (Lstream),(i % 2) ? (Lstream) : (Lstream2),(float ) (i));
//        streamWrite((i % 2) ? (Lstream) : (Lstream2),empty);
//        print_mat_f("Intermediate L =:\n","%3.2lf ",(float *) (empty),Height,Width);
      }

      streamWrite((i % 2) ? (Lstream2) : (Lstream),L);
    }

    Stop(0);
  }

  if (cmd.Verbose)
  {
    print_mat_f("Output Matrix L = cholesky(M):\n","%3.2lf ",(float *) (L),Height,Width);
  }

  else
    if (!cmd.Quiet)
    {
      printf("Printing out first row of output L = cholesky(M), use -v for more info.\n");
      print_mat_f("","%3.2lf ",(float *) (L),1,Width);
    }

  if (cmd.Timing)
  {
    double  time = GetElapsedTime(0);
    double  gflop = 0.000000f;

    printf("%-8s%-8s%-16s%-16s\n","Width","Height","Iterations","GPU Total Time");
    printf("%-8d%-8d%-16d%-16lf",dim,dim,cmd.Iterations,time);
    printf("\n\n");
  }

  if (cmd.Verify)
  {
    int  res = 0;

    printf("-e Verify correct output.\n");
    printf("Performing Cholesky Factorization on CPU ... ");
    CholeskyCPU(Width,Height,input_matrix);
    printf("Done\n");
    if (cmd.Verbose)
      print_mat_f("CPU Output Matrix L = cholesky(M):\n","%3.2lf ",(float *) (input_matrix),Height,Width);
    res = compare_mat_f(input_matrix,(float *) (L),Height,Width);
    if (res > 0)
    {
      printf("%s: Failed!\n\n",argv[0]);
      retval = -1;
    }

    else
    {
      printf("Passed! %d wrong\n\n",res);
      retval = 0;
    }

  }

  if (cmd.Performance)
  {
    double  cpu_time = 0.000000f;
    double  gpu_time = 0.000000f;

    printf("-p Compare performance with CPU.\n");
    Start(1);
    for (i = 0; i < cmd.Iterations; ++i)
    {
      CholeskyCPU(Width,Height,input_matrix);
    }

    Stop(1);
    cpu_time = GetElapsedTime(1);
    gpu_time = GetElapsedTime(0);
    printf("%-8s%-8s%-16s%-16s%-16s%-16s\n","Width","Height","Iterations","CPU Total Time","GPU Total Time","Speedup");
    printf("%-8d%-8d%-16d%-16lf%-16lf%-16lf",cmd.Width,cmd.Height,cmd.Iterations,cpu_time,gpu_time,cpu_time / gpu_time);
    printf("\n\n");
  }

cleanup:
  if (L)
  {
    free(L);
  }

  if (input_matrix)
  {
    free(input_matrix);
  }

  if (!cmd.Verify)
  {
    printf("\nPress enter to exit...\n");
    getchar();
  }

  return retval;
}


