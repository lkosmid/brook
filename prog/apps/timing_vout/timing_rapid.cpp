#include "rapcol.h"
#include <brook.hpp>
#include <stdio.h>
typedef struct traverser_t {
  float4 index;//.xy is index into the aTree  .zw is index into bTree
  float3 Translation; 
  float3 Rotationx;
  float3 Rotationy;
  float3 Rotationz;
}Traverser;
typedef struct transposedbbox_t{
  float4 transp_rotx;
  float3 transp_roty;
  // float3 mRotationz  // since Rotationx and Rotationy are orthogonal
  /// cross(Rotationx,Rotationy);
  float3 Translation;
  float4 Radius; // if it's a leaf Radius.w is 1 else Radius.w = 0

  // if leaf, the Children.xy is an index to the Triangle
  // if node, the Children.xy is an index to left child
  // assert right.xy is always left + {1,0} this may require gaps in the tree
  float2 Children;  
}TransposedBBox;
void copy4( brook::stream in, brook::stream out);
void Aggregate4(brook::stream fourfloat, brook::stream in, brook::stream out);
void Aggregate34(brook::stream threefloat,brook::stream fourfloat, brook::stream in, brook::stream out);
void Aggregate3(brook::stream threefloat, brook::stream in, brook::stream out);
void Aggregate2(brook::stream twofloat, brook::stream in, brook::stream out);
void Aggregate1(brook::stream onefloat, brook::stream in, brook::stream out);
void  CollideNoCompact (::brook::stream currentNode,
                        ::brook::stream b1Tree,
                        ::brook::stream b2Tree,
                        ::brook::stream nextNode,
                        ::brook::stream collisions,
                        ::brook::stream sentinel);
void  CheckTriangleCollideNoCompact (const float3  mRx,
                                     const float3  mRy,
                                     const float3  mRz,
                                     const float3  mT,
                                     ::brook::stream collisions,
                                     ::brook::stream t1List,
                                     ::brook::stream t2List,
                                     ::brook::stream hits,
                                     ::brook::stream sentinel);
void  updateCurrentNode (const float  stretchX,
                         ::brook::stream curNodeIndices,
                         ::brook::stream aTree,
                         ::brook::stream bTree,
                         ::brook::stream nodes,
                         ::brook::stream nextNode);
void  CheckTriangleCollide (const float3  mRx,
                            const float3  mRy,
                            const float3  mRz,
                            const float3  mT,
                            ::brook::stream collisions,
                            ::brook::stream t1List,
                            ::brook::stream t2List,
                            ::brook::stream &__hits_stream);

void  Collide (::brook::stream currentNode,
		::brook::stream b1Tree,
		::brook::stream b2Tree,
		::brook::stream &__nextNode_stream,
               ::brook::stream &__collisions_stream);
void  copyTriangle(unsigned int  howMany, ::brook::stream t1A, ::brook::stream t1B, ::brook::stream t1C, Tri  *tri)
{
  unsigned int  i;
  float3  *temp;

  temp = (float3 *) (malloc(sizeof(float3 )  * howMany));
  for (i = 0; i < howMany; ++i)
  {
    temp[i] = tri[i].A;
  }

  streamRead(t1A,temp);
  for (i = 0; i < howMany; ++i)
  {
    temp[i] = tri[i].B;
  }

  streamRead(t1B,temp);
  for (i = 0; i < howMany; ++i)
  {
    temp[i] = tri[i].C;
  }

  streamRead(t1C,temp);
  free(temp);
}

void  TransposeBBoxes(unsigned int  howmany, BBox  *bb)
{
  unsigned int  i;

  for (i = 0; i < howmany; ++i)
  {
    float4  x = bb[i].Rotationx;
    float3  y = bb[i].Rotationy;
    float3  z;

    z = float3 (x.w * (x.y * y.z - x.z * y.y),x.w * (x.z * y.x - x.x * y.z),x.w * (x.x * y.y - x.y * y.x));
    bb[i].Rotationx = float4 (x.x,y.x,z.x,x.w);
    bb[i].Rotationy = float3 (x.y,y.y,z.y);
  }

}

void  Broken()
{
  printf("Breaking\n");
}
extern bool debug_rapid;
extern bool forcevanilla;
extern int  checkPassCorrectness(Traverser *, int , int );
unsigned int  doCollide(unsigned int  widt1, unsigned int  heit1, Tri  *t1, unsigned int  bboxwidt1, unsigned int  bboxheit1, BBox  *bboxest1, unsigned int  widt2, unsigned int  heit2, Tri  *t2, unsigned int  bboxwidt2, unsigned int  bboxheit2, BBox  *bboxest2, float3  rX, float3  rY, float3  rZ, float3  trans, float3  csRapidColliderrX, float3  csRapidColliderrY, float3  csRapidColliderrZ, float3  csRapidColliderT, float4  **intersections)
{
  forcevanilla=false;// this makes it so that our vout stuff is cached;
  
  unsigned int  num_intersections = 0;
  unsigned int  temp;
  int  xsize;
  int  ysize;
  int  pass = 0;
  float  stretchX;
  unsigned int  alloc_intersections = 16;
  ::brook::stream m1(::brook::getStreamType(( TransposedBBox  *)0), bboxwidt1 , bboxheit1,-1);
  ::brook::stream m2(::brook::getStreamType(( BBox  *)0), bboxwidt2 , bboxheit2,-1);
  ::brook::stream t1List(::brook::getStreamType(( Tri  *)0), widt1 , heit1,-1);
  ::brook::stream t2List(::brook::getStreamType(( Tri  *)0), widt2 , heit2,-1);
  vector <brook::stream> hitsL;
  vector <brook::stream> nextNodeL;
  vector <brook::stream> travL;
  vector <brook::stream> temptravL;
  vector <brook::stream> trioutL;
  vector <brook::stream> hitsT;
  vector <brook::stream> nextNodeT;
  vector <brook::stream> travT;
  vector <brook::stream> temptravT;
  vector <brook::stream> trioutT;  
  for (int rr=0;rr<2;++rr) {
    Traverser  baseTraverser;

    if (rr==0) {
      hitsL.push_back(::brook::stream(::brook::getStreamType(( float4  *)0), 1 , 1,-1));
      nextNodeL.push_back(brook::stream(::brook::getStreamType(( float4  *)0), 1 , 1,-1));
      travL.push_back(brook::stream(::brook::getStreamType(( Traverser  *)0), 1 , 1,-1));
      
    }
    if (rr==2) {
      unsigned int i;
      for (i=0;i<hitsL.size();++i) {
        hitsT.push_back(::brook::stream(::brook::getStreamType(( float4  *)0), 
                                        hitsL[i]->getExtents()[0] , 
                                        hitsL[i]->getExtents()[1],
                                        -1));
      }
      for (i=0;i<nextNodeL.size();++i) {
        nextNodeT.push_back(::brook::stream(::brook::getStreamType(( float4  *)0),
                                            nextNodeT[i]->getExtents()[0],
                                            nextNodeT[i]->getExtents()[1],
                                            -1));
      }
      for (i=0;i<travL.size();++i) {
        travT.push_back(brook::stream(::brook::getStreamType(( Traverser  *)0), 
                                      travL[i]->getExtents()[0],
                                      travL[i]->getExtents()[1],
                                      -1));
      }
      for (i=0;i<temptravL.size();++i) {
        temptravT.push_back(brook::stream(::brook::getStreamType(( Traverser  *)0),         
                                          temptravL[i]->getExtents()[0],
                                          temptravL[i]->getExtents()[1],
                                          -1));
        
      }
      for (i=0;i<trioutL.size();++i) {
        trioutT.push_back(brook::stream(::brook::getStreamType(( Traverser  *)0),         
                                        trioutL[i]->getExtents()[0],
                                        trioutL[i]->getExtents()[1],
                                        -1));
        
      }
    }
    ::brook::stream hits= hitsL[0];
    ::brook::stream nextNode=nextNodeL[0];
    ::brook::stream trav=travL[0];
    ::brook::stream temptrav;
    
    baseTraverser.index = float4 (0,0,0,0);
    baseTraverser.Translation = trans;
    baseTraverser.Rotationx = rX;
    baseTraverser.Rotationy = rY;
    baseTraverser.Rotationz = rZ;
    streamRead(trav,&baseTraverser);
    *intersections = (float4 *) (malloc(alloc_intersections * sizeof(float4 ) ));
    streamRead(t1List,t1);
    streamRead(t2List,t2);
    streamRead(m2,bboxest2);
    TransposeBBoxes(bboxwidt1 * bboxheit1,bboxest1);
    streamRead(m1,bboxest1);
    int iter=0;
    num_intersections=0;
    do   {
      Collide(trav,m1,m2,nextNode,hits);
      if (rr==0){
        hitsL.push_back(hits);
        assert(hitsL.size()==iter+2);
        nextNodeL.push_back(nextNode);
        temptravL.push_back(brook::stream());
        trioutL.push_back(brook::stream());
      }
      hits= hitsL[iter+1];
      nextNode=nextNodeL[iter+1];
      xsize = (int ) (streamSize(nextNode).x);
      ysize = (int ) (streamSize(nextNode).y);
      stretchX = (ysize * 2 > 2048) ? (1.000000f) : (0.000000f);
      if (stretchX)
        xsize *= 2;
      else
        ysize *= 2;
      if ( xsize && ysize)
        {
          if (rr==0) {
            temptravL[iter]=::brook::stream (::brook::getStreamType(( Traverser  *)0), ysize , xsize,-1);
          }                   
          temptrav = temptravL[iter];          
          streamSwap(temptrav,trav);
          updateCurrentNode(stretchX,nextNode,m1,m2,temptrav,trav);
          if (rr==0) {
            travL.push_back(trav);
          }
          trav = travL[iter+1];
        }
      
      temp = (int ) (streamSize(hits).x) * (int ) (streamSize(hits).y);
      if (debug_rapid&&(xsize * ysize))
        {
          Traverser  *tmptrav;
          
          tmptrav = (Traverser *) (malloc(sizeof(Traverser )  * xsize * ysize));
          streamWrite(trav,tmptrav);
          pass++;
          checkPassCorrectness(tmptrav,xsize * ysize,pass);
          free(tmptrav);
        }
      if (debug_rapid) {
        printf("Pass %d Num Tri Checks %d num nodes %d %d\n",pass,temp,xsize,ysize);
      }
      
      if (temp) {
        if (rr==0) {
          trioutL[iter]=::brook::stream(::brook::getStreamType((float4*)0),
                                            nextNode->getExtents()[0],
                                            nextNode->getExtents()[1],
                                            -1);
        }
        brook::stream triout= trioutL[iter];
        CheckTriangleCollide(csRapidColliderrX,csRapidColliderrY,csRapidColliderrZ,csRapidColliderT,hits,t1List,t2List,triout);
        if (rr==0) {
          trioutL[iter]=triout;
        }
        
        temp = (int ) (streamSize(triout).x) * (int ) (streamSize(triout).y);
      if (temp)
      {
        unsigned int  i;

        while (num_intersections + temp > alloc_intersections)
        {
          alloc_intersections *= 2;
          *intersections = (float4 *) (realloc(*intersections,alloc_intersections * sizeof(float4 ) ));
        }

        streamWrite(triout,*intersections + num_intersections);
        for (i = 0; i < temp; ++i)
        {
          float4  f4 = *(*intersections + num_intersections + i);

          if (!(f4.x >= 0 && f4.x < streamSize(t1List).x))
            break;
          if (!(f4.y >= 0 && f4.y < streamSize(t1List).y))
            break;
        }

        num_intersections += i;
        if (debug_rapid)
          printf("Detected %d hits\n",i);
      }

      } 
    iter++;
    }
    while (xsize && ysize);
  }

    return num_intersections;
}
