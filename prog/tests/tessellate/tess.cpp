#include <iostream>
#include <vector>
#include <float.h>
#define TIMING_DEF
#include "timing.h"
#include <brook.hpp>
#include "graphics.h"
#include <stdio.h>
using std::vector;
using namespace std;//vc6 bug
/*
class float4 {
 public:
  float x;float y; float z; float w;
  float4(float a, float b, float c, float d) {x=a;y=b;z=c;w=d;}
  float4() {}
};
*/
float curve (float x, float y) {
  float inputx= 512.0f+x;
  float inputy = 512.0f+y;
  return inputx*inputx+inputy*inputy*inputy*inputy*inputx;
}
float evaluateError(float input, float actual) {
  return fabs((input-actual)/(actual+1));
}
void Tessellate (const vector<float4> &inputv, float toler, vector<float4> &outputv) {
  for (vector<float4>::const_iterator input=inputv.begin();input!=inputv.end();++input) {
    float heightx, heighty,centerx,centery,centerz;
    heightx = curve(input->x,input->y);
    heighty = curve(input->z,input->w);
    centerx = .5f*input->x+.5f*input->z;
    centery = .5f*input->y+.5f*input->w;
    centerz = .5f*heightx+.5f*heighty;
    float fheight=curve(centerx,centery);
    float error=evaluateError(centerz,fheight);
    if (error>=toler) {
      float4 myoutput(input->x,input->y,centerx,centery);
      outputv.push_back(myoutput);
      myoutput.x=input->z;myoutput.y=input->w;      
      outputv.push_back(myoutput);
      myoutput.x=input->x;myoutput.y=input->w;      
      outputv.push_back(myoutput);
      myoutput.x=input->z;myoutput.y=input->y;      
      outputv.push_back(myoutput);
    }else {
      outputv.push_back(inputv.back());
    }
    
  }
}
static float tofd (double i) {
	return (float)i;
}

#define STRIDE 1024
#define SIZE 1024
int haupt (int argc, char ** argv) {
/*
vector <float> a(1024);
vector <float> b;
start = GetTimeMillis();
for (unsigned int j=0;j<1024;++j) {
for (unsigned int i=0;i<1024;++i) {
  if (i%2==0)b.push_back(2.0f);
  else b.push_back(1.0f);
  b.push_back(a[i]);

}
b.clear();

}
stop=GetTimeMillis();
  printf("Total Time %f\n",(stop-start)/(1024*1000.));
return 0;
*/
{
  float toler = argc>1?tofd(atof(argv[1])):tofd(.1);
  int limit = argc>2?atoi(argv[2]):32;
  float lastsize;
  vector <float4> polys;
  polys.push_back(float4(0,0,SIZE/2,SIZE/2));
  polys.push_back(float4(0,SIZE/2,SIZE/2,SIZE));
  polys.push_back(float4(SIZE/2,0,SIZE,SIZE/2));
  polys.push_back(float4(SIZE/2,SIZE/2,SIZE,SIZE));
  int ite;
  SetupMillisTimer();
  start = GetTimeMillis();
  ite=0;
  do {
    int lastsizeint;ite++;
    lastsize=(float)polys.size();
    lastsizeint=polys.size();
    {      
      vector<float4> nextPolys;
      Tessellate(polys,toler,nextPolys);
      polys.swap(nextPolys);      
    }
  }while (polys.size()/STRIDE<(unsigned int)limit
          &&polys.size()!=(unsigned int)lastsize);
  {
    float4 * finaldata=&polys[0];
    char * data=(char*)malloc((SIZE+1)*(SIZE+1)*sizeof(char));
    memset(data,0,sizeof(char)*(1+SIZE)*(1+SIZE));
    stop = GetTimeMillis();

    Draw(finaldata,polys.size(),data,SIZE+1);
    writePng("polys.ppm",data,SIZE+1,SIZE+1);
    free (data);   
  }
  printf ("%f ",(float)(stop-start));
  return 0;
}
}