#include <vector>
#include "rapcol.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
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
   fprintf(stderr, "filename: %s\n", filename);
   if (strcmp(filename,"dabunny")==0) 
     LoadPly("bunny.ply",tList);
   else
     LoadPly(filename,tList);
   *tri = (Tri*)malloc(sizeof(Tri)*tList.size());
   fprintf(stderr, "Return tri length = %d\n", tList.size());
   *neigh =(Neighbor*)malloc(sizeof(Neighbor)*tList.size());
   for (unsigned int i=0;i<tList.size();++i) {
      unsigned int j;
      float4 zero4(0,0,0,0);
      Tri t = tList[i];
      t.A.w=0;
      t.B.w=0;
      t.C.w=0;
      Neighbor n;
      memset(&n,0,sizeof(Neighbor));
      if (strcmp(filename,"dabunny")) {
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
      if (n.ABB==n.BBC)
        n.BBC=zero4;
      if (n.BCC==n.ACC)
        n.ACC=zero4;
      if (n.AAB==n.AAC)
        n.AAB=zero4;
      if (n.AB==n.ABB)
        n.ABB=zero4;
      if (n.AAB==n.AB)
        n.AAB=zero4;
      if (n.AAC==n.AC)
        n.AAC=zero4;
      if (n.ACC==n.AC)
        n.ACC=zero4;
      if (n.BBC==n.BC)
        n.BBC=zero4;
      if (n.BCC==n.BC)
        n.BBC=zero4;
      (*tri)[i]=t;
      (*neigh)[i]=n;
   }
   return tList.size();
}

int myLog (int l) {
  if (l==0)
    return 0;
  int i;
  for (i=0;i<32;++i) {
    if ((1<<i)>=l)
      break;
  }
  return i;
}
float dawt (float3 a, float3 b) {
  return a.x*b.x+a.y*b.y+a.z*b.z;
}
extern void computeFunctionCallPattern(float epsilon,
                                       int argc, 
                                       char ** argv, 
                                       int numTri,
                                       Tri*triangles,
                                       Neighbor *neigh){
  std::string filename = "sum-subdiv";
  int i;
  for (i=1;i<argc;++i) {
    if (argv[i][0]>='0'&&argv[i][0]<='9') {
      filename+="-";
    }
    filename+=argv[i];
  }
  char bleh[128];
  sprintf(bleh,"-eps%f",epsilon);
  filename+=bleh;
  int EstablishGuess=0;
  int UpdateGuess = 0;
  int RelativeGather=0;
  int produceTriP=0;
  int splitTriangles=0;
  int writeFinalTriangles=0;
  vector <Tri> trivec;
  for (i=0;i<numTri;++i) {
    trivec.push_back(triangles[i]);
  }
  do {
    produceTriP+=trivec.size();
    EstablishGuess+=2*trivec.size();
    UpdateGuess+=2*(myLog(trivec.size())-1)*trivec.size();
    vector <Tri> split;
    vector <Tri> nosplit;
    //do vout stage on CPU;
    for (unsigned int j=0;j<trivec.size();++j) {
      Tri t = trivec[j];
      float3 ab(t.A.x-t.B.x,t.A.y-t.B.y,t.A.z-t.B.z);
      float3 ac(t.A.x-t.C.x,t.A.y-t.C.y,t.A.z-t.C.z);
      float3 bc(t.B.x-t.C.x,t.B.y-t.C.y,t.B.z-t.C.z);
      if (dawt(ab,ab)<epsilon&&
          dawt(ac,ac)<epsilon&&
          dawt(bc,bc)<epsilon) {
        nosplit.push_back(t);
      }else {
        float4 a2b(.5*(t.A.x+t.B.x),
                   .5*(t.A.y+t.B.y),
                   .5*(t.A.z+t.B.z),0);
                   
        float4 a2c(.5*(t.A.x+t.C.x),
                   .5*(t.A.y+t.C.y),
                   .5*(t.A.z+t.C.z),0);
        float4 b2c(.5*(t.B.x+t.C.x),
                   .5*(t.B.y+t.C.y),
                   .5*(t.B.z+t.C.z),0);
        Tri u;
        u.A=t.A;
        u.B=a2b;
        u.C=a2c;
        split.push_back(u);
        u.A=a2b;
        u.B=t.B;
        u.C=b2c;
        split.push_back(u);
        u.A=a2c;
        u.B=b2c;
        u.C=t.C;
        split.push_back(u);
        u.A=a2c;
        u.B=a2b;
        u.C=b2c;
        split.push_back(u);
      }
    }
    RelativeGather+=split.size()/4;
    splitTriangles+=split.size()/4;
    trivec.swap(split);
    RelativeGather+=nosplit.size();
    writeFinalTriangles+=nosplit.size();
  } while (trivec.size());
  FILE * fp = fopen (filename.c_str(),"w");
  fprintf (fp,"computeNeighbors %d\n",splitTriangles);  
  fprintf (fp,"EstablishGuess %d\n",EstablishGuess);
  fprintf (fp,"linearReorgSplitTriangles %d\n",4*splitTriangles);
  fprintf (fp,"NanToBoolRight %d\n",EstablishGuess);
  fprintf (fp,"NanToRight %d\n",UpdateGuess);
  fprintf (fp,"produceTriP %d\n",produceTriP);
  fprintf (fp,"splitTriangles %d\n",splitTriangles);
  fprintf (fp,"RelativeGather %d\n",RelativeGather);
  fprintf (fp,"UpdateGuess %d\n",UpdateGuess);
  fprintf (fp,"writeFinalTriangles %d\n",writeFinalTriangles*3);
  fclose(fp);
}
