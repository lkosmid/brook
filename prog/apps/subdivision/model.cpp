#include <vector>
#include "rapcol.h"

using std::vector;
void LoadPly (const char * file,vector<Tri> &ret);
bool operator ==(const float4 &a, const float4 &b) {
   return a.x==b.x&&a.y==b.y&&a.z==b.z;
}
void checkEdgeNeighbor(const float4 &a, const float4 &b, float4 &c,const Tri &t){ 
   if ((t.A==a&&t.B==b)||(t.A==b&&t.B==a))
      c = t.C;
   if ((t.A==a&&t.C==b)||(t.A==b&&t.C==a))
      c = t.B;
   if ((t.B==a&&t.C==b)||(t.B==b&&t.C==a))
      c = t.A;
}
unsigned int loadModelData(const char * filename,
                           Tri ** tri,
                           Neighbor ** neigh) {
   std::vector<Tri>tList;
   LoadPly(filename,tList);
   
   for (unsigned int i=0;i<tList.size();++i) {
      unsigned int j;
      Tri t = tList[i];
      t.A.w=0;
      t.B.w=0;
      t.C.w=0;
      Neighbor n;
      for (j=0;j<tList.size();++j) {
         //check for AB, BC, AC
         Tri nei=tList[j];
         checkEdgeNeighbor(t.A,t.B,n.AB,nei);
         checkEdgeNeighbor(t.A,t.C,n.AC,nei);
         checkEdgeNeighbor(t.B,t.C,n.BC,nei);         
      }
      for (j=0;j<tList.size();++j) {
         Tri nei=tList[j];
         checkEdgeNeighbor(t.A,n.AC,n.AAC,nei);
         checkEdgeNeighbor(t.A,n.AB,n.AAB,nei);
         checkEdgeNeighbor(t.B,n.BC,n.BBC,nei);
         checkEdgeNeighbor(t.B,n.AB,n.ABB,nei);
         checkEdgeNeighbor(t.C,n.BC,n.BCC,nei);
         checkEdgeNeighbor(t.C,n.AC,n.ACC,nei);
      }
   }
   return 0;
}


