/*
 * main.cpp --
 *
 *      Main routine for the Brook version of Tim Purcell's raytracer (or at
 *      least, what started life as Tim Purcell's raytracer).
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <brook/brook.hpp>


#include "main.h"
#include "getopt.h"
#include "Bitvector.h"
#include "FileIO.h"
#include "streamTypes.h"
#include "ppmImage.h"

#include "built/tracerays.hpp"  /* Produced from tracerays.brh */


typedef struct Opts {
   char *progName;

   char *sceneName;     /* Precomputed scene grid / voxelization */
   char *imageFile;     /* Name of the output image */

   int width, height;   /* Dimensions of the output image */

   /*
    * Boolean options (always default to false)
    */

   bool verbose;

   /*
    * Extra state that doesn't map directly to commandline args
    */

   Scene *scene;
} Opts;


/*
 * Changing per-frame scene parameters --
 *      should initialize from command line, hardcode for now
 */
static Scene glassnerScene = {
   /* fileName: */      "glassner04.vox",

   /* lookFrom: */      float3(-7.610099f, -339.653137f, 237.733948f),
   /* lookAt: */        float3(23.224901f, 980.535034f, -321.560577f),
   /* up: */            float3(0,0,1),
   /* fov: */           55.0f,

   /* pointLight: */    float3(-206.808f, -16.2987f, 297.281f),
};
static Scene cboxScene = {
   /* fileName: */      "cornell02.vox",

   /* lookFrom: */      float3(278.0f,273.0f, -800.0f),
   /* lookAt: */        float3(278.1f, 273.1f, 0.0f),
   /* up: */            float3(0,1,0),
   /* fov: */           35.0f,

   /* pointLight: */    float3(278.0f, 530.0f, 279.5f),
};


/*
 * Static scene data --
 *      read in from preprocessed file
 */

static Grid grid;
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


/*
 * Usage --
 *
 *      Print program usage information and exit.
 *
 * Returns:
 *      void.
 */

static void
Usage(const char *progName)
{
   fprintf(stderr,
           "Usage: %s <options>\n"
           "  Options:\n"
           "  -?, --help\n"
           "            This message\n"
           "  -h, --height\n"
           "            Height for the image produced (256)\n"
           "  -o, --imageFile\n"
           "            Filename for the image produced (out.ppm)\n"
           "  -s, --scene\n"
           "            Name of the scene to raytrace (cbox or glassner)\n"
           "            (default: cbox)\n"
           "  -v, --verbose\n"
           "            Print extra information while running\n"
           "  -w, --width\n"
           "            Width for the image produced (256)\n",
           progName);
   exit(1);
}


/*
 * ParseOpts --
 *
 *      Process all the commandline options and set the various options
 *      accordingly.
 *
 * Returns:
 *      void.
 */

static void
ParseOpts(int *argc, char *argv[], Opts *opts)
{
   int opt;
   int opt_index;

   static struct option long_options[] = {
      {"help",         0, 0, '?'},
      {"height",       1, 0, 'h'},
      {"imageFile",    1, 0, 'o'},
      {"scene",        1, 0, 's'},
      {"verbose",      0, 0, 'v'},
      {"width",        1, 0, 'w'},
      {0, 0, 0, 0}
   };


   opts->progName = argv[0];
   while ((opt = getopt_long(*argc, argv, "?h:o:s:vw:",
                             long_options, &opt_index)) != EOF) {
      switch (opt) {
      case 'h': opts->height = atoi(optarg);         break;
      case 'o': opts->imageFile = optarg;            break;
      case 's': opts->sceneName = optarg;            break;
      case 'v': opts->verbose = true;                break;
      case 'w': opts->width = atoi(optarg);          break;

      case '?':
      default: Usage(opts->progName);                break;
      }
   }

   argv += optind;
   *argc -= optind;
   if (*argc < 0) {
     Usage(opts->progName);
   }

   if (strcmp(opts->sceneName, "cbox") == 0) {
      opts->scene = &cboxScene;
   } else if (strcmp(opts->sceneName, "glassner") == 0) {
      opts->scene = &glassnerScene;
   } else {
      fprintf(stderr, "Unknown scene: %s\n", opts->sceneName);
      Usage(opts->progName);
   }
   fprintf(stderr, "Raytracing the '%s' scene.\n", opts->sceneName);
}


/*
 * PrintGridInfo --
 *
 *      This function dumps various bits of information about the grid for
 *      the scene.
 *
 * Returns:
 *      void
 */

static void
PrintGridInfo(Grid *grid, Opts *opts)
{
   fprintf(stderr, "Grid: %i triangles, %ix%ix%i voxels, %i trilist size.\n",
           grid->nTris, grid->dim.x, grid->dim.y, grid->dim.z,
           grid->trilistSize);

   if (opts->verbose) {
      int maxLength, length, last, i;


      fprintf(stderr, "Triangle List:\n");
      maxLength = length = 0;
      for(i = 0; i < grid->trilistSize; i++) {
         fprintf(stderr, "%i ", grid->trilist[i]);
         if (grid->trilist[i] != -1) {
            length++;
         } else {
            fprintf(stderr, "Length: %d\n", length);
            if (length > maxLength) {
               maxLength = length;
            }
            length = 0;
         }
      }
      fprintf(stderr, "Max length: %d\n", maxLength);

      fprintf(stderr, "Triangle List Offsets:\n");
      maxLength = 0;
      last = grid->trilistOffset[0];
      for (i = 1; i < grid->dim.x * grid->dim.y * grid->dim.z; i++) {
         fprintf(stderr, "%i ", grid->trilistOffset[i]);

         /* The -1 subtracts off the space for the sentinel */
         length = grid->trilistOffset[i] - last - 1;
         last = grid->trilistOffset[i];
         if (length > maxLength) {
            maxLength = length;
         }
      }
      length = grid->trilistSize - last - 1;
      if (length > maxLength) {
         maxLength = length;
      }
      fprintf(stderr, "\n");
      fprintf(stderr, "Max length: %d\n", maxLength);
   }
}


/*
 * TraceRays --
 *
 *      Drives the actual ray-tracing process.  Currently all the heavy
 *      lifting happens here, but hopefully that'll change over time.
 *
 * Returns:
 *      void
 */

static void
TraceRays(const float3& lookFrom,
          const Camera& cam,

          const Grid& grid,

          // triangle data (vertex, normal, and color)
          float* triv0, float* triv1, float* triv2,
          float* trin0, float* trin1, float* trin2,
          float* tric0, float* tric1, float* tric2,

          const float3& pointLight,

          // output image
          int imageW, int imageH, float* imageBuf)
{
   using namespace brook;

   // The following static arrays are only used as temporaries when
   // initializing stream contents.
   static Triangle tridat[2048];
   static ShadingInfo shadinfdat[2048];
   static float4 emptyhits[1024*1024];
   static GridTrilistOffset listoffsetdat[2048];
   static GridTrilist trilistdat[2048];

   float3 gridDim((float) grid.dim.x, (float) grid.dim.y, (float) grid.dim.z);
   int numVoxels = grid.dim.x * grid.dim.y * grid.dim.z;
   int maxIters, maxTris, length, last;
   int ii;

   iter wpos_norm(::brook::__BRTFLOAT2,
                  imageW, imageH, -1, 0.0f, 0.0f, 1.0f, 1.0f, -1);

   // contains vertex info (3 float3's) for a triangle
   stream tris = stream::create<Triangle>(grid.nTris);

   // stores normal and color info (6 float3's) for a triangle
   stream shadinf = stream::create<ShadingInfo>(grid.nTris);

   // trilist is index list of triangles present in each grid voxel.
   // lists for each voxel are dilimited by a negative index value
   stream trilist = stream::create<GridTrilist>(grid.trilistSize);

   // starting position of each voxel's triangle list in trilist
   // stream is stored here
   stream listoffset = stream::create<GridTrilistOffset>(numVoxels);

   stream rays = stream::create<Ray>(imageW, imageH);
   stream raystates = stream::create<RayState>(imageW, imageH);
   stream hits = stream::create<Hit>(imageW, imageH);
   stream candidatehits = stream::create<Hit>(imageW, imageH);
   stream travdatdyn = stream::create<TraversalDataDyn>(imageW, imageH);
   stream travdatstatic = stream::create<TraversalDataStatic>(imageW, imageH);
   stream pixels = stream::create<Pixel>(imageW, imageH);

   // The lack of read-modify-write means we have to ping pong writing
   // between two copies of our dynamic state, which is a big waste of space.
   stream Braystates = stream::create<RayState>(imageW, imageH);
   stream Bhits = stream::create<Hit>(imageW, imageH);
   stream Btravdatdyn = stream::create<TraversalDataDyn>(imageW, imageH);


   /*
    * Fill all the streams:
    */

   for (ii=0; ii< grid.nTris; ii++) {
     // fill in triangle information, first vertex data, then normal and color.
     tridat[ii].v0 = float3(triv0[3*ii+0], triv0[3*ii+1], triv0[3*ii+2]);
     tridat[ii].v1 = float3(triv1[3*ii+0], triv1[3*ii+1], triv1[3*ii+2]);
     tridat[ii].v2 = float3(triv2[3*ii+0], triv2[3*ii+1], triv2[3*ii+2]);
     shadinfdat[ii].n0 = float3(trin0[3*ii+0], trin0[3*ii+1], trin0[3*ii+2]);
     shadinfdat[ii].n1 = float3(trin1[3*ii+0], trin1[3*ii+1], trin1[3*ii+2]);
     shadinfdat[ii].n2 = float3(trin2[3*ii+0], trin2[3*ii+1], trin2[3*ii+2]);
     shadinfdat[ii].c0 = float3(tric0[3*ii+0], tric0[3*ii+1], tric0[3*ii+2]);
     shadinfdat[ii].c1 = float3(tric1[3*ii+0], tric1[3*ii+1], tric1[3*ii+2]);
     shadinfdat[ii].c2 = float3(tric2[3*ii+0], tric2[3*ii+1], tric2[3*ii+2]);
   }
   tris.read(tridat);
   shadinf.read(shadinfdat);

   /*
    * Set .x (which is ray t) to HUGE so that any true hits will have a lower
    * t (i.e. happen closer to the eye position).
    */
   assert(imageW * imageH <= sizeof emptyhits / sizeof emptyhits[0]);
   for (ii=0; ii< imageW * imageH; ii++) {
      emptyhits[ii].x = 999999;
      emptyhits[ii].y = 0;
      emptyhits[ii].z = 0;
      emptyhits[ii].w = -1;
   }
   hits.read(emptyhits);

   for (ii=0; ii< numVoxels; ii++) {
      listoffsetdat[ii].listoffset = (float) grid.trilistOffset[ii];
   }
   listoffset.read(listoffsetdat);

   for (ii=0; ii < grid.trilistSize; ii++) {
      trilistdat[ii].trinum = (float) grid.trilist[ii];
   }
   trilist.read(trilistdat);


   /*
    * Determine the maximum number of steps required.  The worst case is we
    * step through the maximal number of voxels (i.e. a diagonal line) and
    * intersect with each triangle along the way.  This is bounded by the sum
    * of the dimensions of the grid multiplied by the maximum number of
    * triangles in any voxel.
    */

   for (maxTris = 0, last = grid.trilistOffset[0], ii=1; ii <  numVoxels; ii++) {
      /* Subtract off 1 for the space occupied by the sentinel */
      length = grid.trilistOffset[ii] - last - 1;
      last = grid.trilistOffset[ii];
      if (length > maxTris) {
         maxTris = length;
      }
   }
   /* Don't forget the triangles in the final voxel! */
   if (grid.trilistSize - last - 1 > maxTris) {
      maxTris = grid.trilistSize - last - 1;
   }
   maxIters = (grid.dim.x + grid.dim.y + grid.dim.z) * maxTris;
   fprintf(stderr, "Performing %d iterations\n", maxIters);

   /*
    * The actual ray tracing loop:
    */

   fprintf(stderr, "Generating eye rays\n");
   krnGenEyeRays(lookFrom, cam.u, cam.v, cam.w,
                 float2(cam.tx, cam.ty),
                 float2(2.0f * cam.tx, 2.0f * cam.ty),
                 grid.min, grid.max, wpos_norm, rays);

   fprintf(stderr, "setup traversal\n");
   krnSetupTraversal(rays, grid.min, grid.vsize, gridDim,
                     travdatdyn, travdatstatic, raystates);

   fprintf(stderr, "Traversing and intersecting\n");
   for (ii = 0; ii < maxIters; ii++) { //adjust depending on scene...
      int last = 0, prog;

      if ((prog = 100 * ii / maxIters) != last) {
         printf("\r%3d%% Done.", 100 * ii / maxIters);
         last = prog;
      }

      //fprintf(stderr, "traverse voxel %i\n", ii);
      krnTraverseVoxel(rays, travdatstatic, travdatdyn, raystates,
                       listoffset, gridDim, Btravdatdyn, Braystates);

      krnTraverseVoxel(rays, travdatstatic, Btravdatdyn, Braystates,
                       listoffset, gridDim, travdatdyn, raystates);


      //fprintf(stderr, "intersect triangle %i\n", ii);
      krnIntersectTriangle(rays, tris, raystates, trilist, candidatehits);
      krnValidateIntersection(rays, candidatehits, grid.min, grid.vsize,
                              gridDim, hits, travdatdyn, raystates, trilist,
                              Bhits, Braystates);

      krnIntersectTriangle(rays, tris, Braystates, trilist, candidatehits);
      krnValidateIntersection(rays, candidatehits, grid.min, grid.vsize,
                              gridDim, Bhits, travdatdyn, Braystates, trilist,
                              hits, raystates);
   }
   printf("\r%3d%% Done.\n", 100 * ii / maxIters);

#if 0
   for (ii = 0; ii <  grid.nTris; ii++) {
      fprintf(stderr, "intersect triangle %i\n", ii);
      krnBruteIntersectTriangle(rays, tris, (float)ii, hits,
                                travdatdyn, raystates, hits, raystates);
   }
#endif

   fprintf(stderr, "Shading hits\n");
   krnShadeHits(rays, hits, tris, shadinf, pointLight, raystates, pixels);

   pixels.write(imageBuf);
}


/*
 * CreateCamera --
 *
 *      Helper routine that computes the camera parameters from the scene
 *      and image parameters.
 *
 * Returns:
 *      void.
 */

static void
CreateCamera(Camera *cam, Scene *scene, int imageW, int imageH)
{
   float3 w = float3(scene->lookAt.x - scene->lookFrom.x,
                     scene->lookAt.y - scene->lookFrom.y,
                     scene->lookAt.z - scene->lookFrom.z);

   cam->w = Normalize(w);
   cam->u = Normalize(Cross(scene->up, cam->w));
   cam->v = Normalize(Cross(cam->w, cam->u));
   cam->ty = tanf((M_PI/180.0f*scene->fov) / 2.0f);
   cam->tx = cam->ty * ((float) imageW / imageH);
}


/*
 * main --
 *
 *      Entry point for the raytracer.  Raytraces the specified scene and
 *      writes out the resulting image.
 *
 * Returns:
 *      0 on success
 */

int
main(int argc, char **argv)
{
   Opts opts = {
      /* progName: */   NULL,

      /* sceneName: */  "cbox",
      /* imageFile: */  "out.ppm",
      /* width: */      256,
      /* height: */     256,
   };
   ppmImage *image;
   Camera cam;

   ParseOpts(&argc, argv, &opts);
   image = new ppmImage(opts.width, opts.height);

   ReadVoxFile(opts.scene->fileName, opts.verbose,
               grid, v0, v1, v2, n0, n1, n2, c0, c1, c2);
   PrintGridInfo(&grid, &opts);

   CreateCamera(&cam, opts.scene, image->Width(), image->Height());
   TraceRays(opts.scene->lookFrom, cam, grid,
             (float *) v0, (float *) v1, (float *) v2,
             (float *) n0, (float *) n1, (float *) n2,
             (float *) c0, (float *) c1, (float *) c2,
             opts.scene->pointLight,
             image->Width(), image->Height(), image->Data());

   fprintf(stderr, "Writing image to %s\n", opts.imageFile);
   image->Write(opts.imageFile);
   delete image;
   return 0;
}
