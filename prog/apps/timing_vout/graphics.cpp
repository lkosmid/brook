#include <stdio.h>
#include <stdlib.h>
#include <brook/brook.hpp>
int isdebug=1;
int linecount=0;
void DrawLine (char * data, float x1, float y1, float x2, float y2, int width) {
   
  int lx,gx,ly,gy,i;
  ++linecount;
  lx = (int)(x1<x2?x1:x2);
  gx = (int)(x1<x2?x2:x1);
  ly = (int)(y1<y2?y1:y2);
  gy = (int)(y1<y2?y2:y1);
  if (lx<0||ly<0||gx<0||gy<0) return;
  if (gx-lx<.25){
    for (i=ly;i<=gy;++i){      
	if (isdebug)
      data[i*width+lx]+=63;
        else
      data[i*width+lx]=127;
    }
  }else if (gy-ly<.25) {
    for (i=lx;i<=gx;++i){
      if (isdebug)
      data[ly*width+i]+=63;
      else data[ly*width+i]=127;
    }
  }else {
    printf ("unsupported draw from %f %f to %f %f\n",x1,y1,x2,y2);
  }
}
void Draw (float4 * dat, int datanum, char * pic, int width) {
  int i;
  for (i=0;i<datanum;++i) {
    float4 data=dat[i];
    if (finite_float(data.x)&&finite_float(data.y)&&finite_float(data.z)&&finite_float(data.w)) {
      DrawLine(pic,data.x,data.y,data.x,data.w,width);
      DrawLine(pic,data.x,data.y,data.z,data.y,width);
      DrawLine(pic,data.x,data.w,data.z,data.w,width);
      DrawLine(pic,data.z,data.w,data.z,data.y,width);
    }
  }
  if (isdebug)
     printf ("Line count: %d\n",linecount);
}
void Draw (float4 * dat, float4 datasize, char * pic, int width) {
   Draw(dat,(int)(datasize.x*datasize.y),pic,width);
}
