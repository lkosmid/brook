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
#include "ppmImage.h"
#include "timer.h"

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
   printf("Raytracing the '%s' scene.\n", opts->sceneName);
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
   printf("Grid: %i triangles, %ix%ix%i voxels, %i trilist size.\n",
          grid->nTris, grid->dim.x, grid->dim.y, grid->dim.z,
          grid->trilistSize);

   if (opts->verbose) {
      int maxLength, length, last, i;


      printf("Triangle List:\n");
      maxLength = length = 0;
      for(i = 0; i < grid->trilistSize; i++) {
         printf("%i ", grid->trilist[i]);
         if (grid->trilist[i] != -1) {
            length++;
         } else {
            printf("Length: %d\n", length);
            if (length > maxLength) {
               maxLength = length;
            }
            length = 0;
         }
      }
      printf("Max length: %d\n", maxLength);

      printf("Triangle List Offsets:\n");
      maxLength = 0;
      last = grid->trilistOffset[0];
      for (i = 1; i < grid->dim.x * grid->dim.y * grid->dim.z; i++) {
         printf("%i ", grid->trilistOffset[i]);

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
      printf("\n");
      printf("Max length: %d\n", maxLength);
   }
}


/*
 * DensestVoxelSize --
 *
 *      Helper routine that computes the number of triangles in the most
 *      densely packed voxel in the grid.
 *
 * Returns:
 *      Maximum number of triangles in any grid cell.
 */

static int
DensestVoxelSize(const Grid& grid)
{
   int numVoxels = grid.dim.x * grid.dim.y * grid.dim.z;
   int maxTris = 0, ii, last;

   for (last = grid.trilistOffset[0], ii=1; ii < numVoxels; ii++) {
      int length;

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
   return maxTris;
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
TraceRays(const float3& lookFrom, const Camera& cam, const Grid& grid,

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
   static Triangle triDat[4096];
   static ShadingInfo shadInfoDat[2048];
   static GridTrilistOffset listOffsetDat[2048];
   static GridTrilist trilistDat[2048];

   float3 gridDim((float) grid.dim.x, (float) grid.dim.y, (float) grid.dim.z);
   int numVoxels = grid.dim.x * grid.dim.y * grid.dim.z;
   int maxIters, lastLive, ii, aa, bb;
   float now, t, iterTime, iterTimeSquare;
   RayState *states;

   /*
    * The eye-ray generator uses this iterator to reflect evenly spaced
    * pixels on the film (image), coordinatized from 1 to -1 (it gets
    * flipped when it passes through the pin-hole, so that's -1 to 1 in
    * object space).
    */
   iter filmPos(::brook::__BRTFLOAT2,
                imageW, imageH, -1, 1.0f, 1.0f, -1.0f, -1.0f, -1);

   // Vertex info (3 float3's) for a triangle
   stream tris = stream::create<Triangle>(grid.nTris);

   // Normal and color info (6 float3's) for a triangle
   stream shadInfo = stream::create<ShadingInfo>(grid.nTris);

   // trilist is index list of triangles present in each grid voxel.
   // lists for each voxel are dilimited by a negative index value
   stream trilist = stream::create<GridTrilist>(grid.trilistSize);

   // starting position of each voxel's triangle list in trilist
   // stream is stored here
   stream listOffset = stream::create<GridTrilistOffset>(numVoxels);

   stream rays = stream::create<Ray>(imageW, imageH);
   stream travDatStatic = stream::create<TraversalDataStatic>(imageW, imageH);
   stream pixels = stream::create<Pixel>(imageW, imageH);

   /*
    * The lack of read-modify-write means we have to ping pong writing
    * between two copies of our dynamic state, which is a big waste of space.
    */
   stream rayStates[] = {
      stream::create<RayState>(imageW, imageH),
      stream::create<RayState>(imageW, imageH),
   };
   stream hits[] = {
      stream::create<Hit>(imageW, imageH),
      stream::create<Hit>(imageW, imageH),
   };
   stream candidateHits = stream::create<Hit>(imageW, imageH);
   stream travDatDyn[] = {
      stream::create<TraversalDataDyn>(imageW, imageH),
      stream::create<TraversalDataDyn>(imageW, imageH),
   };

   Timer_Reset();

   assert(3 * grid.nTris <= sizeof triDat / sizeof triDat[0]);
   for (ii=0; ii < grid.nTris; ii++) {
     // fill in triangle information, first vertex data, then normal and color.
     triDat[ii].v0 = float3(triv0[3*ii+0], triv0[3*ii+1], triv0[3*ii+2]);
     triDat[ii].v1 = float3(triv1[3*ii+0], triv1[3*ii+1], triv1[3*ii+2]);
     triDat[ii].v2 = float3(triv2[3*ii+0], triv2[3*ii+1], triv2[3*ii+2]);
   }
   tris.read(triDat);

   assert(numVoxels <= sizeof listOffsetDat / sizeof listOffsetDat[0]);
   for (ii = 0; ii < numVoxels; ii++) {
      listOffsetDat[ii] = (float) grid.trilistOffset[ii];
   }
   listOffset.read(listOffsetDat);

   assert(grid.trilistSize <= sizeof trilistDat / sizeof trilistDat[0]);
   for (ii = 0; ii < grid.trilistSize; ii++) {
      trilistDat[ii] = (float) grid.trilist[ii];
   }
   trilist.read(trilistDat);

   /*
    * Set .x (which is ray t) to HUGE so that any true hits will have a lower
    * t (i.e. happen closer to the eye position).
    */
   krnFloat4Set(float4(999999, 0, 0, -1), hits[0]);

   now = Timer_GetMS();
   printf("Filling streams took %3.2f seconds\n", now / 1000.0);


   /*
    * In the worst case we step through the maximal number of voxels
    * (which is the sum of the dimensions) and each cell is also the most
    * densely filled cell.
    */

   maxIters = DensestVoxelSize(grid) * (grid.dim.x + grid.dim.y + grid.dim.z);

   /*
    * The actual ray tracing loop:
    */

   printf("Generating eye rays\n");
   krnGenEyeRays(lookFrom, cam.u, cam.v, cam.w, float2(cam.tx, cam.ty),
                 grid.min, grid.max, filmPos, rays);

   printf("Initializing traversal data and ray states\n");
   krnSetupTraversal(rays, grid.min, grid.vsize,
                     gridDim, travDatDyn[0], travDatStatic, rayStates[0]);

   t = Timer_GetMS();
   printf("Setup kernels took %3.2f seconds\n", (t - now)/1000.0);
   now = t;

   printf("Traversing and intersecting (up to %d iterations)\n", maxIters);
   states = new RayState [imageW * imageH];
   lastLive = imageW * imageH;
   printf("%6d Live rays.", lastLive);
   iterTime = iterTimeSquare = 0;
   for (lastLive = -1, aa = 0, bb = 1, ii = 0; ii < maxIters; ii++) {
      int jj, live;

      t = Timer_GetMS();
      krnTraverseVoxel(rays, travDatStatic, travDatDyn[aa], rayStates[0],
                       listOffset, gridDim, travDatDyn[bb], rayStates[1]);
      krnIntersectTriangle(rays, tris, rayStates[1], trilist, candidateHits);
      krnValidateIntersection(rays, candidateHits, grid.min, grid.vsize,
                              gridDim, hits[aa], travDatDyn[bb], rayStates[1],
                              trilist, hits[bb], rayStates[0]);
      aa = bb;
      bb = 1 - bb;

      /*
       * Early termination: determine how many rays are actually still live
       * and bail when all have either left the grid or hit something.
       */

      rayStates[0].write(states);
      for (live = 0, jj = 0; jj < imageW * imageH; jj++) {
         if (states[jj].x > 0 || states[jj].y > 0) {
            live++;
         }
      }
      if (live != lastLive) {
         printf("\r%6d Live rays.", live);
         lastLive = live;

         if (live == 0) break;
      }

      t = Timer_GetMS() - t;
      iterTime += t; iterTimeSquare += t * t;
   }
   t = Timer_GetMS();
   delete [] states;
   printf("\nFinished in %d iterations (%3.2f seconds, %3.2f mean, %3.2f std. dev).\n",
          ii, (t - now) / 1000.0f, iterTime / 1000.0f / ii,
          sqrt(iterTimeSquare) / 1000.0f / ii);
   now = t;

#if 0
   for (ii = 0; ii < grid.nTris; ii++) {
      fprintf(stderr, "intersect triangle %i\n", ii);
      krnBruteIntersectTriangle(rays, tris, (float) ii, hits,
                                travDatDyn, rayStates, hits, rayStates);
   }
#endif

   printf("Shading hits\n");
   for (ii = 0; ii < grid.nTris; ii++) {
      shadInfoDat[ii].n0 = float3(trin0[3*ii+0], trin0[3*ii+1], trin0[3*ii+2]);
      shadInfoDat[ii].n1 = float3(trin1[3*ii+0], trin1[3*ii+1], trin1[3*ii+2]);
      shadInfoDat[ii].n2 = float3(trin2[3*ii+0], trin2[3*ii+1], trin2[3*ii+2]);
      shadInfoDat[ii].c0 = float3(tric0[3*ii+0], tric0[3*ii+1], tric0[3*ii+2]);
      shadInfoDat[ii].c1 = float3(tric1[3*ii+0], tric1[3*ii+1], tric1[3*ii+2]);
      shadInfoDat[ii].c2 = float3(tric2[3*ii+0], tric2[3*ii+1], tric2[3*ii+2]);
   }
   shadInfo.read(shadInfoDat);
   krnShadeHits(rays,
                hits[aa], tris, shadInfo, pointLight, rayStates[0], pixels);
   pixels.write(imageBuf);
   t = Timer_GetMS();
   printf("Shading plus readback took %3.2f seconds\n", (t - now) / 1000.0f);
   printf("TraceRays took %3.2f seconds.\n", t / 1000.0f);
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

   printf("Writing image to %s\n", opts.imageFile);
   image->Write(opts.imageFile);
   delete image;
   return 0;
}
