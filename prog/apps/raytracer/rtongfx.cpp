/*
 * XXX This file is old and has been replaced by main.cpp.  Don't use it.
 */

#error "Don't compile me.  See main.cpp instead"
#if 0
#define CBOX
//#define GLASSNER

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Types.h"
#include "Bitvector.h"
#include "FileIO.h"
#include "ppmImage.h"

extern void TraceRays( float lookfromX, float lookfromY, float lookfromZ,
		       float uX, float uY, float uZ,
		       float vX, float vY, float vZ,
		       float wX, float wY, float wZ,
		       float tx, float ty,
		       float gridminX, float gridminY, float gridminZ,
		       float gridmaxX, float gridmaxY, float gridmaxZ,
		       float griddimX, float griddimY, float griddimZ,
		       int numvox,
		       float gridvsizeX, float gridvsizeY, float gridvsizeZ,
		       int* grid_trilist_offset,
		       int* grid_trilist,
		       int trilistsize,
		       int numtris, 
		       float* triv0, float* triv1, float* triv2,
		       float* trin0, float* trin1, float* trin2,
		       float* tric0, float* tric1, float* tric2,
		       float pointlightX, float pointlightY, float pointlightZ,
		       float* imgbuf);
  

//variables
float W = 256;
float H = 256;

float scale = 1.0f;

ppmImage img((int)W, (int)H);

//changing per-frame scene parameters -- should initialize from command line, hardcode for now
#ifdef GLASSNER
char *infile = "glassner04.vox";
Point3 lookfrom(-7.610099f, -339.653137f, 237.733948f);
Point3 lookat(23.224901f, 980.535034f, -321.560577f);
Vector3 vup(0,0,1);
float fov = 55;
Point3 pointlight(-206.808f, -16.2987f, 297.281f);
#endif
#ifdef CBOX
char *infile = "cornell02.vox";
Point3 lookfrom(278.0f,273.0f, -800.0f );
Point3 lookat(278.1f, 273.1f, 0.0f);
Vector3 vup(0,1,0);
float fov = 35;
Point3 pointlight(278.0f, 530.0f, 279.5f);
#endif

//some camera stuff
Vector3 u, v, w;
float tx, ty;

//static scene data -- read in from preprocessed file
static Tuple3i grid_dim;
static Tuple3f grid_min;
static Tuple3f grid_max;
static Tuple3f grid_vsize;
static int numtris = 0;
static int *grid_trilist_offset;
static int *grid_trilist;
static int grid_trilist_size;
static BitVector *grid_bitmap;
static Point3 *v0;
static Point3 *v1;
static Point3 *v2;
static Normal3 *n0;
static Normal3 *n1;
static Normal3 *n2;
static Spectra *c0;
static Spectra *c1;
static Spectra *c2;



void CreateCamera(){
  w = (lookat - lookfrom).Normalize();
  u = Vector3::Cross(vup,w).Normalize();
  v = Vector3::Cross(w,u).Normalize();
  ty = tanf( (M_PI/180.0f*fov) / 2.0f);
  tx = ty*(W/H);
}


int main(int argc, char **argv){

  // grid_bitmap is unused in the rest of the application
  ReadVoxFile( infile, grid_dim, grid_min, grid_max, grid_vsize, grid_bitmap,
	       grid_trilist_offset, grid_trilist_size, grid_trilist, numtris,
               v0, v1, v2, n0, n1, n2, c0, c1, c2 );
  
  fprintf(stderr, "numtris: %i, numvox: %i, trilistsize: %i\n",
	  numtris, grid_dim.x*grid_dim.y*grid_dim.z, grid_trilist_size);
  /*
  for(int i=0; i<grid_trilist_size; i++)
    fprintf(stderr, "%i ", grid_trilist[i]);
  fprintf(stderr, "\n");
  */
  CreateCamera();
  TraceRays( lookfrom.x, lookfrom.y, lookfrom.z,
	     u.x, u.y, u.z,
	     v.x, v.y, v.z,
	     w.x, w.y, w.z,
	     tx, ty,
	     grid_min.x, grid_min.y, grid_min.z,
	     grid_max.x, grid_max.y, grid_max.z,
	     (float)grid_dim.x, (float)grid_dim.y, (float)grid_dim.z,
	     grid_dim.x*grid_dim.y*grid_dim.z,
	     grid_vsize.x, grid_vsize.y, grid_vsize.z,
	     grid_trilist_offset,
	     grid_trilist,
	     grid_trilist_size,
	     numtris, 
	     (float*)v0, (float*)v1, (float*)v2,
	     (float*)n0, (float*)n1, (float*)n2,
	     (float*)c0, (float*)c1, (float*)c2,
	     pointlight.x, pointlight.y, pointlight.z,
	     img.Data() );
  img.Write("out.ppm");
  return 0;
}
#endif
