#include <brook.hpp>
#include "csgeom/rapcol.h"
extern void LoadPly (const char * file, std::vector<Tri> &ret);
extern unsigned int doCollide(unsigned int wida, unsigned int heia, Tri * a,
                              unsigned int bboxwida, unsigned int bboxheia, BBox * bboxesa,
                              unsigned int widb, unsigned int heib, Tri * b,
                              unsigned int bboxwidb, unsigned int bboxheib, BBox * bboxesb,
                              float3 rX, float3 rY, float3 rZ,
                              float3 trans,
                              float4 **intersections);

int main (int argc, char ** argv) {
  float4 * intersections =0;
  float matrix[16]={1,0,0,0,
                    0,0,1,0,
                    0,1,0,0,
                    0,0,0,1};
  srand(1);
  std::vector <bsp_polygon> model;
  std::vector<Tri> triangles;
  std::vector<BBox> bboxes;
  LoadPly ("bunny.ply",model);
  csRapidCollider collide(model);
  collide.createBrookGeometry(bboxes,triangles);
  printf ("Num BBoxes %d Num Triangles %d",bboxes.size(),triangles.size());
  unsigned int num = doCollide(triangles.size()/2048,2048,&triangles[0],
                               bboxes.size()/2048,2048,&bboxes[0],
                               triangles.size()/2048,2048,&triangles[0],
                               bboxes.size()/2048,2048,&bboxes[0],
                               float3(matrix[0],matrix[4],matrix[8]),
                               float3(matrix[1],matrix[5],matrix[9]),
                               float3(matrix[2],matrix[6],matrix[10]),
                               float3(matrix[12],matrix[13],matrix[14]),
                               &intersections);
  for (unsigned int i=0;i<num;++i) {
     printf ("{%f %f %f %f}\n",intersections[i].x,
             intersections[i].y,
             intersections[i].z,
             intersections[i].w);
  }
  return 0;
}
