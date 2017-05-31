#ifndef _COMMON_H_
#define _COMMON_H_

#include "float.h"
#include <math.h>
#include "brook/VectorDataType.h"


#define ZERO        0 // Flag to fill with zero's
#define RANDOM      1 // fill method of using low order random bits
#define PREVRAND    2 // Adds a random number to the previous value in the matrix for the current value
#define LINEAR      3 // Assigns the numbers linearly
#define RLINEAR     4 // Assigns the numbers in reverse linear
#define ILINEAR     5 // Assigns the numbers in inverse linear
#define POS         6 // Assigns the position
#define LINEAR_INT  7 // Assigns integers linearly
#define RANDOM_INT  8 // Assigns integers randomly
#define CONSTANT    9 // Assigns the bound as the constant value
#define POSX        10 // Assigns only the x location
#define POSY        11 // Assigns only the y location
#define NONZERO_RANDOM 12 // Assigns a non zero random number (float/double)
#define POSP1       13 // Position plus one

// Defaults used for the struct below. They are defined in the cpp.
extern unsigned int DEFAULT_WIDTH;
extern unsigned int DEFAULT_HEIGHT;
extern unsigned int DEFAULT_ITERATIONS;
extern int          DEFAULT_VERBOSE;
extern int          DEFAULT_VERIFY;
extern int          DEFAULT_QUIET;
extern int          DEFAULT_TIMING;
extern unsigned int DEFAULT_SEED;

typedef struct infoRec
{
    /**
     * Flag to specifiy whether to use verbose output.
     * Activated with the -v command line option.
     */
    int Verbose;
    /**
     * Flag to specify whether to do CPU verification.
     * Activiated with -e command line option.
     */
    int Verify;
    /**
     * Flag to specify whether to compare performance versus CPU
     * Activiated with -p command line option.
     */
    int Performance;
    /**
     * Value of the seed to use in the randomization.
     * Defaults to 0, modified with the -s command line option.
     */
    unsigned int Seed;
    /**
      Flag to specify whether to be quiet or not.
      Activated with the -q command line option.
      */
    int Quiet;
    /**
      Flag to specify whether to print timing results or not.
      Activated with the -t command line option.
      */
    int Timing;
    /**
     * Modified via the -y command line parameter.
     * Specifies the height of the input matrix
     */
    unsigned int Height;
    /**
     * Modified via the -x command line parameter.
     * Specifies the width of the input matrix
     */
    unsigned int Width;

    /**
     * Number of iterations to run the kernels
     * Specified with the -i command line option
     */
    unsigned int Iterations;
} Info;

void ParseCommandLine(int argc, char** argv, Info* info);
void Usage(char *name);
int floatcmp(const void *, const void *);
// Single precision float functions
void fill_mat_f(float* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type);
void print_mat_f(const char* title, const char* fmt, const float* array, unsigned int height, unsigned int width);
int compare_mat_f(const float* mat1, const float* mat2, int height, int width);
void copy_mat_f(const float* input, float *output, int ystart, int xstart, int ystop, int xstop, int iw);
float* load_mat_f(const char* filename, unsigned int* height, unsigned int* width);
float* allocate_mat_f(unsigned int height, unsigned int width);
int floatCompare(float, float);
void save_mat_f(const char* filename, float* matrix, unsigned int height, unsigned int width);
void normalize_mat_f(float* matrix, unsigned int height, unsigned int width, float min, float max);

// Single precision float2 functions
void fill_mat_f2(float2* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type);
void print_mat_f2(const char* title, const char* fmt, const float2* array, unsigned int height, unsigned int width);
int compare_mat_f2(const float2* mat1, const float2* mat2, int height, int width);
float2* load_mat_f2(const char* filename, unsigned int* height, unsigned int* width);
float2* allocate_mat_f2(unsigned int height, unsigned int width);
void save_mat_f2(const char* filename, float2* matrix, unsigned int height, unsigned int width);
void normalize_mat_f2(float2* matrix, unsigned int height, unsigned int width, float min, float max);

// Single precision float3 functions
void fill_mat_f3(float3* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type);
void print_mat_f3(const char* title, const char* fmt, const float3* array, unsigned int height, unsigned int width);
int compare_mat_f3(const float3* mat1, const float3* mat2, int height, int width);
float3* load_mat_f3(const char* filename, unsigned int* height, unsigned int* width);
float3* allocate_mat_f3(unsigned int height, unsigned int width);
void save_mat_f3(const char* filename, float3* matrix, unsigned int height, unsigned int width);
void normalize_mat_f3(float3* matrix, unsigned int height, unsigned int width, float min, float max);

// Single precision float4 functions
void fill_mat_f4(struct float4* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type);
void print_mat_f4(const char* title, const char* fmt, const struct float4* array, unsigned int height, unsigned int width);
int compare_mat_f4(const struct float4* mat1, const struct float4* mat2, int height, int width);
float4* load_mat_f4(const char* filename, unsigned int* height, unsigned int* width);
float4* allocate_mat_f4(unsigned int height, unsigned int width);
void save_mat_f4(const char* filename, float4* matrix, unsigned int height, unsigned int width);
void normalize_mat_f4(float4* matrix, unsigned int height, unsigned int width, float min, float max);

//double precision double functions
void fill_mat_d(double* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type);
void print_mat_d(const char* title, const char* fmt, const double* array, unsigned int height, unsigned int width);
int compare_mat_d(const double* mat1, const double* mat2, int height, int width);
void copy_mat_d(const double* input, double *output, int ystart, int xstart, int ystop, int xstop, int iw);
double* load_mat_d(const char* filename, unsigned int* height, unsigned int* width);
double* allocate_mat_d(unsigned int height, unsigned int width);
int doubleCompare(double, double);
void save_mat_d(const char* filename, double* matrix, unsigned int height, unsigned int width);
void normalize_mat_d(float* matrix, unsigned int height, unsigned int width, float min, float max);

//int functions
void fill_mat_i(int* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type);
void print_mat_i(const char* title, const char* fmt, const int* array, unsigned int height, unsigned int width);
int compare_mat_i(const int* mat1, const int* mat2, int height, int width);
void copy_mat_i(const int* input, int *output, int ystart, int xstart, int ystop, int xstop, int iw);
int* load_mat_i(const char* filename, unsigned int* height, unsigned int* width);
int* allocate_mat_i(unsigned int height, unsigned int width);
int intCompare(int, int);
void save_mat_i(const char* filename, int* matrix, unsigned int height, unsigned int width);
void normalize_mat_i(float* matrix, unsigned int height, unsigned int width, float min, float max);

#endif
