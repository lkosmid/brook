#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

typedef struct tagBITMAPFILEHEADER {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    unsigned int biSize;
    unsigned int biWidth;
    unsigned int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} RGBQUAD;
static unsigned int padWidth  = 0;
static unsigned int byteWidth = 0;
static unsigned int datasize  = 0;
const short BITMAP_MAGIC_NUMBER = 19778;
const int RGB_BYTE_SIZE = 3;
const unsigned int Offset = 54;

float4* openBMPimage(const char* filename, unsigned int* height, unsigned int* width);
void saveBMPimage(const char* filename, float4* data, unsigned int height, unsigned int width);

unsigned int DEFAULT_WIDTH  = 64;
unsigned int DEFAULT_HEIGHT = 64;
unsigned int DEFAULT_ITERATIONS = 1;
int          DEFAULT_VERBOSE = 0;
int          DEFAULT_VERIFY = 0;
int          DEFAULT_PERFORMANCE = 0;
int          DEFAULT_QUIET = 0;
int          DEFAULT_TIMING = 0;
unsigned int DEFAULT_SEED = 0;

void ParseCommandLine(int argc, char** argv, Info* info)
{
    int x;
    info->Width = DEFAULT_WIDTH;
    info->Height = DEFAULT_HEIGHT;
    info->Iterations = DEFAULT_ITERATIONS;
    info->Verbose = DEFAULT_VERBOSE;
    info->Verify = DEFAULT_VERIFY;
	info->Performance = DEFAULT_PERFORMANCE;
    info->Quiet = DEFAULT_QUIET;
    info->Timing = DEFAULT_TIMING;
    info->Seed = DEFAULT_SEED;

    for (x = 1; x < argc; ++x)
    {
        switch (argv[x][1])
        {
        case 'v':
            info->Verbose = 1;
            break;
        case 'e':
            info->Verify = 1;
            break;
        case 'p':
            info->Performance = 1;
            break;
        case 'q':
            info->Quiet = 1;
            break;
        case 't':
            info->Timing = 1;
            break;
        case 'i':
            if (++x < argc)
            {
                sscanf(argv[x], "%u", &info->Iterations);
            }
            else
            {
                fprintf(stderr, "Error: Invalid argument, %s", argv[x-1]);
                Usage(argv[0]);
                exit(-1);
            }
            break;
        case 'x':
            if (++x < argc)
            {
                sscanf(argv[x], "%u", &info->Width);
            }
            else
            {
                fprintf(stderr, "Error: Invalid argument, %s", argv[x-1]);
                Usage(argv[0]);
                exit(-1);
            }
            break;
        case 'y':
            if (++x < argc)
            {
                sscanf(argv[x], "%u", &info->Height);
            }
            else
            {
                fprintf(stderr, "Error: Invalid argument, %s", argv[x-1]);
                Usage(argv[0]);
                exit(-1);
            }
            break;
        case 's':
            if (++x < argc)
            {
                sscanf(argv[x], "%u", &info->Seed);
            }
            else
            {
                fprintf(stderr, "Error: Invalid argument, %s",argv[x-1]);
                Usage(argv[0]);
                exit(-1);
            }
            break;
        case 'h':
            Usage(argv[0]);
            exit(-1);
            break;
        default:
            fprintf(stderr, "Error: Invalid command line parameter, %c\n", argv[x][1]);
            Usage(argv[0]);
            exit(-1);
        }
    }
    if ((info->Verbose + info->Quiet) == 2)
    {
        printf("Verbose and Quiet cancel each other out.\n");
        info->Verbose = 0;
        info->Quiet = 0;
    }
}
void Usage(char *name)
{
    printf("\tUsage: %s [-e] [-v] [-h] [-s <int>]\n", name);
    printf("   -h       Print this help menu.\n");
    printf("   -v       Print verbose output.\n");
    printf("   -e       Verify correct output.\n");
    printf("   -p       Compare performance with CPU.\n");
    printf("   -q       Surpress all data output.\n");
    printf("   -s <int> Set the randomization seed.\n");
    printf("   -t       Print out timing information.\n");
    printf("   -x <int> Sets the matrix width.\n");
    printf("   -y <int> Sets the matrix height.\n");
    printf("   -i <int> Number of iterations.\n");

}

float value(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int bound, unsigned int type)
{
    switch(type)
    {
    case RANDOM:
        return (rand() / ((float)RAND_MAX + 1)) * bound;
    case NONZERO_RANDOM:
        if (bound != 0)
            return ((rand()+1)/ ((float)RAND_MAX + 2)) * bound;
        else
            return ((rand()+1)/ ((float)RAND_MAX + 2)) * 1;
    case PREVRAND:
        return (rand() / ((float)RAND_MAX + 1)) * bound;
    case LINEAR:
        return (float)((y * width + x) % bound);
    case RLINEAR:
        return (float)(((width * height) - (y * width + x)) % bound);
    case ILINEAR:
        return (float)((x * width + y) % bound);
    case POS:
        return (float)(y * width + x);
    case LINEAR_INT:
        return ceil((float)(y * width + x));
    case RANDOM_INT:
        return ceil((rand() / ((float)RAND_MAX + 1)) * bound);
    case CONSTANT:
        return (float)(bound);
    case POSX:
        return (float)(x);
    case POSY:
        return (float)(y);
    case POSP1:
        return (float)(y * width + x + 1);
    }
    return 0.0f;
}

double value_double(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int bound, unsigned int type)
{
    switch(type)
    {
    case RANDOM:
        return (rand() / ((double)RAND_MAX + 1)) * bound;
    case NONZERO_RANDOM:
        if (bound != 0)
            return ((rand()+1)/ ((double)RAND_MAX + 2)) * bound;
        else
            return ((rand()+1)/ ((double)RAND_MAX + 2)) * 1;
    case PREVRAND:
        return (rand() / ((double)RAND_MAX + 1)) * bound;
    case LINEAR:
        return (double)((y * width + x) % bound);
    case RLINEAR:
        return (double)(((width * height) - (y * width + x)) % bound);
    case ILINEAR:
        return (double)((x * width + y) % bound);
    case POS:
        return (double)(y * width + x);
    case LINEAR_INT:
        return ceil((double)(y * width + x));
    case RANDOM_INT:
        return ceil((rand() / ((double)RAND_MAX + 1)) * bound);
    case CONSTANT:
        return (double)(bound);
    case POSX:
        return (double)(x);
    case POSY:
        return (float)(y);
    case POSP1:
        return (double)(y * width + x + 1);
    }
    return 0.0f;
}

int value_int(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int bound, unsigned int type)
{
    switch(type)
    {
    case RANDOM:
        return (int)((rand() / ((float)RAND_MAX + 1)) * bound);
    case NONZERO_RANDOM:
        return (int)((rand() / ((float)RAND_MAX + 1)) * bound) + 1;
    case PREVRAND:
        return (int)((rand() / ((float)RAND_MAX + 1)) * bound);
    case LINEAR:
        return ((y * width + x) % bound);
    case RLINEAR:
        return (((width * height) - (y * width + x)) % bound);
    case ILINEAR:
        return ((x * width + y) % bound);
    case POS:
        return (y * width + x);
    case LINEAR_INT:
        return (y * width + x);
    case RANDOM_INT:
        return (int)((rand() / ((float)RAND_MAX + 1)) * bound);
    case CONSTANT:
        return bound;
    case POSX:
        return x;
    case POSY:
        return y;
    case POSP1:
        return (y * width + x + 1);
    }
    return 0;
}

void fill_mat_f(float* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type)
{
    unsigned int y = 0;
    for (y = 0; y < height; ++y)
    {
        unsigned int x = 0;
        for (x = 0; x < width; ++x)
        {
            if (type == PREVRAND)
            {
                int index = y * width + x;
                if (index)
                {
                    dst[index] = dst[index - 1] + value(x, y, width, height, bound, type);
                }
                else
                {
                    dst[index] = value(x, y, width, height, bound, type);
                }
            }
            else
            {
                dst[y * width + x] = value(x, y, width, height, bound, type);
            }
        }
    }
}

void print_mat_f(const char* title, const char* fmt, const float* array, unsigned int height, unsigned int width)
{
    unsigned int y = 0;
    unsigned int x = 0;
    printf("%s\n",title);
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            printf(fmt, array[y * width + x]);
        }
        printf("\n");
    }
    printf("\n");
}

int compare_mat_f(const float* mat1, const float* mat2, int height, int width)
{
    int wrong = 0;
    int y = 0;
    for (y = 0; y < height; ++y)
    {
        int x;
        for (x = 0; x < width; ++x)
        {
            int index = y * width + x;
            if (floatCompare(mat1[index], mat2[index]))
            {
                ++wrong;
#ifdef DEBUG_CORRECTNESS
                printf("@%d: %f / %f\n", index, mat1[index], mat2[index]);
#endif
            }
        }
    }
    return wrong;
}

void copy_mat_f(const float* input, float *output, int ystart, int xstart, int ystop, int xstop, int iw)
{
    int y = 0;
    int i = 0;
    for (i = ystart; i < ystop; ++i)
    {
        int j = 0;
        for (j = xstart; j < xstop; ++j, ++y)
        {
            output[y] = input[i * iw + j];
        }
    }
}

inline int isnan_flt(float x)
{
#ifdef _WIN32
    return _isnan(x);
#else
    return isnan(x);
#endif
}

int floatCompare(float val1, float val2)
{
    if (isnan_flt(val1) || isnan_flt(val2))
    {
        return 1;
    }
    float epsilon = (float)1e-2;
    
	if (fabs(val1)  > epsilon)
    {
        float re = (val2 - val1) / val1;
        return (fabs(re) > epsilon);
    }
    else
    {
        return (fabs(val2 - val1) > epsilon);
    }
    
}

int floatcmp(const void *a, const void *b)
{
    const float c = *(const float*)a;
    const float d = *(const float*)b;
    if (c > d) {
        return 1;
    } else if (c < d) {
        return -1;
    } else {
        return 0;
    }
}

float* load_mat_f(const char* filename, unsigned int* height, unsigned int* width)
{
    printf("Loading into float struct not yet supported!\n");
    return NULL;
}

float* allocate_mat_f(unsigned int height, unsigned int width)
{
    float* ptr = (float*)malloc(sizeof *ptr * height * width);
    if (ptr)
    {
        memset(ptr, 0, sizeof *ptr * height * width);
    }
    return ptr;
}

void save_mat_f(const char* filename, float* matrix, unsigned int height, unsigned int width)
{
    float4 *data = NULL;
    unsigned int y, x;
    if (!(data = allocate_mat_f4(height, width)))
    {
        return;
    }
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            unsigned int index = y * width + x;
            data[index].x = matrix[index];
            data[index].y = matrix[index];
            data[index].z = matrix[index];
            data[index].w = matrix[index];
        }
    }
    saveBMPimage(filename, data, height, width);
    free(data);
}

void normalize_mat_f(float* matrix, unsigned int height, unsigned int width, float min, float max)
{
    float tmin = (float)INT_MAX;
    float tmax = (float)INT_MIN;
    unsigned int size = height * width;
    float *ptr = matrix;
    do {
        tmax = (*ptr > tmax) ? *ptr : tmax;
        tmin = (*ptr < tmin) ? *ptr : tmin;
    }while(++ptr < (matrix + size));
    ptr = matrix;
    do {
        *ptr = floor((((*ptr - tmin) * (max - min)) / (tmax - tmin)) + min);
    }while(++ptr < (matrix + size));
}


void fill_mat_f2(float2* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type)
{
    unsigned int y = 0;
    for (y = 0; y < height; ++y)
    {
        unsigned int x = 0;
        for (x = 0; x < width; ++x)
        {
            if (type == POS)
            {
                dst[y * width + x].x = (float)x;
                dst[y * width + x].y = (float)y;
            }
            else if (type == PREVRAND)
            {
                int index = y * width + x;
                if (index)
                {
                    dst[index].x = dst[index - 1].x + value(x, y, width, height, bound, type);
                    dst[index].y = dst[index - 1].y + value(x, y, width, height, bound, type);
                }
                else
                {
                    dst[index].x = value(x, y, width, height, bound, type);
                    dst[index].y = value(x, y, width, height, bound, type);
                }
            }
            else
            {
                dst[y * width + x].x = value(x, y, width, height, bound, type);
                dst[y * width + x].y = value(x, y, width, height, bound, type);
            }
        }
    }
}

void print_mat_f2(const char* title, const char* fmt, const float2* array, unsigned int height, unsigned int width)
{
    unsigned int y = 0;
    unsigned int x = 0;
    printf("%s\n",title);
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            printf("{ ");
            printf(fmt, array[y * width + x].x);
            printf(", ");
            printf(fmt, array[y * width + x].y);
            printf("} ");
        }
        printf("\n");
    }
    printf("\n");
}

int compare_mat_f2(const float2* mat1, const float2* mat2, int height, int width)
{
    int wrong = 0;
    int y = 0;
    for (y = 0; y < height; ++y)
    {
        int x;
        for (x = 0; x < width; ++x)
        {
            int index = y * width + x;
            if (floatCompare(mat1[index].x, mat2[index].x))
            {
                ++wrong;
            }
            if (floatCompare(mat1[index].y, mat2[index].y))
            {
                ++wrong;
            }
        }
    }
    return wrong;
}
float2* load_mat_f2(const char* filename, unsigned int* height, unsigned int* width)
{
    printf("Loading into float2 struct not yet supported!\n");
    return NULL;
}

float2* allocate_mat_f2(unsigned int height, unsigned int width)
{
    float2* ptr = (float2*)malloc(sizeof *ptr * height * width);
    if (ptr)
    {
        memset(ptr, 0, sizeof *ptr * height * width);
    }
    return ptr;
}
void save_mat_f2(const char* filename, float2* matrix, unsigned int height, unsigned int width)
{
    printf("saving a float2 struct not yet supported!\n");
}

void fill_mat_f3(float3* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type)
{
    unsigned int y = 0;
    for (y = 0; y < height; ++y)
    {
        unsigned int x = 0;
        for (x = 0; x < width; ++x)
        {
            if (type == POS)
            {
                dst[y * width + x].x = (float)x;
                dst[y * width + x].y = (float)y;
                dst[y * width + x].z = 1.f;
            }
            else if (type == PREVRAND)
            {
                int index = y * width + x;
                if (index)
                {
                    dst[index].x = dst[index - 1].x + value(x, y, width, height, bound, type);
                    dst[index].y = dst[index - 1].y + value(x, y, width, height, bound, type);
                    dst[index].z = dst[index - 1].z + value(x, y, width, height, bound, type);
                }
                else
                {
                    dst[index].x = value(x, y, width, height, bound, type);
                    dst[index].y = value(x, y, width, height, bound, type);
                    dst[index].z = value(x, y, width, height, bound, type);
                }
            }
            else
            {
                dst[y * width + x].x = value(x, y, width, height, bound, type);
                dst[y * width + x].y = value(x, y, width, height, bound, type);
                dst[y * width + x].z = value(x, y, width, height, bound, type);
            }
        }
    }
}

void print_mat_f3(const char* title, const char* fmt, const float3* array, unsigned int height, unsigned int width)
{
    unsigned int y = 0;
    unsigned int x = 0;
    printf("%s\n",title);
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            printf("{ ");
            printf(fmt, array[y * width + x].x);
            printf(", ");
            printf(fmt, array[y * width + x].y);
            printf(", ");
            printf(fmt, array[y * width + x].z);
            printf("} ");
        }
        printf("\n");
    }
    printf("\n");
}

int compare_mat_f3(const float3* mat1, const float3* mat2, int height, int width)
{
    int wrong = 0;
    int y = 0;
    for (y = 0; y < height; ++y)
    {
        int x;
        for (x = 0; x < width; ++x)
        {
            int index = y * width + x;
            if (floatCompare(mat1[index].x, mat2[index].x))
            {
                ++wrong;
            }
            if (floatCompare(mat1[index].y, mat2[index].y))
            {
                ++wrong;
            }
            if (floatCompare(mat1[index].z, mat2[index].z))
            {
                ++wrong;
            }
        }
    }
    return wrong;
}
float3* load_mat_f3(const char* filename, unsigned int* height, unsigned int* width)
{
    printf("Loading into float3 struct not yet supported!\n");
    return NULL;
}

float3* allocate_mat_f3(unsigned int height, unsigned int width)
{
    float3* ptr = (float3*)malloc(sizeof *ptr * height * width);
    if (ptr)
    {
        memset(ptr, 0, sizeof *ptr * height * width);
    }
    return ptr;
}
void save_mat_f3(const char* filename, float3* matrix, unsigned int height, unsigned int width)
{
    printf("saving a float3 struct not yet supported!\n");
}


void fill_mat_f4(float4* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type)
{
    unsigned int y = 0;
    for (y = 0; y < height; ++y)
    {
        unsigned int x = 0;
        for (x = 0; x < width; ++x)
        {
            if(type == POS)
            {
                dst[y * width + x].x = (float)x;
                dst[y * width + x].y = (float)y;
                dst[y * width + x].z = value(x, y, width, height, bound, type);
                dst[y * width + x].w = value(x, y, width, height, bound, type);
            }
            else if (type == PREVRAND)
            {
                int index = y * width + x;
                if (index)
                {
                    dst[index].x = dst[index - 1].x + value(x, y, width, height, bound, type);
                    dst[index].y = dst[index - 1].y + value(x, y, width, height, bound, type);
                    dst[index].z = dst[index - 1].z + value(x, y, width, height, bound, type);
                    dst[index].w = dst[index - 1].w + value(x, y, width, height, bound, type);
                }
                else
                {
                    dst[index].x = value(x, y, width, height, bound, type);
                    dst[index].y = value(x, y, width, height, bound, type);
                    dst[index].z = value(x, y, width, height, bound, type);
                    dst[index].w = value(x, y, width, height, bound, type);
                }
            }
            else
            {
                dst[y * width + x].x = value(x, y, width, height, bound, type);
                dst[y * width + x].y = value(x, y, width, height, bound, type);
                dst[y * width + x].z = value(x, y, width, height, bound, type);
                dst[y * width + x].w = value(x, y, width, height, bound, type);
            }
        }
    }
}
void print_mat_f4(const char* title, const char* fmt, const struct float4* array, unsigned int height, unsigned int width)
{
    unsigned int y = 0;
    unsigned int x = 0;
    printf("%s\n",title);
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            printf("{ ");
            printf(fmt, array[y * width + x].x);
            printf(", ");
            printf(fmt, array[y * width + x].y);
            printf(", ");
            printf(fmt, array[y * width + x].z);
            printf(", ");
            printf(fmt, array[y * width + x].w);
            printf("} ");
        }
        printf("\n");
    }
    printf("\n");
}
int compare_mat_f4(const struct float4* mat1, const struct float4* mat2, int height, int width)
{
    int wrong = 0;
    int y = 0;
    for (y = 0; y < height; ++y)
    {
        int x;
        for (x = 0; x < width; ++x)
        {
            int index = y * width + x;
            if (floatCompare(mat1[index].x, mat2[index].x))
            {
                ++wrong;
#ifdef DEBUG_CORRECTNESS
                printf("@%d.x: %f / %f\n", index, mat1[index].x, mat2[index].x);
#endif
            }
            if (floatCompare(mat1[index].y, mat2[index].y))
            {
                ++wrong;
#ifdef DEBUG_CORRECTNESS
                printf("@%d.y: %f / %f\n", index, mat1[index].y, mat2[index].y);
#endif
            }
            if (floatCompare(mat1[index].z, mat2[index].z))
            {
                ++wrong;
#ifdef DEBUG_CORRECTNESS
                printf("@%d.z: %f / %f\n", index, mat1[index].z, mat2[index].z);
#endif
            }
            if (floatCompare(mat1[index].w, mat2[index].w))
            {
                ++wrong;
#ifdef DEBUG_CORRECTNESS
                printf("@%d.w: %f / %f\n", index, mat1[index].w, mat2[index].w);
#endif
            }
        }
    }
    return wrong;
}
float4* load_mat_f4(const char* filename, unsigned int* height, unsigned int* width)
{
    return openBMPimage(filename, height, width);
}

float4* allocate_mat_f4(unsigned int height, unsigned int width)
{
    float4* ptr = (float4*)malloc(sizeof *ptr * height * width);
    if (ptr)
    {
        memset(ptr, 0, sizeof *ptr * height * width);
    }
    return ptr;
}

void save_mat_f4(const char* filename, float4* matrix, unsigned int height, unsigned int width)
{
    saveBMPimage(filename, matrix, height, width);
}

void normalize_mat_f4(float4* matrix, unsigned int height, unsigned int width, float min, float max)
{
    float tmin = (float)INT_MAX;
    float tmax = (float)INT_MIN;
    unsigned int size = height * width;
    float4 *ptr = matrix;
    do {
        tmax = (ptr->x > tmax) ? ptr->x : tmax;
        tmin = (ptr->x < tmin) ? ptr->x : tmin;
        tmax = (ptr->y > tmax) ? ptr->y : tmax;
        tmin = (ptr->y < tmin) ? ptr->y : tmin;
        tmax = (ptr->z > tmax) ? ptr->z : tmax;
        tmin = (ptr->z < tmin) ? ptr->z : tmin;
        tmax = (ptr->w > tmax) ? ptr->w : tmax;
        tmin = (ptr->w < tmin) ? ptr->w : tmin;
    }while(++ptr < (matrix + size));
    ptr = matrix;
    do {
        ptr->x = ((((ptr->x - tmin) * (max - min)) / (tmax - tmin)) + min);
        ptr->y = ((((ptr->y - tmin) * (max - min)) / (tmax - tmin)) + min);
        ptr->z = ((((ptr->z - tmin) * (max - min)) / (tmax - tmin)) + min);
        ptr->w = ((((ptr->w - tmin) * (max - min)) / (tmax - tmin)) + min);
    }while(++ptr < (matrix + size));
}

void fill_mat_d(double* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type)
{
    unsigned int y = 0;
    for (y = 0; y < height; ++y)
    {
        unsigned int x = 0;
        for (x = 0; x < width; ++x)
        {
            if (type == PREVRAND)
            {
                int index = y * width + x;
                if (index)
                {
                    dst[index] = dst[index - 1] + value_double(x, y, width, height, bound, type);
                }
                else
                {
                    dst[index] = value_double(x, y, width, height, bound, type);
                }
            }
            else
            {
                dst[y * width + x] = value_double(x, y, width, height, bound, type);
            }
        }
    }
}

void print_mat_d(const char* title, const char* fmt, const double* array, unsigned int height, unsigned int width)
{
    unsigned int y = 0;
    unsigned int x = 0;
    printf("%s\n",title);
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            printf(fmt, array[y * width + x]);
        }
        printf("\n");
    }
    printf("\n");
}

int compare_mat_d(const double* mat1, const double* mat2, int height, int width)
{
    int wrong = 0;
    int y = 0;
    for (y = 0; y < height; ++y)
    {
        int x;
        for (x = 0; x < width; ++x)
        {
            int index = y * width + x;
            if (doubleCompare(mat1[index], mat2[index]))
            {
                ++wrong;
            }
        }
    }
    return wrong;
}

void copy_mat_d(const double* input, double *output, int ystart, int xstart, int ystop, int xstop, int iw)
{
    int y = 0;
    int i = 0;
    for (i = ystart; i < ystop; ++i)
    {
        int j = 0;
        for (j = xstart; j < xstop; ++j, ++y)
        {
            output[y] = input[i * iw + j];
        }
    }
}


int doubleCompare(double val1, double val2)
{
    double epsilon = (double)1e-2;
    
        if (fabs(val1)  > epsilon)
        {
            double re = (val2 - val1) / val1;
            return (fabs(re) > epsilon);
        }
        else
        {
            return (fabs(val2 - val1) > epsilon);
        }
}

int doublecmp(const void *a, const void *b)
{
    const double c = *(const double*)a;
    const double d = *(const double*)b;
    if (c > d) {
        return 1;
    } else if (c < d) {
        return -1;
    } else {
        return 0;
    }
}

double* load_mat_d(const char* filename, unsigned int* height, unsigned int* width)
{
    printf("Loading into double struct not yet supported!\n");
    return NULL;
}

double* allocate_mat_d(unsigned int height, unsigned int width)
{
    double* ptr = (double*)malloc(sizeof *ptr * height * width);
    if (ptr)
    {
        memset(ptr, 0, sizeof *ptr * height * width);
    }
    return ptr;
}

void save_mat_d(const char* filename, double* matrix, unsigned int height, unsigned int width)
{
    float4 *data = NULL;
    unsigned int y, x;
    if (!(data = allocate_mat_f4(height, width)))
    {
        return;
    }
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            unsigned int index = y * width + x;
            data[index].x = (float)matrix[index];
            data[index].y = (float)matrix[index];
            data[index].z = (float)matrix[index];
            data[index].w = (float)matrix[index];
        }
    }
    saveBMPimage(filename, data, height, width);
    free(data);
}
void normalize_mat_d(double* matrix, unsigned int height, unsigned int width, double min, double max)
{
    double tmin = (double)INT_MAX;
    double tmax = (double)INT_MIN;
    unsigned int size = height * width;
    double *ptr = matrix;
    do {
        tmax = (*ptr > tmax) ? *ptr : tmax;
        tmin = (*ptr < tmin) ? *ptr : tmin;
    }while(++ptr < (matrix + size));
    ptr = matrix;
    do {
        *ptr = floor((((*ptr - tmin) * (max - min)) / (tmax - tmin)) + min);
    }while(++ptr < (matrix + size));
}

void fill_mat_i(int* dst, unsigned int height, unsigned int width, unsigned int bound, unsigned int type)
{
    unsigned int y = 0;
    for (y = 0; y < height; ++y)
    {
        unsigned int x = 0;
        for (x = 0; x < width; ++x)
        {
            if (type == PREVRAND)
            {
                int index = y * width + x;
                if (index)
                {
                    dst[index] = dst[index - 1] + value_int(x, y, width, height, bound, type);
                }
                else
                {
                    dst[index] = value_int(x, y, width, height, bound, type);
                }
            }
            else
            {
                dst[y * width + x] = value_int(x, y, width, height, bound, type);
            }
        }
    }
}

void print_mat_i(const char* title, const char* fmt, const int* array, unsigned int height, unsigned int width)
{
    unsigned int y = 0;
    unsigned int x = 0;
    printf("%s\n",title);
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            printf(fmt, array[y * width + x]);
        }
        printf("\n");
    }
    printf("\n");
}

int compare_mat_i(const int* mat1, const int* mat2, int height, int width)
{
    int wrong = 0;
    int y = 0;
    for (y = 0; y < height; ++y)
    {
        int x;
        for (x = 0; x < width; ++x)
        {
            int index = y * width + x;
            if (intCompare(mat1[index], mat2[index]))
            {
                ++wrong;
            }
        }
    }
    return wrong;
}

void copy_mat_i(const int* input, int *output, int ystart, int xstart, int ystop, int xstop, int iw)
{
    int y = 0;
    int i = 0;
    for (i = ystart; i < ystop; ++i)
    {
        int j = 0;
        for (j = xstart; j < xstop; ++j, ++y)
        {
            output[y] = input[i * iw + j];
        }
    }
}


int intCompare(int val1, int val2)
{
    double epsilon = (double)1e-2;
    if (abs(val1)  > epsilon)
    {
        double re = (val2 - val1) / val1;
        return (re > epsilon);
    }
    else
    {
        return ((val2 - val1) > epsilon);
    }

}

int intcmp(const void *a, const void *b)
{
    const int c = *(const int*)a;
    const int d = *(const int*)b;
    if (c > d) {
        return 1;
    } else if (c < d) {
        return -1;
    } else {
        return 0;
    }
}

int* load_mat_i(const char* filename, unsigned int* height, unsigned int* width)
{
    printf("Loading into int struct not yet supported!\n");
    return NULL;
}

int* allocate_mat_i(unsigned int height, unsigned int width)
{
    int* ptr = (int*)malloc(sizeof *ptr * height * width);
    if (ptr)
    {
        memset(ptr, 0, sizeof *ptr * height * width);
    }
    return ptr;
}

void save_mat_i(const char* filename, int* matrix, unsigned int height, unsigned int width)
{
    float4 *data = NULL;
    unsigned int y, x;
    if (!(data = allocate_mat_f4(height, width)))
    {
        return;
    }
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            unsigned int index = y * width + x;
            data[index].x = (float)matrix[index];
            data[index].y = (float)matrix[index];
            data[index].z = (float)matrix[index];
            data[index].w = (float)matrix[index];
        }
    }
    saveBMPimage(filename, data, height, width);
    free(data);
}
void normalize_mat_i(int* matrix, unsigned int height, unsigned int width, int min, int max)
{
    int tmin = (int)INT_MAX;
    int tmax = (int)INT_MIN;
    unsigned int size = height * width;
    int *ptr = matrix;
    do {
        tmax = (*ptr > tmax) ? *ptr : tmax;
        tmin = (*ptr < tmin) ? *ptr : tmin;
    }while(++ptr < (matrix + size));
    ptr = matrix;
    do {
        *ptr = ((((*ptr - tmin) * (max - min)) / (tmax - tmin)) + min);
    }while(++ptr < (matrix + size));
}


float4* convert8(unsigned char* data, unsigned int height, unsigned int width, RGBQUAD* colors);
float4* convert24(unsigned char* data, unsigned int height, unsigned int width);
void saveBMPimage(const char* filename, float4* matrix, unsigned int height, unsigned int width)
{
    BITMAPFILEHEADER bmfh = {0, 0, 0, 0, 0};
    BITMAPINFOHEADER bmih = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int  offset, diff;
    FILE *fid = NULL;
    bmih.biWidth = width;
    bmih.biHeight = height;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biSize = 40;
    bmih.biXPelsPerMeter = 2835;
    bmih.biYPelsPerMeter = 2835;
    bmfh.bfType = BITMAP_MAGIC_NUMBER;
    bmfh.bfOffBits = Offset;
    bmfh.bfSize = (width * height * 3) + 40 + 14;
    if (!(fid = fopen(filename, "wb")))
    {
        printf("Error creating file: %s\n", filename);
        return;
    }
    fwrite(&bmfh.bfType, 2, 1, fid);
    fwrite(&bmfh.bfSize, 4, 1, fid);
    fwrite(&bmfh.bfReserved1, 2, 1, fid);
    fwrite(&bmfh.bfReserved2, 2, 1, fid);
    fwrite(&bmfh.bfOffBits, 4, 1, fid);
    fwrite(&bmih, sizeof(bmih), 1, fid);
    diff = width * height * RGB_BYTE_SIZE;
    byteWidth = padWidth = (width * bmih.biBitCount) >> 3;
    datasize = (byteWidth * height);
    normalize_mat_f4(matrix, height, width, 0.0, 255.0);
    while((padWidth % 4) != 0) padWidth++;
    if (height > 0)
    {
        unsigned int j = 0;
        offset = padWidth - byteWidth;
        for (unsigned int i = 0; i < datasize; i += 3)
        {
            if (((i + 1) % padWidth) == 0)
            {
                i += offset;
            }
            fprintf(fid,"%c",(char)((int)(matrix[j].z)));
            fprintf(fid,"%c",(char)((int)(matrix[j].y)));
            fprintf(fid,"%c",(char)((int)(matrix[j].x)));
            ++j;
        }
    }
    else
    {
        unsigned int j = diff / 3;
        offset = padWidth - byteWidth;
        for (unsigned int i = 0; i < datasize; i += 3)
        {
            if (((i + 1) % padWidth) == 0)
            {
                i += offset;
            }
            fprintf(fid,"%c",(char)((int)(matrix[j].z)));
            fprintf(fid,"%c",(char)((int)(matrix[j].y)));
            fprintf(fid,"%c",(char)((int)(matrix[j].x)));
            --j;
        }
    }
    fclose(fid);
}
float4* openBMPimage(const char* filename, unsigned int* height, unsigned int* width)
{
    float4* image   = NULL;
    FILE *fid       = NULL;
    unsigned char *tempData  = NULL;
    RGBQUAD* colors = NULL;
    unsigned int numColors = 0;
    BITMAPFILEHEADER bmfh = {0, 0, 0, 0, 0};
    BITMAPINFOHEADER bmih = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if (!(fid = fopen(filename, "rb")))
    {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    fread(&bmfh.bfType, 2, 1, fid);
    fread(&bmfh.bfSize, 4, 1, fid);
    fread(&bmfh.bfReserved1, 2, 1, fid);
    fread(&bmfh.bfReserved2, 2, 1, fid);
    fread(&bmfh.bfOffBits, 4, 1, fid);
    if (bmfh.bfType != BITMAP_MAGIC_NUMBER)
    {
        printf("File is not in DIB format\n");
        goto exit;
    }
    fread(&bmih, sizeof(bmih), 1, fid);
    *height = bmih.biHeight;
    *width = bmih.biWidth;
    numColors = 1 << bmih.biBitCount;
    datasize = *width * *height * (bmih.biBitCount >> 3);
    if (bmih.biBitCount == 8)
    {
        colors = (RGBQUAD*)malloc(sizeof*colors * numColors);
        if (!(colors))
        {
            printf("Error allocating space for color data\n");
            goto exit;
        }
        fread(colors, sizeof*colors, numColors, fid);
    }

    if (!(tempData = (unsigned char*)malloc(sizeof *tempData * datasize)))
    {
        printf("Error allocating temp data space\n");
        goto exit;
    }

    fread(tempData, sizeof *tempData, datasize, fid);

    byteWidth = padWidth = (*width * bmih.biBitCount) >> 3;

    while((padWidth % 4) != 0) padWidth++;
    switch(bmih.biBitCount)
    {
    case 8:
        image = convert8(tempData, *height, *width, colors);
        break;
    case 16:
        printf("Does not support 16 bit images yet.\n");
        break;
    case 24:
        image = convert24(tempData, *height, *width);
        break;
    case 32:
        printf("Does not support 32 bit images yet.\n");
        break;
    }

exit:
    if (tempData)
    {
        free(tempData);
    }
    if (colors)
    {
        free(colors);
    }
    if (fid)
    {
        fclose(fid);
    }
    return image;
}

float4* convert8(unsigned char* data, unsigned int height, unsigned int width, RGBQUAD* colors)
{
    float4* image;
    unsigned int offset, diff;
    diff = width * height * RGB_BYTE_SIZE;
    if (!(image = allocate_mat_f4(height, width)))
    {
        printf("Unable to allocate image matrix data.\n");
        return NULL;
    }

    if (height > 0)
    {
        unsigned int j = 0;
        offset = padWidth - byteWidth;
        for (unsigned int i = 0; i < datasize * RGB_BYTE_SIZE; i += 3)
        {
            if ((i + 1) % padWidth == 0)
            {
                i += offset;
            }
            (*(image + i)).x = (float)colors[*(data + j)].rgbRed;
            (*(image + i)).y = (float)colors[*(data + j)].rgbGreen;
            (*(image + i)).z = (float)colors[*(data + j)].rgbBlue;
            j++;
        }
    }
    else
    {
        unsigned int j = datasize - 1;
        offset = padWidth - byteWidth;
        for (unsigned int i = 0; i < datasize * RGB_BYTE_SIZE; i += 3)
        {
            if ((i + 1) % padWidth == 0)
            {
                i += offset;
            }
            (*(image + i)).x = (float)colors[*(data + j)].rgbRed;
            (*(image + i)).y = (float)colors[*(data + j)].rgbGreen;
            (*(image + i)).z = (float)colors[*(data + j)].rgbBlue;
            j--;
        }
    }
    normalize_mat_f4(image, height, width, 0.0, 1.0);
    return image;
}

float4* convert24(unsigned char* data, unsigned int height, unsigned int width)
{
    float4* image;
    unsigned int offset, diff;
    diff = width * height * RGB_BYTE_SIZE;
    if (!(image = allocate_mat_f4(height, width)))
    {
        printf("Unable to allocate image matrix data.\n");
        return NULL;
    }
    if (height > 0)
    {
        int j = 0;
        offset = padWidth - byteWidth;
        for (unsigned int i = 0; i < datasize; i += 3)
        {
            if (((i + 1) % padWidth) == 0)
            {
                i += offset;
            }
            (*(image + j)).x = (*(data + i + 2));
            (*(image + j)).y = (*(data + i + 1));
            (*(image + j)).z = (*(data + i + 0));
            ++j;
        }
    }
    else
    {
        int j;
        offset = padWidth - byteWidth;
        j = datasize / 3;
        for (unsigned int i = 0; i < datasize; i += 3)
        {
            if (((i + 1) % padWidth) == 0)
            {
                i += offset;
            }
            (*(image + j)).x = (*(data + i + 2));
            (*(image + j)).y = (*(data + i + 1));
            (*(image + j)).z = (*(data + i + 0));
            --j;
        }
    }
    normalize_mat_f4(image, height, width, 0.0, 1.0);
    return image;
}

float4* convert16(unsigned char* data, unsigned int height, unsigned int width)
{
    return NULL;
}

float4* convert32(unsigned char* data, unsigned int height, unsigned int width)
{
    return NULL;
}
