#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <brook/brook.hpp>
#include <map>
#include <stdio.h>
class DualInt {
public:
   int k;
   int N;
   DualInt (int k, int N) {
      this->k=k;
      this->N=N;
   }
   bool operator < (const DualInt &other) const{
      if (N<other.N)
         return true;
      if (N>other.N)
         return false;
      return k<other.k;
   }
};
static int TwoPowerX(int nNumber) {
  // Achieve the multiplication by left-shifting 
  return (1<<nNumber);
}

static int BitReverse(int nNumberToReverse, int nNumberOfBits) {
  int nBitIndex;
  int nReversedNumber = 0;
  for (nBitIndex = nNumberOfBits-1; nBitIndex >= 0; --nBitIndex) {
    if ((1 == nNumberToReverse >> nBitIndex)) {         
      nReversedNumber  += TwoPowerX(nNumberOfBits-1-nBitIndex);    
      nNumberToReverse -= TwoPowerX(nBitIndex);                      
    }
  }
  return(nReversedNumber);
}

static std::map <DualInt,brook::stream *> Ws;
int myabs (int i) {
   return i>=0?i:-i;
}
brook::stream &getW(int k, int logN, bool vertical) {
  int N = (1<<logN);
  int Stride=(N/2)/(1<<k);
  const brook::StreamType* flawt2 = brook::getStreamType((float2*)NULL);
  std::map<DualInt,brook::stream*>::iterator iter = Ws.find(DualInt(k,vertical?myabs(N):-myabs(N)));
  if (iter!=Ws.end()) {
    return *(*iter).second;
  }
  brook::stream *ret;
  if (vertical) {
    ret = new brook::stream (flawt2,1,N/2,-1);
  }else {
    ret = new brook::stream (flawt2,N/2,1,-1);
  }
  float2 *W = new float2 [N/2];
  //printf ("W logn:%d k:%d N:%d Stride:%d:\n",logN,k,N,Stride);
  for (int i=0;i<N/2;++i) {
    int aindex=i*2;
    int bindex=BitReverse(aindex,logN);
    int index=(bindex-(bindex%Stride));
    float ang=index*3.1415926536f/(N/2);
    W[i].x=cos(ang);
    W[i].y=sin(ang);
    //printf ("index: %d i:%d b:%d a:(%f) %f %f] ",index,aindex,bindex,ang*180/3.1415926536,W[i].x,W[i].y);
  }
  printf ("\n");
  streamRead(*ret,W);
  delete []W;
  Ws[DualInt(k,vertical?myabs(N):-myabs(N))]=ret;   
  return *ret;
}
void freeWs () {
   for (std::map<DualInt,brook::stream *>::iterator i=Ws.begin();
        i!=Ws.end();
        ++i) {
      delete (*i).second;      
   }
}
