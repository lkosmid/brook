#include "brook.hpp"
#include "lin_time.h"
#include <stdio.h>
#include <stdlib.h>
static void floatMath (float a, float b, float4 c,
                    float d[10][10], float &e) {
   float g=a+b;
   float h=b*b;
   float i = g*h+a*b;
   float j = a>.5f?g*i/(h>.001f?h:1.0f)+h:g*i*h-h;
   float k = g*i*h+h;
   float l = g+h*g+k*i;
   float m = g+j*k-i*k/(h>.001f?h:1.0f);
   float o = k;
   o+=5;
   g+=9;
   h*=24;
   i*=1;
   o+=0+0+0+0+0+0+0+0+0+0+0+0+0+0+0+0+0;
   o+=21049;
   o+=h*i+g+i-m*(l-j)/(k>.001f?k:1.0f);
   e= g+h*i/(k>.001f?k:1.0f)+m*l*1*1*1*1*1*1*1*1*1*1*1;
}

static void  FloatMath (const __BrtFloat1  &a,
                         const __BrtFloat1  &b,
                         const __BrtFloat4  &c,
                         const __BrtArray<__BrtFloat1> &d,
                         __BrtFloat1  &e){
  __BrtFloat1  g = a + b;
  __BrtFloat1  h = b * b;
  __BrtFloat1  i = g * h + a * b;
  __BrtFloat1  j = (__BrtFloat1(0.5f) < a).questioncolon(g * i / ((__BrtFloat1(0.001f) < h).questioncolon(h,__BrtFloat1(1.0f))) + h,g * i * h - h);
  __BrtFloat1  k = g * i * h + h;
  __BrtFloat1  l = g + h * g + k * i;
  __BrtFloat1  m = g + j * k - i * k / ((__BrtFloat1(0.001f) < h).questioncolon(h,__BrtFloat1(1.0f)));
  __BrtFloat1  o = k;

  o += __BrtInt1(5);
  g += __BrtInt1(9);
  h *= __BrtInt1(24);
  i *= __BrtInt1(1);
  o += __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0) + __BrtInt1(0);
  o += __BrtInt1(21049);
  o += h * i + g + i - m * (l - j) / ((__BrtFloat1(0.001f) < k).questioncolon(k,__BrtFloat1(1.0f)));
  e = g + h * i / ((__BrtFloat1(0.001f) < k).questioncolon(k,__BrtFloat1(1.0f))) + m * l * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1) * __BrtInt1(1);
}


static void math(float a, float b, float4 c,
                    float d[10][10], float &e) {
   float4 tmp;
   float4 g;
   float f;
   {
     float4 assgn=float4(a+b,a*b,(a+b)*(b*a),a*c.x);
     tmp=assgn;
   }
   g=tmp;
   tmp.x*=c.x+tmp.x;
   tmp.y*=c.y+tmp.y;
   tmp.z*=c.z+tmp.z;
   tmp.w*=c.w+tmp.w;
   f=tmp.x+tmp.y+tmp.z+tmp.w;
   {
     float4 assgn=float4(f,f,f,f);
     tmp=assgn;
   }
   e=f+(tmp.x+g.x)+(tmp.y+g.y)+(tmp.z+g.z);
}


static void  Math(const __BrtFloat1  &a,
                   const __BrtFloat1  &b,
                   const __BrtFloat4  &c,
                   const __BrtArray<__BrtFloat1> &d,
                   __BrtFloat1  &e){
  __BrtFloat4  tmp;
  __BrtFloat4  g;
  __BrtFloat1  f;

  tmp = __BrtFloat4 (a + b,a * b,(a + b) * (b * a),a * c.swizzle1(maskX));
  g = tmp;
  tmp *= c + tmp;
  f = tmp.swizzle1(maskX) + tmp.swizzle1(maskY) + tmp.swizzle1(maskZ) + tmp.swizzle1(maskW);
  tmp = f.swizzle4(maskX, maskX, maskX, maskX);
  e = f + (tmp + g).swizzle1(maskX) + (tmp + g).swizzle1(maskY) + (tmp + g).swizzle1(maskZ);
}
 
static void gather (float a, float b, float4 c,
                    float d[10][10], float &e) {
  e = a+b+d[(int)c.y][(int)c.x]+d[(int)c.z][(int)c.y]+d[(int)c.w][(int)c.z]
     +d[(int)c.z][(int)c.x]+d[(int)c.w][(int)c.x]+d[(int)c.w][(int)c.y]
     +d[(int)c.w][(int)c.z];
}


void  Gather (const __BrtFloat1  &a,
               const __BrtFloat1  &b,
               const __BrtFloat4  &c,
               const __BrtArray<__BrtFloat1> &d,
               __BrtFloat1  &e){
  e = a + b + d[c.swizzle2(maskX, maskY)] + d[c.swizzle2(maskY, maskZ)] + d[c.swizzle2(maskZ, maskW)] + 
d[c.swizzle2(maskX, maskZ)] + d[c.swizzle2(maskX, maskW)] + d[c.swizzle2(maskY, maskW)] + d[c.swizzle2(maskZ, maskW)];
}

void cputest(float *a, float *b, float4 c, float * d, float * e, unsigned int size) {
   unsigned int total=size*size;
   float * aa=a;
   float* bb = b; 
   float (*dd)[10] = (float (*)[10])d;
   float * ee=e;
   unsigned int i;
   UpdateTime();
   for (i=0;i<total;++i) {
      gather (*aa++,*bb++,c,dd,*ee++);
   }
   UpdateTime();
   fprintf (stderr,"Native Gather took %f seconds\n",GetElapsedTime());
   aa=a;bb = b;ee=e;
   UpdateTime();
   for (i=0;i<total;++i) {
      math (*aa++,*bb++,c,dd,*ee++);
   }
   UpdateTime();
   fprintf (stderr,"Native Math took %f seconds\n",GetElapsedTime());

   aa=a;bb = b;ee=e;
   UpdateTime();
   for (i=0;i<total;++i) {
      floatMath (*aa++,*bb++,c,dd,*ee++);
   }
   aa=a;bb = b;ee=e;

   UpdateTime();
   fprintf (stderr,"Native Float Math took %f seconds\n",GetElapsedTime());


   total*=10;

   {
      __BrtFloat1 * aaa=(__BrtFloat1*)a;
      __BrtFloat1* bbb = (__BrtFloat1*)b; 
      __BrtFloat1* d_data =(__BrtFloat1*)d;
      unsigned int extents[2]={size,size};
      __BrtArray<__BrtFloat1> dd (d_data,2,4,&extents[0]);
      __BrtFloat1* eee=(__BrtFloat1*)e;
      __BrtFloat4* cc = (__BrtFloat4*)&c;
      __BrtFloat1* aa=aaa;__BrtFloat1 *bb=bbb; __BrtFloat1 *ee=eee;
   UpdateTime();
   for (i=0;i<total;++i) {
      Gather (*aa,*bb,*cc,dd,*ee);
   }
   UpdateTime();
   fprintf (stderr,"Same Memory BRT Gather took %f seconds\n",GetElapsedTime());
   aa=aaa;bb = bbb;ee=eee;
   UpdateTime();
   for (i=0;i<total;++i) {
      Math (*aa,*bb,*cc,dd,*ee);
   }
   UpdateTime();
   fprintf (stderr,"Same Memory BRT Math took %f seconds\n",GetElapsedTime());

   aa=aaa;bb = bbb;ee=eee;
   UpdateTime();
   for (i=0;i<total;++i) {
      FloatMath (*aa,*bb,*cc,dd,*ee);
   }
   UpdateTime();
   fprintf (stderr,"Same Memory Float Math took %f seconds\n",GetElapsedTime());

   }


   UpdateTime();
   for (i=0;i<total;++i) {
      gather (*aa,*bb,c,dd,*ee);
   }
   UpdateTime();
   fprintf (stderr,"Same Memory Native Gather took %f seconds\n",GetElapsedTime());
   aa=a;bb = b;ee=e;
   UpdateTime();
   for (i=0;i<total;++i) {
      math (*aa,*bb,c,dd,*ee);
   }
   UpdateTime();
   fprintf (stderr,"Same Memory Native Math took %f seconds\n",GetElapsedTime());

   aa=a;bb = b;ee=e;
   UpdateTime();
   for (i=0;i<total;++i) {
      floatMath (*aa,*bb,c,dd,*ee);
   }
   UpdateTime();
   fprintf (stderr,"Same Memory Native Float Math took %f seconds\n",GetElapsedTime());

}






