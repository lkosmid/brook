#include <brook.hpp>
typedef struct Tri_t {
  float3 A;
  float3 B;
  float3 C;
} Tri;

extern void SimpleCheckTriangleCollide(float * matrix,
                                       int sizex,int sizey,
                                       Tri * hitsa,
                                       Tri* hitsb,
                                       float4 * ohits);
#define SIZEX 1024
#define SIZEY 1024
int main (int argc, char ** argv) {
  Tri * a= (Tri*)malloc(SIZEX*SIZEY);
  Tri * b= (Tri*)malloc(SIZEX*SIZEY);
  float4 * rez = (float4*) malloc (SIZEX*SIZEY);
  float matrix[16]={1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1};

  free (a);
  free (b);
  free (rez);
  return 0;
}
