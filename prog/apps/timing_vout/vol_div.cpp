#include <brook.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "volume_division.h"
#include "ppm3d.h"
#include "timing.h"
char use_vout_filter=1;
char use_vout_amplify=1;
bool firstRound=true;
bool vanilla=false;
bool forcevanilla=true;
int count=0;
int hpcount=0;
int qtcount=0;
bool debug_model=false;
static std::vector<brook::stream> savedStreams;
::brook::stream & quickAllocStream (const __BRTStreamType *t, int wid, int len, int gar){

   if (forcevanilla||vanilla) {
      static brook::stream s;
      s= brook::stream(t,wid,len,gar);
      return s;
   }
   if (firstRound) {
      savedStreams.push_back(brook::stream(t,wid,len,gar));
      return savedStreams.back();
   }else {
      return savedStreams[count++%savedStreams.size()];
   }
}
static std::vector<brook::stream> hpsavedStreams;
::brook::stream & hpquickAllocStream (const __BRTStreamType *t, int wid, int len, int gar){

   if (vanilla) {
      static brook::stream s;
      s= brook::stream(t,wid,len,gar);
      return s;
   }
   if (firstRound) {
      hpsavedStreams.push_back(brook::stream(t,wid,len,gar));
      return hpsavedStreams.back();
   }else {
      return hpsavedStreams[hpcount++%hpsavedStreams.size()];
   }
}

static std::vector<brook::stream> qtsavedStreams;
::brook::stream & qtquickAllocStream (const __BRTStreamType *t, int wid, int len, int gar){

   if (vanilla) {
      static brook::stream s;
      s= brook::stream(t,wid,len,gar);
      return s;
   }
   if (firstRound) {
      qtsavedStreams.push_back(brook::stream(t,wid,len,gar));
      return qtsavedStreams.back();
   }else {
      return qtsavedStreams[qtcount++%qtsavedStreams.size()];
   }
}
void copy4( brook::stream in, brook::stream out);
void Aggregate4(brook::stream fourfloat, brook::stream in, brook::stream out);
void Aggregate34(brook::stream threefloat,brook::stream fourfloat, brook::stream in, brook::stream out);
void Aggregate3(brook::stream threefloat, brook::stream in, brook::stream out);
void Aggregate2(brook::stream twofloat, brook::stream in, brook::stream out);
void Aggregate1(brook::stream onefloat, brook::stream in, brook::stream out);
void  processSliceNoCompact (::brook::stream curgather,
		::brook::stream nextslice,
		::brook::stream vertex,
		const __BRTIter& center,
		const __BRTIter& up,
		const __BRTIter& forward,
		const __BRTIter& upforward,
                             const float2  slice) ;
void processSlice(brook::stream curslice,
                  brook::stream nextslice,
                  ::brook::stream &__vertex_stream,
                  const __BRTIter& center,
                  const __BRTIter& up,
                  const __BRTIter& forward,
                  const __BRTIter& upforward,
                  const float2  slice);
void  processTrianglesNoCompact (::brook::stream trianglesA,
                                 ::brook::stream trianglesB,
                                 ::brook::stream trianglesC,
                                 ::brook::stream trianglesD,
                                 ::brook::stream trianglesE,
                                 ::brook::stream vertices1,
                                 ::brook::stream volumeTriangles);
void  processTrianglesNoCompactOneOut (::brook::stream trianglesA,
                                       ::brook::stream vertices1,
                                       ::brook::stream volumeTriangles);
void  processTriangles (::brook::stream &__triangles_stream,
                        ::brook::stream vertices,
                        ::brook::stream volumeTriangles,
                        const __BRTIter& streamsize);
void  processFirstTriangles (::brook::stream triangles,
                             ::brook::stream vertices,
                             ::brook::stream volumeTriangles);
typedef float3 Triangle[5][3];
/**
 * The following function generates the triangle lookup table from the raw
 * edge list data stored in volume_division.h
 * First it fills everything with the sentinel.
 * Then it goes through up to 15 items, filling the triangle table until a 
 * -1 in the edge list is encountered at which point it stops adding to the tex
 */
char volumeTriangles(Triangle tri[256]) {
  unsigned int i,j,k,m;

   for (i=0;i<256;++i) {
      for (m=0;m<15;++m) {
        // fill with sentinel
         tri[i][(m/3)%5][m%3]=float3(1.0f/(float)floor(.5),
                                     1.0f/(float)floor(.5),
                                     1.0f/(float)floor(.5));
      }
      j=0;
      for (j=0;;j+=3) {
         if (m_triTable[i][j]==-1){// our edge list is -1, stop!
           m_triNum[i]=(float)(j/3);//set the count of triangles to j
           break; // it's too dangerous we have to slow down first!
         }
         for (k=0;k<3;++k) {
           // single out the point that will be set on this pass
            float3 * p=&tri[i][j/3][k];
            p->x=m_triTable[i][j+k];// for debugging only, overwritten
            p->y=m_triTable[i][j+k];// for debugging only
            p->z=m_triTable[i][j+k];// for debugging only
            switch ((int)m_triTable[i][j+k]) {
              // set the appropriate coordinates in a 1x1x1 cube
              // given the lookup value assuming the data is exactly
              // in the middle--i.e. no adaptive
            case 0:
               p->x=p->z=0;p->y=0.5f;
               break;
            case 1:
               p->x=0.5f;p->z=0;p->y=1.0f;
               break;
            case 2:
               p->x=1.0f;p->z=0;p->y=0.5f;
               break;
            case 3:
               p->x=0.5f;p->z=0;p->y=0;
               break;
            case 4:
               p->x=0;p->z=1.0f;p->y=0.5f;
               break;
            case 5:
               p->x=0.5f;p->z=1.0f;p->y=1.0f;
               break;
            case 6:
               p->x=1.0f;p->z=1.0f;p->y=0.5f;
               break;
            case 7:
               p->x=0.5f;p->z=1.0f;p->y=0;
               break;
            case 8:
               p->x=p->y=0; p->z=0.5f;
               break;
            case 9:
               p->x=0;p->y=1.0f;p->z=0.5f;
               break;
            case 10:
               p->x=1.0f;p->y=1.0f;p->z=0.5f;
               break;
            case 11:
               p->x=1.0f;p->y=0;p->z=0.5f;
               break;
            }
         }
      }
   }
  return 0;
}

/**compute the volume triangles only once and cache it.
 */
Triangle* getVolumeTriangles () {
  static Triangle tri[256];
  static char eval =volumeTriangles(tri);
  return tri;
}

/** Because brook doesn't allow casts in initializers
 */
float tof (int a) {
  return (float)a;
}

/** Because brook doesn't allow casts in initializers
 */
int toi (float a) {
  return (int)a;
}

/** int main (int argc, char ** argv)
 *  usage
 *  ./volume_division [-nofilter] [-noamplify] <width>
 *     runs the isosurface on a sphere generated into texture coordinates
 *  ./volume_division [-nofilter] [-noamplify] <filename.ppm>
 *     runs the isosurface on a modified ppm 3d... may not be 100% valid
 *  ./volume_division [-nofilter] [-noamplify] <width_height> <depth>
 *     runs the isosurface on a 3d synthetic dataset that avoids texture reads
 *  The -nofilter option generates exactly 1 lookup index per voxel
 *   without said option a lookup is only generated when the voxel will have
 *   at least 1 triangle in it
 *  The -noamplify option generates exactly 5 triangles per lookup index given
 *   without said option the number of triangles outputted is the minumim for 
 *   the given voxel
 */
int volume_division (int argc, char ** argv) {
   int i;
   ::brook::stream volumeTriangles(::brook::getStreamType(( float3  *)0), 256 , 15,-1);
   struct ppm dat;
   unsigned int wasted_tri=0;
   unsigned int total_tri=0;
   unsigned int numspheres=1;
   float spheredist=2.0f;
   float * slice=0;
   char generatedData=0;
   char useCharTex=0;
   char synthNoFilter=0;
   for (i=0;i<argc;++i) {
     char match=0;
     int j;
     if (strcmp(argv[i],"-nofilter")==0) {
       match=1;
       use_vout_filter=0;//turn off filtering
     }else if (strcmp(argv[i],"-noamplify")==0) {
       match=1;
       use_vout_amplify=0;//turn off amplification
     }else if (strcmp(argv[i],"-filter")==0) {
       match=1;
       use_vout_filter=1;//deprecated
     }else if (strcmp(argv[i],"-synthnoamplify")==0) {
       match=1;
       synthNoFilter=1;     
     }else if (strcmp(argv[i],"-amplify")==0) {
       match=1;
       use_vout_amplify=1;//deprecated
     }else if (strncmp(argv[i],"-numspheres",11)==0) {
       match=1;

       numspheres=atoi(argv[i]+11);
     }else if (strncmp(argv[i],"-spheredist",11)==0) {
       match=1;
       spheredist=(float)atoi(argv[i]+11);
     }else if (strcmp(argv[i],"-debug")==0) {
       match=1;
       debug_model=true;
     }else if (strcmp(argv[i],"-precache")==0) {
       forcevanilla=false;
       match=1;
     }else if (strcmp(argv[i],"-char")==0) {
       useCharTex=1;
       match=1;
     }
     if (match) {
       for (j=i+1;j<argc;++j) argv[j-1]=argv[j];
       argc--;
       i--;
     }
   }
   if (argc<2) {
     fprintf (stderr,"usage\n"
           "  %s [-nofilter] [-noamplify] <width>\n"
           "     runs the isosurface on a sphere generated into texture coordinates\n"
           "  %s [-nofilter] [-noamplify] <filename.ppm>\n"
           "     runs the isosurface on a modified ppm 3d... may not be 100% valid\n"
           "  %s [-nofilter] [-noamplify] <width_height> <depth>\n"
           "     runs the isosurface on a 3d synthetic dataset that avoids texture reads\n"
           "  The -nofilter option generates exactly 1 lookup index per voxel\n"
           "   without said option a lookup is only generated when the voxel will have\n"
           "   at least 1 triangle in it\n"
           "  The -noamplify option generates exactly 5 triangles per lookup index given\n"
           "   without said option the number of triangles outputted is the minumim for \n"
           "   the given voxel\n",argv[0],argv[0],argv[0]);
      exit (1);
   }
   // read in volume triangle lookups
   streamRead(volumeTriangles,getVolumeTriangles());   
   if (atoi(argv[1])==0) {
      dat = openPPM (argv[1]);
      if (!dat.fp)
         exit(1);
   }else {
      dat = randomPPM(atoi(argv[1]),atoi(argv[1]),atoi(argv[1]));
   }
   slice = mallocSlice(dat);
   std::vector<brook::stream> volumeData;
   std::vector<brook::stream> vertexData;
   float toagg[4]={0,0,0,0};
   ::brook::stream agg(::brook::getStreamType(( float  *)0), 2 , 2,-1);
   streamRead(agg,toagg);
   dat.numspheres=numspheres;
   dat.spheredist=spheredist;
   for (i=0;i<(int)dat.depth;++i) {
     int numactivetextures=dat.depth;
      readPPM3dSlice(dat,i,slice);
      if (useCharTex) {
#ifdef _WIN32
        volumeData.push_back(::brook::stream (brook::getStreamType(( char  *)0), dat.height , dat.width,-1));
#endif
        streamRead(volumeData.back(),slice);        
      }else {
        volumeData.push_back(::brook::stream (brook::getStreamType(( float  *)0), dat.height , dat.width,-1));
        streamRead(volumeData.back(),slice);
      }
     if (i==numactivetextures-1) {                      
        ::brook::stream v(::brook::getStreamType(( float4  *)0), dat.height , dat.width,-1);    
                  
        float * temp=(float*)malloc(sizeof(float)*dat.width*dat.height);                        
        for (int k=0;k<2;++k) {                          
         ::brook::iter center(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (0.000000f,0.000000f)).x, (float2 (0.000000f,0.000000f)).y, (float2 (tof(dat.width),tof(dat.height))).x, (float2 (tof(dat.width),tof(dat.height))).y, -1);                                    
         // this iterator tracks exactly 1 pixel higher in our current or next                  
         // 2d sliceor truy                              
         ::brook::iter up(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (0.000000f,1.000000f)).x, (float2 (0.000000f,1.000000f)).y, (float2 (0.000000f + tof(dat.width),1.000000f + tof(dat.height))).x, (float2 (0.000000f + tof(dat.width),1.000000f + tof(dat.height))).y, -1);                               
         // This iterator tracks +1 in x for the current or next 2d slice                       
         ::brook::iter upforward(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (1.000000f,1.000000f)).x, (float2 (1.000000f,1.000000f)).y, (float2 (1.000000f + tof(dat.width),1.000000f + tof(dat.height))).x, (float2 (1.000000f + tof(dat.width),1.000000f + tof(dat.height))).y, -1);                        
         // This iterator tracks +1 in both x & y for the current or next slice                 
         ::brook::iter forward(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (1.000000f,0.000000f)).x, (float2 (1.000000f,0.000000f)).y, (float2 (1.000000f + tof(dat.width),tof(dat.height))).x, (float2 (1.000000f + tof(dat.width),tof(dat.height))).y, -1);           
          start = GetTimeMillis();                       
                  
          for (int j=0;j<(int)dat.depth;++j) {           
            float2 sliceZ;                               
            sliceZ.x=(float)j;sliceZ.y=(float)(j+1);     
            processSliceNoCompact(volumeData[j%numactivetextures],                              
                                  volumeData[(j+1)%numactivetextures],                          
                                  v,//volumeData[(j+2)%numactivetextures],                      
                                  center,                
                                  up,                    
                                  forward,               
                                  upforward,             
                                  sliceZ);               
            Aggregate4(v,agg,agg);                       
          }       
          //streamWrite(volumeData[(j+2)%numactivetextures],temp);                              
          //Aggregate1(volumeData[(j+1)%2],agg,agg);     
          streamWrite(agg,toagg);                        
          stop = GetTimeMillis();                        
          printf ("One Time Cost %f\n",(float)(stop-start));                                    
        }         
      }       

   }
   std::vector<int> sizesx;
   std::vector<int> sizesy;
   // now we begin the actual algorithm
   ::brook::stream vbak(::brook::getStreamType(( float4  *)0), dat.height , dat.width,-1);
   for (unsigned int rr=0;rr<1;++rr) {
     ::brook::stream v(::brook::getStreamType(( float4  *)0), dat.height , dat.width,-1);

     count=0;
     hpcount=0;
     qtcount=0;
     if (rr!=0)
       firstRound=false;
     if (rr>=2) {use_vout_amplify=0;use_vout_filter=0;}
     
         unsigned int i;
         // which volumetric slice is this run going to produce
         float2 sliceZ;
         // this iterator tracks the center of the block we're working with
         ::brook::iter center(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (0.000000f,0.000000f)).x, (float2 (0.000000f,0.000000f)).y, (float2 (tof(dat.width),tof(dat.height))).x, (float2 (tof(dat.width),tof(dat.height))).y, -1);
         // this iterator tracks exactly 1 pixel higher in our current or next
         // 2d sliceor truy 
         ::brook::iter up(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (0.000000f,1.000000f)).x, (float2 (0.000000f,1.000000f)).y, (float2 (0.000000f + tof(dat.width),1.000000f + tof(dat.height))).x, (float2 (0.000000f + tof(dat.width),1.000000f + tof(dat.height))).y, -1);
         // This iterator tracks +1 in x for the current or next 2d slice
         ::brook::iter upforward(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (1.000000f,1.000000f)).x, (float2 (1.000000f,1.000000f)).y, (float2 (1.000000f + tof(dat.width),1.000000f + tof(dat.height))).x, (float2 (1.000000f + tof(dat.width),1.000000f + tof(dat.height))).y, -1);
         // This iterator tracks +1 in both x & y for the current or next slice
         ::brook::iter forward(__BRTFLOAT2, dat.height , dat.width, -1, (float2 (1.000000f,0.000000f)).x, (float2 (1.000000f,0.000000f)).y, (float2 (1.000000f + tof(dat.width),tof(dat.height))).x, (float2 (1.000000f + tof(dat.width),tof(dat.height))).y, -1);
         if (dat.width>512) {// we don't have so much memory.
           printf("Exceeded 512 wide texture bounds %d\n",dat.width);
           return 1;
         }
         start = GetTimeMillis();
         // setup the current and next slice Z coordinates
         sliceZ.x=0.0f;sliceZ.y=1.0f;
         for (i=0;i<dat.depth-1;++i) {//loop through z values
               if (firstRound) {
                   TallyKernel("processSlice",
                               volumeData[i],
                               i!=dat.depth-1?volumeData[i+1]:volumeData[i]);
               }
              // read a new slice for the 'next' category
               use_vout_filter?
                 /// only output triangle lookup indices and centers 
                 /// when those indices are not 0 or 255 (empty)
                        processSlice(volumeData[i],
                                     i!=dat.depth-1?volumeData[i+1]:volumeData[i],
                                     v,
                                     center,
                                     up,
                                     forward,
                                     upforward,
                                     sliceZ):
                 /// produce exactly the number of lookup indices as textures
                 processSliceNoCompact(volumeData[i],
                                       i!=dat.depth-1?volumeData[i+1]:volumeData[i],
                                       v,
                                       center,
                                       up,
                                       forward,
                                       upforward,
                                       sliceZ);
               
            if (rr==0) {
               static float4 * tris;
               tris=(float4*)malloc(sizeof(float4)*dat.width*dat.height);
               sizesx.push_back(toi(streamSize(v).x));
               sizesy.push_back(toi(streamSize(v).y));
               streamWrite(v,tris);
               for (int i=0;i<sizesx.back()*sizesy.back();++i) {
                  if (tris[i].w>=0&&tris[i].w<=255.5) {
                     unsigned int tmp=(unsigned int)m_triNum
                        [(unsigned int)tris[i].w];
                     total_tri+=tmp;
                     wasted_tri+=5-tmp;
                  }
               }
            }
            if (rr==0&&i==(dat.depth/2)) {
               copy4(v,vbak);
            }
                // we know the width of the triangles (i.e. for the address
                // calc) will be 4x as big...we have no idea how many 3x
                // vout[3] outputs there will be (0 or 3 for each tri)
            ::brook::stream trianglesB;
            if (!debug_model)
               trianglesB=hpquickAllocStream(::brook::getStreamType(( float3  *)0), (rr>=2?sizesy[i]:toi(streamSize(v).y))*15 , (rr>=2?sizesx[i]:toi(streamSize(v).x)) ,-1);
            else if (rr!=0&&!use_vout_amplify)
               trianglesB=brook::stream(::brook::getStreamType(( float3  *)0), (rr>=2?sizesy[i]:toi(streamSize(v).y))*15 , (rr>=2?sizesx[i]:toi(streamSize(v).x)) ,-1);
            if (sizesy[i]){

              if (use_vout_amplify||(rr==0&&!synthNoFilter)) {
                 ::brook::stream triangles=qtquickAllocStream(::brook::getStreamType(( float3  *)0),000?sizesy[i]:1 , 000?(sizesx[i]*4):(toi(streamSize(v).x) * 4),-1);
                 ::brook::stream trianglesFirst=qtquickAllocStream(::brook::getStreamType(( float3  *)0), toi(streamSize(v).y) , toi(streamSize(v).x) * 3,-1);

                // multiply our width by 4x since we could output up to 4x
                // of our original values
                ::brook::iter newsize(__BRTFLOAT2, 
                                      rr>=2?sizesy[i]:toi(streamSize(v).y) , 
                                      rr>=2?sizesx[i]*4:(toi(streamSize(v).x) * 4), 
                                      -1, 
                                      (float2 (0,0)).x, 
                                      (float2 (0,0)).y, 
                                      (float2 ((rr>=2?(float)sizesx[i]:streamSize(v).x) * 4.0f,
                                               rr>=2?(float)sizesy[i]:streamSize(v).y)).x, 
                                      (float2 ((rr>=2?sizesx[i]:streamSize(v).x) * 4.0f,
                                               rr>=2?sizesy[i]:streamSize(v).y)).y, 
                                      -1);
                 // our first triangle lookups are going to be exactly 3x as big
                                      
                // process all the first triangles
                processFirstTriangles(trianglesFirst,v,volumeTriangles);
                Aggregate3(trianglesFirst,agg,agg);
                if (firstRound) {
                  TallyKernel("processFirstTriangles",trianglesFirst);
                }
                // process the rest
                processTriangles(triangles, 
                                 v,
                                 volumeTriangles,
                                 newsize);
                if (firstRound) {
                  TallyKernel("processTriangles",newsize);
                }

                //write it to the same place in memory
                if (debug_model) {
                  vertexData.push_back(trianglesFirst);
                  vertexData.push_back(triangles);
                }
                 Aggregate3(triangles,agg,agg);
                 /*
                 streamWrite(trianglesFirst,
                             consolidateVertices(dat,streamSize(trianglesFirst)));
                 streamWrite(triangles,
                             consolidateVertices(dat,streamSize(triangles)));
                 */
                                
              }else {
                  // each triangle stream will be 3x bigger than the volume
                  
                 // output exactly 5 vertices for each input
                if (rr==0) {
                  TallyKernel("processTrianglesNoCompactOneOut",trianglesB);
                }
 
                 processTrianglesNoCompactOneOut(trianglesB,
                                           vbak, 
                                           volumeTriangles);
                 // write them all into mem
                 Aggregate34(trianglesB,v,agg,agg);
                 
                 //Aggregate3(trianglesB,agg,agg);
                 //Aggregate3(trianglesC,agg,agg);
                 //Aggregate3(trianglesD,agg,agg);
                 //Aggregate3(trianglesE,agg,agg);
                 if (!debug_model)
                    vertexData.push_back(trianglesB);
                /*
                vertexData.push_back(trianglesB);
                vertexData.push_back(trianglesC);
                vertexData.push_back(trianglesD);
                vertexData.push_back(trianglesE);
                */
                /*
                 streamWrite(trianglesA,
                             consolidateVertices(dat,streamSize(trianglesA)));
                 streamWrite(trianglesB,
                             consolidateVertices(dat,streamSize(trianglesB)));
                 streamWrite(trianglesC,
                             consolidateVertices(dat,streamSize(trianglesC)));
                 streamWrite(trianglesD,
                             consolidateVertices(dat,streamSize(trianglesD)));
                 streamWrite(trianglesE,
                             consolidateVertices(dat,streamSize(trianglesE)));
*/
              }
            }
            
            // increment the z
            sliceZ.x++;sliceZ.y++;
         }
         int tsize = vertexData.size()-1;
         int j;
         if (!vertexData.empty())
            
            Aggregate3(vertexData[tsize],agg,agg);
         streamWrite(agg,toagg);
         stop = GetTimeMillis();
         for (j=1;j<tsize;j+=2) {
           //Aggregate34(vertexData[tsize],vertexData[tsize-1],agg,agg);
         }
         streamWrite(agg,toagg);

         if (rr==0)
            printf ("\nTotal Triangles %d Wasted Triangles %d Total Volume wid:%d hei %d dep %d\n",total_tri,wasted_tri,dat.width,dat.height,dat.depth);
         printf ("Ready time %f %f ",(float)(stop-start),(float)(GetTimeMillis()-start));
         for (j=0;j<(int)vertexData.size();++j) {
            if (rr<2) {
               streamWrite(vertexData[j],
                           consolidateVertices(dat,streamSize(vertexData[j])));
            }else {
               float * k =(float*) malloc(dat.width*dat.height*sizeof(float3)*15);
               streamWrite(vertexData[j],k);
               
            }
           if (j==0){
             stop = GetTimeMillis();
             printf ("Total time %f",(float)(stop-start));
             if (rr>=2) break;
           }
         }
         vertexData.clear();
         printf("Total time with reads %f\n",(float)(GetTimeMillis()-start));
         if (debug_model&&rr<2)printVolume(dat);
         //         dat.vertices.clear();
         if (synthNoFilter)
           break;
   }
   free(slice);
   volumeData.clear();
   //write the mesh to stdout
   return 0;
   
 }


