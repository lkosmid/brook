#include "handvector.hpp"
#include "lin_time.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct {
  float x,y,z,w;
} float4;

static void  FloatMath (const Float1  &a,
                         const Float1  &b,
                         const float4  &c,
                         const Float1 &d,
                         Float1  &e){
  Float1  g = a + b;
  Float1  h = b * b;
  Float1  i = g * h + a * b;
  Float1  j = (Float1(0.5f) < a).questioncolon(g * i / ((Float1(0.001f) < h).questioncolon(h,Float1(1))) + h,g * i * h - h);
  Float1  k = g * i * h + h;
  Float1  l = g + h * g + k * i;
  Float1  m = g + j * k - i * k / ((Float1(0.001f) < h).questioncolon(h,Float1(1.0f)));
  Float1  o = k;

  o += Float1(5);
  g += Float1(9);
  h *= Float1(24);
  i *= Float1(1);
  o += Float1(0+0+0+0 +0 +0 +0 +0 +0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0);
  o += Float1(21049);
  o += h * i + g + i - m * (l - j) / ((Float1(0.001f) < k).questioncolon(k,Float1(1.0f)));
  e = g + h * i / ((Float1(0.001f) < k).questioncolon(k,Float1(1.0f))) + m * l * Float1(1*1*1 *1 * 1 * 1 * 1 * 1 * 1 * 1 * 1);
}



void handcputest(float *a, float *b, float4 c, float * d, float * e, unsigned int size) {
   unsigned int total=size*size;
   total*=10;
   {
     unsigned int i;
      Float1 * aaa=(Float1*)a;
      Float1* bbb = (Float1*)b; 
      Float1* ddd =(Float1*)d;
      Float1* eee=(Float1*)e;
      float4* cc = &c;
      Float1* dd= ddd;
      Float1* aa=aaa;Float1 *bb=bbb; Float1 *ee=eee;
   UpdateTime();
   for (i=0;i<total;++i) {
      FloatMath (*aa,*bb,*cc,*dd,*ee);
   }
   UpdateTime();
   fprintf (stderr,"Same Memory Hand Coded Float Math took %lf seconds\n",
            GetElapsedTime());

   }

}






