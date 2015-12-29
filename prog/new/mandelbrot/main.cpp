/* mandelbrot.cpp
Generates a high definition Mandelbrot set
(C) 2007 Niall Douglas
Created: 18th July 2007
*/

#include <stdio.h>
#include <string.h>
#include "brook/brook.hpp"
#include "ppmImage.h"

//#define MANDELBROT_SIZE 2048	// Generates 188168055 iterations
#define MANDELBROT_SIZE 3584	// Generates 575265920 iterations
//#define MANDELBROT_SIZE 4096   // Generates 751087345 iterations
//#define MANDELBROT_SIZE 1024
//#define USE_SSE

#ifdef USE_SSE
#include <xmmintrin.h>
#endif
//#include "built/mandelbrot.cpp"
void  mandelbrot (const float2 &position,
		const float &zoom,
		const float &max_iterations,
		const float2 &outputsize,
		::brook::stream output);

// A generic four vector like on a GPU
template<typename type> struct vector4
{
	typedef type TYPE;
	union
	{
		type vec[4];
		struct
		{
			type x, y, z, w;
		};
	};
	vector4() { }
	vector4(type a) { vec[0]=a; vec[1]=a; vec[2]=a; vec[3]=a; }
	vector4(type a, type b) { vec[0]=a; vec[1]=b; vec[2]=b; vec[3]=b; }
	vector4(type a, type b, type c, type d) { vec[0]=a; vec[1]=b; vec[2]=c; vec[3]=d; }
	const float &operator[](int i) const { return vec[i]; }
	float &operator[](int i) { return vec[i]; }
#define BINARYOP(op) vector4 operator op (const vector4 &o) const { return vector4(vec[0] op o.vec[0], vec[1] op o.vec[1], vec[2] op o.vec[2], vec[3] op o.vec[3]); }
	BINARYOP(+)
	BINARYOP(-)
	BINARYOP(*)
	BINARYOP(/)
	BINARYOP(!=)
	BINARYOP(<=)
#undef BINARYOP
#define BINARYOP(op) vector4 &operator op (const vector4 &o) { vec[0] op o.vec[0]; vec[1] op o.vec[1]; vec[2] op o.vec[2]; vec[3] op o.vec[3]; return *this; }
	BINARYOP(+=)
	BINARYOP(-=)
	BINARYOP(*=)
	BINARYOP(/=)
#undef BINARYOP
};
template<typename vectortype> bool operator &&(bool a, const vector4<vectortype> &b) { return a && (b[0] || b[1] || b[2] || b[3]); }

// An aligned four vector using compiler-specific hinting
template<typename type> struct BRTALIGNED vector4aligned
{
	typedef type TYPE;
	union
	{
		type vec[4];
		struct
		{
			type x, y, z, w;
		};
	};
	vector4aligned() { }
	vector4aligned(type a) { vec[0]=a; vec[1]=a; vec[2]=a; vec[3]=a; }
	vector4aligned(type a, type b) { vec[0]=a; vec[1]=b; vec[2]=b; vec[3]=b; }
	vector4aligned(type a, type b, type c, type d) { vec[0]=a; vec[1]=b; vec[2]=c; vec[3]=d; }
	const float &operator[](int i) const { return vec[i]; }
	float &operator[](int i) { return vec[i]; }
#define BINARYOP(op) vector4aligned operator op (const vector4aligned &o) const { return vector4aligned(vec[0] op o.vec[0], vec[1] op o.vec[1], vec[2] op o.vec[2], vec[3] op o.vec[3]); }
	BINARYOP(+)
	BINARYOP(-)
	BINARYOP(*)
	BINARYOP(/)
	BINARYOP(!=)
	BINARYOP(<=)
#undef BINARYOP
#define BINARYOP(op) vector4aligned &operator op (const vector4aligned &o) { vec[0] op o.vec[0]; vec[1] op o.vec[1]; vec[2] op o.vec[2]; vec[3] op o.vec[3]; return *this; }
	BINARYOP(+=)
	BINARYOP(-=)
	BINARYOP(*=)
	BINARYOP(/=)
#undef BINARYOP
};
template<typename vectortype> bool operator &&(bool a, const vector4aligned<vectortype> &b) { return a && (b[0] || b[1] || b[2] || b[3]); }

#ifdef USE_SSE
// A SSE based four vector
template<typename type> struct BRTALIGNED vector4SSE
{
	typedef type TYPE;
	union
	{
		__m128 vec;
		type veca[4];
		struct
		{
			type x, y, z, w;
		};
	};
	vector4SSE() { }
	// GCC is a bit braindead when it comes to SSE intrinsics, so give it
	// some help to help it avoid copying back & forth from SSE regs & memory
	vector4SSE &operator=(const vector4SSE &o) { vec=o.vec; return *this; }
	vector4SSE(__m128 v) { vec=v; }
	vector4SSE(const vector4SSE &o) { vec=o.vec; }

	vector4SSE(type a) { vec = _mm_set1_ps(a); }
	vector4SSE(type a, type b) { vec= _mm_set_ps(b,b,b,a); }
	vector4SSE(type a, type b, type c, type d) { vec= _mm_set_ps(d,c,b,a); }
	const float &operator[](int i) const { return veca[i]; }
	float &operator[](int i) { return veca[i]; }
#define BINARYOP(op, sseop) vector4SSE operator op (const vector4SSE &o) const { return _mm_##sseop##_ps(vec, o.vec); }
	BINARYOP(+, add)
	BINARYOP(-, sub)
	BINARYOP(*, mul)
	BINARYOP(/, div)
#undef BINARYOP
#define BINARYOP(op, sseop) vector4SSE operator op (const vector4SSE &o) const { return _mm_and_ps(_mm_set_ps1(1.0f), _mm_##sseop##_ps(vec, o.vec)); }
	BINARYOP(!=, cmpneq)
	BINARYOP(<=, cmple)
#undef BINARYOP
#define BINARYOP(op, sseop) vector4SSE &operator op (const vector4SSE &o) { vec = _mm_##sseop##_ps(vec, o.vec); return *this; }
	BINARYOP(+=, add)
	BINARYOP(-=, sub)
	BINARYOP(*=, mul)
	BINARYOP(/=, div)
#undef BINARYOP
};
template<typename vectortype> bool operator &&(bool a, const vector4SSE<vectortype> &b) { return a && _mm_movemask_ps(_mm_sub_ps(_mm_setzero_ps(), b.vec)); }
#endif





// Work around a non-aligned temporary bug in MSVC9 and earlier
#if defined(_MSC_VER) && _MSC_VER<=1500
#define MSVCFORCEINLINE __forceinline
#else
#define MSVCFORCEINLINE
#endif

// This implementation uses templates to let the compiler generate an optimal implementation
template<typename vectortype> void nativeColour(typename vectortype::TYPE n, int max_iterations, vectortype &output)
{
	typedef typename vectortype::TYPE type;
	if(n<max_iterations)
	{	/* Do some colouring */
		type v = n / max_iterations;
		type frac;
		
		if (v < 0.167f)
		{
			frac = v / 0.167f;
			output = vectortype(frac, 0.0f, 0.0f, 1.0f);
		}
		else if (v < 0.333f)
		{
			frac = (v - 0.167f) / 0.167f;
			output = vectortype(1.0f, frac, 0.0f, 1.0f);
		}
		else if (v < 0.5f)
		{
			frac = (v - 0.333f) / 0.167f;
			output = vectortype(1.0f, 1.0f, frac, 1.0f);
		}
		else
		{
			frac = (v - 0.5f) / 0.5f;
			output = vectortype(1.0f-frac, 1.0f, 1.0f, 1.0f);
		}			
	}
	else
	{	/* Point lies inside set */
		output = vectortype( 0.0f, 0.0f, 0.0f, 1.0f );
	}
}

// This implementation uses templates to let the compiler generate an optimal implementation
template<typename vectortype> MSVCFORCEINLINE void nativeCompute(vectortype cx, vectortype cy, int max_iterations, vectortype &output)
{
	int i=0;
	vectortype n(0), on(1);
	const vectortype two(2), four(4);
	vectortype x=cx, y=cy, sq, c0, c1, c2, c3;
	for(; i<max_iterations && on!=n; i+=5)
	{
		int j;
		on=n;
		for(j=0; j<5; ++j)			// Six multiplies, five additions, one compare = 12 ops
		{
			vectortype outX = x*x - y*y + cx;
			vectortype outY = two*x*y + cy;
			x=outX;
			y=outY;
			sq=x*x + y*y;
			n+=(sq<=four);
		}
	}
	nativeColour<vectortype>(n[0], max_iterations, c0);
	nativeColour<vectortype>(n[1], max_iterations, c1);
	nativeColour<vectortype>(n[2], max_iterations, c2);
	nativeColour<vectortype>(n[3], max_iterations, c3);
	output=(c0+c1+c2+c2)/four;
}

// This implementation uses templates to let the compiler generate an optimal implementation
template<typename vectortype> void nativeMandelbrot(float *outputData, float &t1, float &t2, int width, int height, const char *runtime)
{
	typename vectortype::TYPE zoom = 1.75f, *_output, *output;
	vectortype position(-1.5f, -0.5f);
	vectortype outputsize((float) width, (float) height);
    output=_output=(typename vectortype::TYPE *) brmalloc(width*height*sizeof(vectortype));
    brook::int64 start=brook::microseconds();

	vectortype pixelsize=vectortype(1)/outputsize/zoom;			/* How much of the mandelbrot set one pixel covers */
	vectortype *outputptr=(vectortype *) output;
	vectortype diffx( 0.25f,  0.25f, -0.25f, -0.25f);
	vectortype diffy( 0.25f, -0.25f, -0.25f,  0.25f);
	diffx*=pixelsize;
	diffy*=pixelsize;
#ifdef _OPENMP
#pragma omp parallel for private(outputptr) schedule(dynamic, 16)
#endif
	for(int y=0; y<height; y++)
	{
#ifdef _OPENMP
		outputptr=(vectortype *) output;
		outputptr+=y*width;
#endif
		for(int x=0; x<width; x++)
		{
			vectortype cx(position.x+x*pixelsize.x);
			vectortype cy(position.y+y*pixelsize.y);
			cx+=diffx;
			cy+=diffy;
			nativeCompute<vectortype>(cx, cy, 100, *outputptr++);
		}
	}
    brook::int64 aftercalc=brook::microseconds();
	{
		typename vectortype::TYPE *outputptr=output;
		for(int n=0; n<width*height*4; n++)
		{
			*outputData++=(float) *outputptr++;
		}
	}
    brook::int64 afterread=brook::microseconds();
    t1=(aftercalc-start)/1000.0f;
    t2=(afterread-aftercalc)/1000.0f;
    brfree(_output);
}

// This implementation uses Brook
static void brookMandelbrot(float *outputData, float &t1, float &t2, int width, int height, const char *runtime)
{
	using namespace brook;
	if(runtime)
		brook::initialize(runtime, NULL);
    {
	    float maxsize = 1.75f;
	    float2 position(-1.5f, -0.5f);
	    float2 outputsize((float) width, (float) height);
        brook::int64 start=brook::microseconds();
	    stream output = stream::create<float4>(width, height);
	    mandelbrot(position, maxsize, 100.0f, outputsize, output);
	    brook::finish();
        brook::int64 aftercalc=brook::microseconds();
	    output.write(outputData);
	    brook::finish();
        brook::int64 afterread=brook::microseconds();
        t1=(aftercalc-start)/1000.0f;
        t2=(afterread-aftercalc)/1000.0f;
    }
	brook::finalize();
}

float Go(void (*impl)(float *outputData, float &t1, float &t2, int width, int height, const char *runtime), const char *runtime=0)
{
    float t1, t2;
	{
		ppmImage image(MANDELBROT_SIZE, MANDELBROT_SIZE);

		printf("\nRendering %u x %u Mandelbrot using '%s' runtime ...\n", MANDELBROT_SIZE*2, MANDELBROT_SIZE*2, runtime);
		impl(image.Data(), t1, t2, image.Width(), image.Height(), runtime);
		printf("Took %f secs to render, %f secs to read the results (TOTAL=%f secs)\n", t1/1000.0f, t2/1000.0f, (t1+t2)/1000.0f);
		printf("    Transfer rate was %f Mb/sec\n", (MANDELBROT_SIZE*MANDELBROT_SIZE*sizeof(float4)/(t2/1000.0f))/(1024*1024));
#if MANDELBROT_SIZE==4096
		printf("        and your GPU did around %f MFLOP/sec\n", 751087.345f*4*13/t1);
#elif MANDELBROT_SIZE==3584
		printf("        and your GPU did around %f MFLOP/sec\n", 575265.920f*4*13/t1);
#elif MANDELBROT_SIZE==2048
		printf("        and your GPU did around %f MFLOP/sec\n", 188168.055f*4*13/t1);
#endif
		char buffer[256];
		sprintf(buffer, "Mandelbrot_%s.ppm", runtime);
                for(char *c=buffer; *c; ++c) if('/'==*c || '\\'==*c || ':'==*c) *c='_';
		image.Write(buffer);
	}
	return t1+t2;
}

int main(void)
{
    bool hasDX9=false;
	const char **devs=brook::runtimeTargets(), **dev;
	printf("Mandelbrot GPU benchmarker (C) 2007 Niall Douglas\n"
		   "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"
		   "Available devices:\n\n");
	for(dev=devs; *dev; ++dev)
    {
		printf("%s\n", *dev);
        if(!strncmp(*dev, "dx9", 3)) hasDX9=true;
    }
#ifdef _OPENMP
    printf("\nCompiled with OpenMP support! ");
#pragma omp parallel
#pragma omp single nowait
    {
      printf("Using %d threads on %d processors ...\n", omp_get_num_threads(), omp_get_num_procs());
    }
#else
    printf("\nCompiled without OpenMP - single processor usage only\n");
#endif
	//Go();
	//float nativetime=Go(nativeMandelbrot< vector4<float> >, "native(float)");
#ifdef USE_SSE
	//float SSEnativetime=Go(nativeMandelbrot< vector4SSE<float> >, "native(SSE)");
   	//printf("SSE native is %f times faster than generic native\n", nativetime/SSEnativetime);
#endif
    float cputime=0;
    for(dev=devs; *dev; ++dev)
    {
        float time=Go(brookMandelbrot, *dev);
        if(dev==devs)
        {
            cputime=time;
            //printf("Generic native is %f times faster than CPU backend\n", cputime/nativetime);
        }
        else
        {
            printf("%s is %f times faster than CPU backend\n", *dev, cputime/time);
        }
    }

	printf("Press Return to exit!\n");
	getchar();
	return 0;
}
