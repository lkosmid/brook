#include <brook.hpp>
#include <stdio.h>
#include "csgeom/rapcol.h"
extern void LoadPly (const char * file, std::vector<Tri> &ret);
extern unsigned int doCollide(unsigned int wida, unsigned int heia, Tri * a,
                              unsigned int bboxwida, unsigned int bboxheia, BBox * bboxesa,
                              unsigned int widb, unsigned int heib, Tri * b,
                              unsigned int bboxwidb, unsigned int bboxheib, BBox * bboxesb,
                              float3 rX, float3 rY, float3 rZ,
                              float3 trans,
                              float4 **intersections);
float3 convertVec(csVector3 v){
  return float3 (v.x,v.y,v.z);
}

int main (int argc, char ** argv) {
  float4 * intersections =0;
  csMatrix3 rot(0,1,0,
                1,0,0,
                0,0,1);
  csVector3 trans(0,0,0);
  srand(1);
  std::vector <bsp_polygon> model;
  std::vector<Tri> triangles;
  std::vector<BBox> bboxes;
  LoadPly (argc>1?argv[1]:"dragon.ply",model);
  csRapidCollider collide(model);
  collide.createBrookGeometry(bboxes,triangles);
  collide.CollideRecursive(const_cast<csCdBBox*>( collide.GetBbox()),
                           const_cast<csCdBBox*>(collide.GetBbox()),
                           rot,
                           trans);

  fprintf (stderr,"Num Collisions %d Num BBoxes %d Num Triangles %d",
           csRapidCollider::numHits,
           bboxes.size(),
           triangles.size());
  unsigned int num = doCollide(triangles.size()/2048,2048,&triangles[0],
                               bboxes.size()/2048,2048,&bboxes[0],
                               triangles.size()/2048,2048,&triangles[0],
                               bboxes.size()/2048,2048,&bboxes[0],
                               convertVec(rot.Row1()),
                               convertVec(rot.Row2()),
                               convertVec(rot.Row3()),
                               convertVec(trans),
                               &intersections);
  fprintf (stderr,"\nNum Collisions brook %d\n",num);
  for (unsigned int i=0;i<num;++i) {
     printf ("{%f %f %f %f}\n",intersections[i].x,
             intersections[i].y,
             intersections[i].z,
             intersections[i].w);
  }
  return 0;
}
