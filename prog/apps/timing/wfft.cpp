#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <brook.hpp>
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
static std::map<int, float2 *> rawWstream;
static std::map <DualInt,brook::stream *> Ws;
int myabs (int i) {
   return i>=0?i:-i;
}
brook::stream &getW(int k, int N, bool vertical) {
   const __BRTStreamType* flawt2 = brook::getStreamType((float2*)0);
   std::map<DualInt,brook::stream*>::iterator iter = Ws.find(DualInt(k,vertical?myabs(N):-myabs(N)));
   if (iter!=Ws.end()) {
      return *(*iter).second;
   }
   brook::stream *ret;
   std::map<int,float2 *>::iterator rawW = rawWstream.find(N);
   float2 * rW=0;
   if (rawW==rawWstream.end()) {
      rW = new float2 [N/2];
      for (int i=0;i<N/2;++i) {
         float theta = 2*3.1415926536*i/N;
         rW[i].x=cos(theta);
         rW[i].y=sin(theta);
         printf ("[[[%f %f]]]",rW[i].x,rW[i].y);
      }
      rawWstream[N]=rW;
   }else {
      rW = (*rawW).second;
   }
   if (vertical) {
      ret = new brook::stream (flawt2,1,N/2,-1);
   }else {
      ret = new brook::stream (flawt2,N/2,1,-1);
   }
   if (k!=0) {
      //else we stay the same
      float2 *stridedW = new float2 [N/2];
      for (int i=0;i<N/2;++i) {
         stridedW[i]=rW[i-(i%(1<<k))];
      }
      streamRead(*ret,stridedW);
      delete []stridedW;
   }else {
      streamRead(*ret,rW);
   }
   Ws[DualInt(k,vertical?myabs(N):-myabs(N))]=ret;   
   return *ret;
}
void freeWs () {
   for (std::map<DualInt,brook::stream *>::iterator i=Ws.begin();
        i!=Ws.end();
        ++i) {
      delete (*i).second;      
   }
   if (1) {
      for (std::map<int, float2 *>::iterator i=rawWstream.begin();
           i!=rawWstream.end();
           ++i) {
            delete [](*i).second;      
      }
      rawWstream.clear();
   }
}
