#include "cpu.hpp"
static float lerp (unsigned int i, unsigned int end,float lower,float upper) {
   float frac=end>0?((float)i)/(float)end:(float)upper;//used to be end-1 on denom
   return (1-frac)*lower+frac*upper;
}

namespace brook {
  void CPUIter::allocateStream(int dims, 
                                int extents[],
                                float ranges[]) {
     //     Stream * s = brook::RunTime::GetInstance()->
     //        CreateStream( type, dims, extents );
     float * data = (float*)malloc (stream.getTotalSize()*sizeof(float)*type);
     if (dims<2) {
        for (int i=0;i<extents[0];++i) {
           for (int j=0;j<type;++j) {
              data[i*type+j]=lerp(i,extents[0],ranges[j],ranges[j+type]);
           }
        }
     }else if (dims==2){
         //now we know dims == data type;
        int i[2]={0,0};
        for (i[0]=0;i[0]<extents[0];++i[0]) {
           for (i[1]=0;i[1]<extents[1];++i[1]) {
              for (unsigned int k=0;k<2;++k) {
                 float f= lerp (i[k],extents[k],ranges[k],ranges[2+k]);
                 data[(i[0]*extents[1]+i[1])*2+k]=f;
              }
           }
        }
     }else {
        assert(0);
     }
     streamRead(&stream,data);
     free(data);
       //XXX daniel this needs to be done
       //will use standard brook BRTCreateStream syntax and then copy data in
       //dx9 can then call this to easily fallback if cpu is necessary
  }

}
