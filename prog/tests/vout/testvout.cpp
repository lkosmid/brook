
////////////////////////////////////////////
// Generated by BRCC v0.1
// BRCC Compiled on: Jan  4 2004 11:13:15
////////////////////////////////////////////

#include <brook.hpp>
#include <stdio.h>


static const char __foo_ps20[] = {
"    ps_2_0\n"
"    def c0, -1, 0, 0, 0\n"
"    dcl t0.xy\n"
"    dcl t1.xy\n"
"    dcl_2d s0\n"
"    dcl_2d s1\n"
"    dcl_2d s2\n"
"    mov r5.xy, c3\n"
"    mul r0.xy, c2, r5\n"
"    add r7.x, r0.x, c3.z\n"
"    add r7.y, r0.y, c3.w\n"
"    texld r2, t0, s0\n"
"    texld r9, t1, s1\n"
"    texld r4, r7, s2\n"
"    add r4.w, r2.x, r9.x\n"
"    add r11.w, r4.x, r4.w\n"
"    mov r5.w, c4.x\n"
"    add r6.w, c0.x, r5.w\n"
"    add r8.w, -r6.w, c0.x\n"
"    abs r10.w, r8.w\n"
"    cmp r0, -r10.w, r11.w, c4.y\n"
"    mov oC0, r0\n"
"\n"
" \n"
"//!!BRCC\n"
"//narg:6\n"
"//si:1:a\n"
"//s:1:b\n"
"//c:4:c\n"
"//c:1:d\n"
"//o:1:__e_stream\n"
"//c:2:__vout_counter\n"
"//workspace:1024\n"
""};


static const char __foo_fp30[] = {
"!!FP1.0\n"
"# NV_fragment_program generated by NVIDIA Cg compiler\n"
"# cgc version 1.1.0003, build date Jul  7 2003  11:55:19\n"
"# command line args: -profile fp30 -DUSERECT -quiet\n"
"#vendor NVIDIA Corporation\n"
"#version 1.0.02\n"
"#profile fp30\n"
"#program main\n"
"#semantic main._workspace : C0\n"
"#semantic main._tex_a : TEXUNIT0\n"
"#semantic main._const_a_invscalebias : C1\n"
"#semantic main._tex_b : TEXUNIT1\n"
"#semantic main.c : C2\n"
"#semantic main.d : TEXUNIT2\n"
"#semantic main._const_d_scalebias : C3\n"
"#semantic main.__vout_counter : C4\n"
"#var float4 _workspace : C0 :  : 0 : 1\n"
"#var samplerRECT _tex_a : TEXUNIT0 : texunit 0 : 1 : 1\n"
"#var float4 _const_a_invscalebias : C1 :  : 2 : 1\n"
"#var float2 _tex_a_pos : $vin.TEXCOORD0 : TEXCOORD0 : 3 : 1\n"
"#var samplerRECT _tex_b : TEXUNIT1 : texunit 1 : 4 : 1\n"
"#var float2 _tex_b_pos : $vin.TEXCOORD1 : TEXCOORD1 : 5 : 1\n"
"#var float4 c : C2 :  : 6 : 1\n"
"#var samplerRECT d : TEXUNIT2 : texunit 2 : 7 : 1\n"
"#var float4 _const_d_scalebias : C3 :  : 8 : 1\n"
"#var float2 __vout_counter : C4 :  : 9 : 1\n"
"#var float4 main : $vout.COLOR0 : COLOR0 : -1 : 1\n"
"DECLARE _workspace;\n"
"DECLARE _const_a_invscalebias;\n"
"DECLARE c;\n"
"DECLARE _const_d_scalebias;\n"
"DECLARE __vout_counter;\n"
"TEX R0.x, f[TEX0].xyxx, TEX0, RECT;\n"
"TEX R1.x, f[TEX1].xyxx, TEX1, RECT;\n"
"ADDR R0.x, R0.x, R1.x;\n"
"MOVR R1.xy, f[TEX0].xyxx;\n"
"SLTR H0.x, R1.x, {0}.x;\n"
"SLTR H0.y, R1.y, {0}.x;\n"
"MULR R0.y, R1.x, {0.5}.x;\n"
"FRCR R0.y, |R0.y|;\n"
"MULR R0.y, R0.y, {2}.x;\n"
"MULR R0.z, R1.y, {0.33333334}.x;\n"
"FRCR R0.z, |R0.z|;\n"
"MULR R0.z, R0.z, {3}.x;\n"
"MOVR R0.w, R0.y;\n"
"MOVXC HC.x, H0.x;\n"
"MOVR R0.w(GT.x), -R0.y;\n"
"SEQR H0.x, R0.w, {0}.x;\n"
"MOVR R0.y, {1}.x;\n"
"ADDR R0.y, __vout_counter.x, -R0.y;\n"
"MOVR R1.y, __vout_counter.y;\n"
"MOVR R1.x, R0.y;\n"
"SEQR H0.z, {-1}.x, R1.x;\n"
"MOVR R2.xy, R1.xyxx;\n"
"ADDR R0.y, R1.x, -{1}.x;\n"
"MOVXC HC.x, H0.x;\n"
"MOVR R2.x(GT.x), R0.y;\n"
"SEQR H0.w, {-1}.x, R2.x;\n"
"MULX H0.w, H0.x, H0.w;\n"
"MOVR R0.y, R0.z;\n"
"MOVXC HC.x, H0.y;\n"
"MOVR R0.y(GT.x), -R0.z;\n"
"SEQR H0.x, R0.y, {0}.x;\n"
"MOVR R1.xy, R2.xyxx;\n"
"ADDR R0.y, R2.x, -{1}.x;\n"
"MOVXC HC.x, H0.x;\n"
"MOVR R1.x(GT.x), R0.y;\n"
"SEQR H0.y, {-1}.x, R1.x;\n"
"MULX H0.y, H0.x, H0.y;\n"
"TEX R2.x, c.xyxx, TEX2, RECT;\n"
"ADDR R0.x, R0.x, R2.x;\n"
"MOVR R0.y, __vout_counter.y;\n"
"MOVXC HC.x, H0.z;\n"
"MOVR R0.y(GT.x), R0.x;\n"
"MOVXC HC.x, H0.w;\n"
"MOVR R0.y(GT.x), {2}.x;\n"
"MOVXC HC.x, H0.y;\n"
"MOVR R0.y(GT.x), {1}.x;\n"
"MOVR R0.x, R1.xyxx;\n"
"ADDR R0.z, R1.x, -{1}.x;\n"
"MOVXC HC.x, H0.x;\n"
"MOVR R0.x(GT.x), R0.z;\n"
"SEQR H0.y, {-1}.x, R0.x;\n"
"MULX H0.y, H0.x, H0.y;\n"
"MOVXC HC.x, H0.y;\n"
"MOVR R0.y(GT.x), {-1}.x;\n"
"MOVR o[COLR].x, R0.y;\n"
"MOVR o[COLR].y, R0.y;\n"
"MOVR o[COLR].z, R0.y;\n"
"MOVR o[COLR].w, R0.y;\n"
"END \n"
"##!!BRCC\n"
"##narg:6\n"
"##si:1:a\n"
"##s:1:b\n"
"##c:4:c\n"
"##c:1:d\n"
"##o:1:__e_stream\n"
"##c:2:__vout_counter\n"
"##workspace:1024\n"
""};

void  __foo_cpu_inner (const __BrtFloat1  &a,
                       const __BrtFloat1  &b,
                       const __BrtFloat4  &c,
                       const __BrtArray<__BrtFloat1  , 2  , false> &d,
                       __BrtFloat1  &__e_stream,
                       __BrtFloat2  __vout_counter,
                       const __BrtFloat4 &__indexof_a){
  __e_stream = __vout_counter.swizzle1(maskY);
  {
    __BrtFloat1  e;

    e = a + b + d[c.swizzle2(maskX, maskY)];
    if (__BrtFloat1(-1.000000f) == __vout_counter.mask1(__vout_counter.swizzle1(maskX) - __BrtFloat1((float)1),maskX))
    {
      __e_stream = e;
    }
    if (__fmod_cpu_inner((__indexof_a).swizzle1(maskX),__BrtFloat1((float)2)) == __BrtFloat1((float)0))
    {
      e = __BrtFloat1((float)2);
      if (__BrtFloat1(-1.000000f) == __vout_counter.mask1(__vout_counter.swizzle1(maskX) - __BrtFloat1((float)1),maskX))
      {
        __e_stream = e;
      }

    }

    if (__fmod_cpu_inner((__indexof_a).swizzle1(maskY),__BrtFloat1((float)3)) == __BrtFloat1((float)0))
    {
      e = __BrtFloat1((float)1);
      if (__BrtFloat1(-1.000000f) == __vout_counter.mask1(__vout_counter.swizzle1(maskX) - __BrtFloat1((float)1),maskX))
      {
        __e_stream = e;
      }

      e = -__BrtFloat1((float)1);
      if (__BrtFloat1(-1.000000f) == __vout_counter.mask1(__vout_counter.swizzle1(maskX) - __BrtFloat1((float)1),maskX))
      {
        __e_stream = e;
      }

    }

  }

}
void  __foo_cpu (const std::vector<void *>&args,
                 const std::vector<const unsigned int *>&extents,
                 const std::vector<unsigned int>&dims,
                 unsigned int mapbegin, 
                 unsigned int mapextent) {
  __BrtFloat1 *arg0 = (__BrtFloat1 *)args[0];
  __BrtFloat1 *arg1 = (__BrtFloat1 *)args[1];
  __BrtFloat4 *arg2 = (__BrtFloat4 *)args[2];
  __BrtArray<__BrtFloat1  , 2  , false> arg3(
      (__BrtFloat1  *)args[3], extents[3]);
  __BrtFloat1 *arg4 = (__BrtFloat1 *)args[4];
  __BrtFloat2 *arg5 = (__BrtFloat2 *)args[5];
  unsigned int dim=dims[4];
  unsigned int newline=extents[4][dim-1];
  unsigned int ratio0 = extents[4][dim-1]/extents[0][dim-1];
  unsigned int scale0=extents[0][dim-1]/extents[4][dim-1];
  if (scale0<1) scale0 = 1;
  unsigned int ratioiter0 = 0;
  if (ratio0) ratioiter0 = mapbegin%ratio0;
  unsigned int iter0 = getIndexOf(mapbegin,extents[0], dim, extents[4]);
  unsigned int ratio1 = extents[4][dim-1]/extents[1][dim-1];
  unsigned int scale1=extents[1][dim-1]/extents[4][dim-1];
  if (scale1<1) scale1 = 1;
  unsigned int ratioiter1 = 0;
  if (ratio1) ratioiter1 = mapbegin%ratio1;
  unsigned int iter1 = getIndexOf(mapbegin,extents[1], dim, extents[4]);
  arg4+=mapbegin;
  ;
__BrtFloat4 indexof0 = computeIndexOf(mapbegin, dims[0], extents[0]);
  unsigned int i=0; 
  while (i<mapextent) {
    __foo_cpu_inner (
      *(arg0 + iter0),
      *(arg1 + iter1),
      *arg2,
      arg3,
      *arg4,
      *arg5,
      indexof0);
    i++;
    incrementIndexOf (indexof0, dims[0], extents[0]);
    if (++ratioiter0>=ratio0){
      ratioiter0=0;
      iter0+=scale0;
    }
    if (++ratioiter1>=ratio1){
      ratioiter1=0;
      iter1+=scale1;
    }
    ++arg4;
    if ((mapbegin+i)%newline==0) {
      iter0=getIndexOf(i+mapbegin,extents[0],dim, extents[4]);
      iter1=getIndexOf(i+mapbegin,extents[1],dim, extents[4]);
    }
  }
}
extern int finiteValueProduced (struct __BRTStream * input);
extern float shiftValues(struct __BRTStream *list_stream,
                         struct __BRTStream *output_stream,
                         int WIDTH, 
                         int LENGTH, 
                         int sign);
void combineStreams (struct __BRTStream **streams,
                     unsigned int num,
                     unsigned int width, 
                     unsigned int length,
                     struct __BRTStream * output) ;
void  foo (const __BRTStream& a,
		const __BRTStream& b,
		const float4  c,
		const __BRTStream& d,
		__BRTStream& __e_stream) {
  float2 __vout_counter(0.0f, 1.0f / (float)floor (.5));
  static const void *__foo_fp[] = {
     "fp30", __foo_fp30,
     "ps20", __foo_ps20,
     "cpu", (void *) __foo_cpu,
     NULL, NULL };
  static __BRTKernel k(__foo_fp);
  std::vector<__BRTStream*> __e_outputs;//get max dimension
  assert (a->getDimension()==2);
  int maxextents[2]={0,0};
  maxDimension(maxextents,a->getExtents(),a->getDimension());
  maxDimension(maxextents,b->getExtents(),b->getDimension());
  
  do {
     __e_outputs.push_back(new __BRTStream(maxextents,
                                           a->getDimension(),
                                           __e_stream->getStreamType()));
    k->PushStream(a);
    k->PushStream(b);
    k->PushConstant(c);
    k->PushGatherStream(d);
    k->PushOutput(*__e_outputs.back());
    k->PushConstant(__vout_counter);
    k->Map();
    __vout_counter.x+=1.0f;
  }while (finiteValueProduced(__e_outputs.back()));
  __BRTStream temp (__e_stream->getStreamType(),1,1,-1);

  combineStreams(&__e_outputs[0],
                 __e_outputs.size()-1,
                 maxextents[0],
                 maxextents[1],
                 &temp);
  float out = shiftValues(&temp,
                          &__e_stream,
                          temp->getExtents()[0],
                          temp->getExtents()[1],
                          -1);
}


int  main()
{
  __BRTStream a(__BRTFLOAT,10 , 10,-1);
  __BRTStream b(__BRTFLOAT,10 , 10,-1);
  __BRTStream d(__BRTFLOAT,10 , 10,-1);
  __BRTStream e(__BRTFLOAT,10 , 10,-1);
  float  data_a[10][10];
  float  data_b[10][10];
  float4  c = float4 (1.000000f,0.000000f,3.200000f,5.000000f);
  float  data_d[10][10];
  float  d_broken = 0.000000f;
  int  i;
  int  j;

  for (i = 0; i < 10; i++)
    for (j = 0; j < 10; j++)
    {
      data_a[i][j] = (float ) (i) + (float ) (j) / 10.000000f;
      data_b[i][j] = (float ) (j) + (float ) (i) / 10.000000f;
      data_d[i][j] = (float ) (i) / 100.000000f;
    }

  streamRead(a,data_a);
  streamRead(b,data_b);
  streamRead(d,data_d);
  foo(a,b,c,d,e);
  streamPrint(e);
  return 0;
}


