#include <vector>
#include <float.h>
using std::vector;
class float4 {
 public:
  float x;float y; float z; float w;
  float4(float a, float b, float c, float d) {x=a;y=b;z=c;w=d;}
  float4() {}
};

float curve (float x, float y) {
  float inputx= 512.0f+x;
  float inputy = 512.0f+y
  return inputx*inputx+inputy*inputy*inputy*inputy*inputx;
}
float evaluateError(float input, float actual) {
  return fabs((input-actual)/(actual+1));
}
void Tessellate (const vector<float4> &inputv, float toler, vector<float4> &outputv) {
  for (vector<float4>::const_iterator input=inputv.begin();input!=inputv.end();++input) {
    float heightx, heighty,centerx,centery,centerz;
    heightx = curve(intput->x,intput->y);
    heighty = curve(intput->z,intput->w);
    centerx = .5*intput->x+.5*intput->z;
    centery = .5*intput->y+.5*intput->w;
    centerz = .5*heightx+.5*heighty;
    float fheight=curve(centerx,centery);
    float error=evaluateError(centerz,fheight);
    if (error>=toler) {
      float4 myoutput(intput->x,intput->y,centerx,centery);
      outputv.push_back(myoutput);
      myoutput.x=intput->z;myoutput.y=intput->w;      
      outputv.push_back(myoutput);
      myoutput.x=intput->x;myoutput.y=intput->w;      
      outputv.push_back(myoutput);
      myoutput.x=intput->z;myoutput.y=intput->y;      
      outputv.push_back(myoutput);
    }else {
      outputv.push_back(inputv.back());
    }
    
  }
}
