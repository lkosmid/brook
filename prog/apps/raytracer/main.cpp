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

#include "getopt.h"
#include "Types.h"
#include "Bitvector.h"
#include "FileIO.h"
#include "ppmImage.h"


typedef struct Camera {
   Vector3 u, v, w;
   float tx, ty;
} Camera;


typedef struct Scene {
   char *fileName;

   Point3 lookFrom, lookAt;
   Vector3 up;
   float fov;

   Point3 pointLight;
} Scene;

typedef struct Opts {
   char *progName;

   char *sceneName;
   char *imageFile;

   /*
    * Boolean options (always default to false)
    */

   bool verbose;

   /*
    * Extra state that doesn't map directly to commandline args
    */

   Scene *scene;
} Opts;


extern void TraceRays(float lookfromX, float lookfromY, float lookfromZ,
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


/*
 * Dimensions of the image produced --
 *      should initialize from command line, hardcode for now
 */
static int imageW = 256, imageH = 256;


/*
 * Changing per-frame scene parameters --
 *      should initialize from command line, hardcode for now
 */
static Scene glassnerScene = {
   /* fileName: */      "glassner04.vox",

   /* lookFrom: */      Point3(-7.610099f, -339.653137f, 237.733948f),
   /* lookAt: */        Point3(23.224901f, 980.535034f, -321.560577f),
   /* up: */            Vector3(0,0,1),
   /* fov: */           55.0f,

   /* pointLight: */    Point3(-206.808f, -16.2987f, 297.281f),
};
static Scene cboxScene = {
   /* fileName: */      "cornell02.vox",

   /* lookFrom: */      Point3(278.0f,273.0f, -800.0f),
   /* lookAt: */        Point3(278.1f, 273.1f, 0.0f),
   /* up: */            Vector3(0,1,0),
   /* fov: */           35.0f,

   /* pointLight: */    Point3(278.0f, 530.0f, 279.5f),
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
           "  -h, --help\n"
           "            This message\n"
           "  -o, --imageFile\n"
           "            Filename for the image produced (out.ppm)\n"
           "  -s, --scene\n"
           "            Name of the scene to raytrace (cbox or glassner)\n"
           "            (default: cbox)\n"
           "  -v, --verbose\n"
           "            Print extra information while running\n",
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
      {"help",         0, 0, 'h'},
      {"imageFile",    1, 0, 'o'},
      {"scene",        1, 0, 's'},
      {"verbose",      0, 0, 'v'},
      {0, 0, 0, 0}
   };


   opts->progName = argv[0];
   while ((opt = getopt_long(*argc, argv, "ho:s:v",
                             long_options, &opt_index)) != EOF) {
      switch (opt) {
      case 'o': opts->imageFile = optarg;            break;
      case 's': opts->sceneName = optarg;            break;
      case 'v': opts->verbose = true;                break;

      case 'h':
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
 * CreateCamera --
 *
 *      Helper routine that computes the camera parameters from the scene
 *      and image parameters.
 *
 * Returns:
 *      void.
 */

static void
CreateCamera(Camera *cam, Scene *scene)
{
   cam->w = (scene->lookAt - scene->lookFrom).Normalize();
   cam->u = Vector3::Cross(scene->up, cam->w).Normalize();
   cam->v = Vector3::Cross(cam->w, cam->u).Normalize();
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
   };
   ppmImage img(imageW, imageH);
   Camera cam;

   ParseOpts(&argc, argv, &opts);

   ReadVoxFile(opts.scene->fileName, opts.verbose,
               grid, v0, v1, v2, n0, n1, n2, c0, c1, c2);
   PrintGridInfo(&grid, &opts);

   CreateCamera(&cam, opts.scene);
   TraceRays(opts.scene->lookFrom.x,
             opts.scene->lookFrom.y, opts.scene->lookFrom.z,
             cam.u.x, cam.u.y, cam.u.z,
             cam.v.x, cam.v.y, cam.v.z,
             cam.w.x, cam.w.y, cam.w.z,
             cam.tx, cam.ty,
             grid.min.x, grid.min.y, grid.min.z,
             grid.max.x, grid.max.y, grid.max.z,
             (float) grid.dim.x, (float) grid.dim.y, (float) grid.dim.z,
             grid.dim.x * grid.dim.y * grid.dim.z,
             grid.vsize.x, grid.vsize.y, grid.vsize.z,
             grid.trilistOffset, grid.trilist, grid.trilistSize, grid.nTris,
             (float *) v0, (float *) v1, (float *) v2,
             (float *) n0, (float *) n1, (float *) n2,
             (float *) c0, (float *) c1, (float *) c2,
            opts.scene->pointLight.x,
            opts.scene->pointLight.y, opts.scene->pointLight.z,
            img.Data());

   fprintf(stderr, "Writing image to %s\n", opts.imageFile);
   img.Write(opts.imageFile);
   return 0;
}
