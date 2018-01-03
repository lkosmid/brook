// brt.cpp
#ifdef BUILD_CTM
#include "gpu/ctm/ctmruntime.hpp"
#endif

#include "brook/brook.hpp"
#include "runtime.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <float.h>

#ifdef BUILD_DX9
#include "gpu/dx9/dx9runtime.hpp"
#endif

#ifdef BUILD_OGL
#include "gpu/ogl/oglruntime.hpp"
#endif

#ifdef BUILD_GLES
#include "gpu/gles/glesruntime.hpp"
#endif

#ifdef WIN32
#include <windows.h>
#else
#ifndef RPI_NO_X
#include <X11/Xlib.h>
#endif
#ifdef BUILD_OGL
#include <GL/gl.h>
#include <GL/glx.h>
#else 
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif
#endif

#include "cpu/cpu.hpp"

#include <brook/brtscatterintrinsic.hpp>
__StreamScatterAssign STREAM_SCATTER_ASSIGN;
__StreamScatterAdd    STREAM_SCATTER_ADD;
__StreamScatterMul    STREAM_SCATTER_MUL;
__StreamGatherInc     STREAM_GATHER_INC;
__StreamGatherFetch   STREAM_GATHER_FETCH;

inline float finite_flt (float x) {
#ifdef _WIN32
   return (float) _finite(x);
#else
#ifdef __APPLE__
   return (float) __isfinitef(x);
#else
   return (float) finite(x);
#endif
#endif
}

inline float isnan_flt (float x) {
#ifdef _WIN32
   return (float) _isnan(x);
#else
#ifdef __APPLE__
   return (float) __isnanf(x);
#else
   return (float) isnan(x);
#endif
#endif
}

#include "logger.hpp"

static int convertGatherIndexToInt( float inIndex ) {
  return (int) (inIndex + 0.5f);
}

extern "C" void brookRuntimeCleanupCallback()
{
   brook::finalize();
}

namespace brook {

  static const char* RUNTIME_ENV_VAR = "BRT_RUNTIME";
  static BRTTLS Runtime* currentRuntime;

  /* Returns a list of GPU resources */
  const char** runtimeTargets()
  {
    static char *devs[64];
    if(devs[0]) return (const char **) devs;
    int n;
    char **dev;
    dev=devs;
    *dev++=(char *) "cpu";
#ifdef BUILD_CTM
    *dev++=(char *) "ctm";
#endif
#if defined(BUILD_OGL) || defined(BUILD_DX9) || defined(BUILD_GLES)
#ifdef WIN32
    DISPLAY_DEVICE odd, dd={ sizeof(DISPLAY_DEVICE) };
    for(n=0; dev<devs+(sizeof(devs)/sizeof(char *))-1; n++)
    {
        odd=dd;
        if(!EnumDisplayDevices(NULL, n, &dd, NULL)) break;
        if((dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) continue;
        if(!(dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;
        if(!strcmp(odd.DeviceID, dd.DeviceID)) continue;
        //if(strstr(dd.DeviceString, "Secondary")) continue;
#ifdef BUILD_OGL
        *dev=(char *) brmalloc(strlen(dd.DeviceName)+strlen(dd.DeviceString)+2+4);
        strcpy(*dev, OGL_RUNTIME_STRING ":");
        strcat(*dev, dd.DeviceName);
        strcat(*dev, ":");
        strcat(*dev, dd.DeviceString);
        dev++;
#endif
#ifdef BUILD_DX9
        *dev=(char *) brmalloc(strlen(dd.DeviceName)+strlen(dd.DeviceString)+2+4);
        strcpy(*dev, DX9_RUNTIME_STRING ":");
        strcat(*dev, dd.DeviceName);
        strcat(*dev, ":");
        strcat(*dev, dd.DeviceString);
        dev++;
#endif
    }
#else
#ifndef RPI_NO_X
    // Maybe consider upgrading this to use Xinerama and XRandR
    XInitThreads();
    Display *dh=XOpenDisplay(NULL);
    if(dh)
    {
        for(n=0; n<ScreenCount(dh); n++)
        {
#endif
#ifdef BUILD_OGL
            int attrib[] = { GLX_RGBA, None };
            XVisualInfo *visual = glXChooseVisual(dh, n, attrib);
            if(visual){
              GLXContext glxContext = glXCreateContext(dh, 
                                            visual, 
                                            0, GL_TRUE);
              if(glxContext){
                   XSetWindowAttributes swa;
                   Colormap cmap = XCreateColormap (dh, 
                                           RootWindow(dh, n),
                                           visual->visual, AllocNone);
                   swa.border_pixel = 0;
                   swa.colormap = cmap;

                   Window glxWindow = XCreateWindow(dh,
                                             RootWindow(dh, n),
                                             0, 0, 1, 1, 0, visual->depth, InputOutput,
                                             visual->visual, CWBorderPixel | CWColormap,
                                                               &swa);
                   if (glXMakeCurrent(dh, glxWindow, glxContext)) {
                     const char *vendor   = (const char *) glGetString(GL_VENDOR);
                     const char *renderer = (const char *) glGetString(GL_RENDERER);

                     *dev=(char *) brmalloc(4+4+strlen(DisplayString(dh))+(vendor ? 2+strlen(vendor)+strlen(renderer) : 0));
                     strcpy(*dev, OGL_RUNTIME_STRING ":");
                     strcat(*dev, DisplayString(dh));
                     char *d=strchr(*dev, 0);
                     if(d[-2]=='.') d-=2;
                     sprintf(d, ".%d", n);
                     if(vendor){
                       strcat(*dev, ":");
                       strcat(*dev, vendor);
                       strcat(*dev, " ");
                       strcat(*dev, renderer);
                     }
                     dev++;
                   }
                   glXMakeCurrent(dh, None, NULL);
                   XDestroyWindow(dh, glxWindow);
                   XFreeColormap(dh, cmap);
                   glXDestroyContext(dh, glxContext);
              }
              XFree(visual);
            }
#endif
#ifdef BUILD_GLES
#ifdef RPI_NO_X
            #define DisplayString(dh) "0"
            n=0;
#endif
            //Keep it simple for the moment to make it work without hassle
            const char *vendor="";
            const char *renderer="";
            *dev=(char *) brmalloc(4+4+strlen(DisplayString(dh))+(vendor ? 2+strlen(vendor)+strlen(renderer) : 0));
            strcpy(*dev, GLES_RUNTIME_STRING":");
            strcat(*dev, DisplayString(dh));
            char *d=strchr(*dev, 0);
            if(d[-2]=='.') d-=2;
            sprintf(d, ".%d", n);
            if(vendor){
              strcat(*dev, ":");
              strcat(*dev, vendor);
              strcat(*dev, " ");
              strcat(*dev, renderer);
            }
            dev++;
#endif
#ifndef RPI_NO_X
        }
        XCloseDisplay(dh);
    }
#endif
#endif
#endif
    return (const char **) devs;
  }

  /* start up the brook runtime with a given implemention */
  Runtime* initialize( const char* inRuntimeName, void* inContextValue )
  {
    return Runtime::GetInstance( inRuntimeName, inContextValue, false );
  }

  /* returns the current brook runtime for the calling thread */
  Runtime* runtime()
  {
    return Runtime::GetInstance();
  }

  /* sets the current brook runtime for the calling thread */
  void setRuntime(Runtime* runtime)
  {
    currentRuntime=runtime;
  }

  /* shut down the brook runtime, (shouldn't often be needed) */
  void finalize()
  {
    delete currentRuntime;
    currentRuntime = NULL;
  }

  /* tell brook to complete any queued operations on the gpu */
  void finish()
  {
     Runtime::GetInstance()->finish();
  }

  /* inform brook that the client intends to use the gpu (e.g. to render) */
  void unbind()
  {
     Runtime::GetInstance()->unbind();
  }

  /* inform brook that the client is done with the gpu, and wants to use brook again */
  void bind()
  {
     Runtime::GetInstance()->bind();
  }

  Runtime* createRuntime( bool addressTranslation )
  {
    return Runtime::GetInstance( 0, 0, addressTranslation );
  }
    
  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  Runtime* Runtime::GetInstance( const char* inRuntimeName, void* inContextValue, bool addressTranslation )
  {
    if( currentRuntime == NULL )
    {
      currentRuntime = CreateInstance( inRuntimeName, inContextValue, addressTranslation );

      static bool sCleanupRegistered = false;
      if( !sCleanupRegistered )
      {
         atexit( brookRuntimeCleanupCallback );
         sCleanupRegistered = true;
      }
    }
    return currentRuntime;
  }

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  Runtime* Runtime::CreateInstance( const char* inRuntimeName, void* inContextValue, bool addressTranslation ) {
    const char *env = inRuntimeName != NULL ? inRuntimeName : getenv(RUNTIME_ENV_VAR);

    BROOK_LOG(0) << "Brook Runtime starting up" << std::endl;

    if (!env) {
      fprintf (stderr,"*****WARNING*****WARNING**********\n");
      fprintf (stderr,"*****WARNING*****WARNING**********\n");
      fprintf (stderr,"*****WARNING*****WARNING**********\n");
      fprintf (stderr,"**********************************\n");
      fprintf (stderr,"*                                *\n");
      fprintf (stderr,"* BRT_RUNTIME env variable is    *\n");
      fprintf (stderr,"* not set. Defaulting to CPU     *\n");
      fprintf (stderr,"* runtime.                       *\n");
      fprintf (stderr,"*                                *\n");
      fprintf (stderr,"* CPU Backend:                   *\n");
      fprintf (stderr,"* BRT_RUNTIME=cpu                *\n");
      fprintf (stderr,"*                                *\n");
      fprintf (stderr,"* OpenGL Backend:                *\n");
#ifdef BUILD_OGL
      fprintf (stderr,"* BRT_RUNTIME=%s                 *\n", OGL_RUNTIME_STRING);
#else
      fprintf (stderr,"* Not supported on this platform *\n");
#endif
      fprintf (stderr,"*                                *\n");
      fprintf (stderr,"* OpenGL ES Backend:             *\n");
#ifdef BUILD_GLES
      fprintf (stderr,"* BRT_RUNTIME=%s               *\n", GLES_RUNTIME_STRING);
#else
      fprintf (stderr,"* Not supported on this platform *\n");
#endif
      fprintf (stderr,"*                                *\n");
      fprintf (stderr,"* DirectX9 Backend:              *\n");
#ifdef BUILD_DX9                                       
      fprintf (stderr,"* BRT_RUNTIME=%s                 *\n", DX9_RUNTIME_STRING);
#else
      fprintf (stderr,"* Not supported on this platform *\n");
#endif
      fprintf (stderr,"*                                *\n");
      fprintf (stderr,"* CTM Backend:                   *\n");
#ifdef BUILD_CTM                                       
      fprintf (stderr,"* BRT_RUNTIME=%s                 *\n", CTM_RUNTIME_STRING);
#else
      fprintf (stderr,"* Not supported on this platform *\n");
#endif
      fprintf (stderr,"*                                *\n");
      fprintf (stderr,"**********************************\n");
      fprintf (stderr,"******WARNING*****WARNING*********\n");
      fprintf (stderr,"******WARNING*****WARNING*********\n");
      fprintf (stderr,"******WARNING*****WARNING*********\n\n");
      fflush  (stderr);
      return new CPURuntime();
    }

#ifdef BUILD_DX9
    if (!strncmp(env, DX9_RUNTIME_STRING, sizeof(DX9_RUNTIME_STRING)-1))
    {
      Runtime* result = GPURuntimeDX9::create( inContextValue, env );
      if( result )
        return result;

      fprintf(stderr, 
	      "Unable to initialize DX9 runtime, falling back to CPU\n");
      return new CPURuntime();
    }
#endif
    
#ifdef BUILD_CTM
    if (!strncmp(env, CTM_RUNTIME_STRING, sizeof(CTM_RUNTIME_STRING)-1))
    {
      Runtime* result = GPURuntimeCTM::create( inContextValue );
      if( result )
        return result;

      fprintf(stderr, 
	      "Unable to initialize CTM runtime, falling back to CPU\n");
      return new CPURuntime();
    }
#endif

#ifdef BUILD_OGL
    if (!strncmp(env, OGL_RUNTIME_STRING, sizeof(OGL_RUNTIME_STRING)-1)) {
      Runtime* result = OGLRuntime::create( inContextValue, env );
      if( result )
        return result;

      fprintf(stderr, 
	      "Unable to initialize OpenGL runtime, falling back to CPU\n");
      return new CPURuntime();
    }
#endif

#ifdef BUILD_GLES
    if (!strncmp(env, GLES_RUNTIME_STRING, sizeof(GLES_RUNTIME_STRING)-1)) {
      Runtime* result = GLESRuntime::create( inContextValue, env );
      if( result )
        return result;

      fprintf(stderr, 
	      "Unable to initialize OpenGL ES 2.0 runtime, falling back to CPU\n");
      return new CPURuntime();
    }
#endif

    if (strcmp(env,CPU_RUNTIME_STRING)) {
      fprintf (stderr, "Unknown runtime requested: %s\n", env);
      fprintf (stderr, "Runtimes:\n\n");
      fprintf (stderr, "  CPU Backend:                   \n");
      fprintf (stderr, "  BRT_RUNTIME = cpu              \n");
      fprintf (stderr, "                                 \n");
      fprintf (stderr, "  OpenGL Backend:                \n");
#ifdef BUILD_OGL       
      fprintf (stderr, "  BRT_RUNTIME = %s              \n", OGL_RUNTIME_STRING);
#else                  
      fprintf (stderr, "  Not supported on this platform \n");
#endif                 
#ifdef BUILD_GLES       
      fprintf (stderr, "  BRT_RUNTIME = %s              \n", GLES_RUNTIME_STRING);
#else                  
      fprintf (stderr, "  Not supported on this platform \n");
#endif                 
      fprintf (stderr, "                                 \n");
      fprintf (stderr, "  DirectX9 Backend:              \n");
#ifdef BUILD_DX9                                        
      fprintf (stderr, "  BRT_RUNTIME = %s              \n", DX9_RUNTIME_STRING);
#else                  
      fprintf (stderr, "  Not supported on this platform \n");
#endif                   
      fprintf (stderr, "                                 \n");
      fprintf (stderr, "  CTM Backend:              \n");
#ifdef BUILD_CTM                                        
      fprintf (stderr, "  BRT_RUNTIME = %s              \n", CTM_RUNTIME_STRING);
#else                  
      fprintf (stderr, "  Not supported on this platform \n");
#endif                           
      fprintf (stderr, "                                \n");
      fprintf (stderr, "Falling back to CPU...\n");
      fflush(stderr);
    }
    return new CPURuntime();
  }

  void* StreamInterface::getIndexedFieldRenderData(unsigned int i) {
    return NULL;
  }

  void StreamInterface::synchronizeRenderData() {
  }
  unsigned int getElementSize(StreamType fieldType) {
      switch(fieldType)
      {
      case __BRTCHAR:
          return sizeof(char);
      case __BRTCHAR2:
          return sizeof(char2);
      case __BRTCHAR3:
          return sizeof(char3);
      case __BRTCHAR4:
          return sizeof(char4);
      case __BRTINT:
          return sizeof(uint32_t);
      case __BRTDOUBLE:
          return sizeof(double);
      case __BRTDOUBLE2:
          return 2*sizeof(double);
      case __BRTFIXED:
         return sizeof(fixed);
      case __BRTFIXED2:
         return sizeof(fixed2);

      case __BRTFIXED3:
         return sizeof(fixed3);

      case __BRTFIXED4:
         return sizeof(fixed4);
      case __BRTSHORTFIXED:
         return sizeof(shortfixed);
      case __BRTSHORTFIXED2:
         return sizeof(shortfixed2);

      case __BRTSHORTFIXED3:
         return sizeof(shortfixed3);

      case __BRTSHORTFIXED4:
         return sizeof(shortfixed4);

      case __BRTFLOAT:
        return sizeof(float);

      case __BRTFLOAT2:
        return sizeof(float2);

      case __BRTFLOAT3:
        return  sizeof(float3);

      case __BRTFLOAT4:
        return sizeof(float4);
      default:
        assert(false && "invalid stream element type");
        return 0;
      };
  }
  unsigned int StreamInterface::getElementSize() const
  {
    unsigned int result = 0;
    int fieldCount = getFieldCount();
    for( int i = 0; i < fieldCount; i++ )
    {
        result+=::brook::getElementSize(getIndexedFieldType(i));
    }
    return result;
  }

  void StreamInterface::readItem (void * output, unsigned int * index){
    void * data = (char*)getData(READ);
    void * ptr = fetchItemPtr(data, index);
    unsigned int size = getElementSize();
    memcpy (output,ptr,size);
    releaseData(READ);
  } 

  __BrtFloat4 StreamInterface::computeIndexOf(unsigned int linear_index) {
     const unsigned int * domain_min = getDomainMin();
     const unsigned int * extents = getExtents();
     unsigned int dim = getDimension();
     int i;

     assert (dim > 0 && dim <= 4);

     unsigned int index[4] = {0, 0, 0, 0};
     unsigned int largestaddr = 1;
     
     for (i=((int)dim)-1; i>=0; --i) {
       largestaddr *= extents[i];
     }

     for (i=((int)dim)-1; i>=0; --i) {
       largestaddr /= extents[dim-i-1];
       index[i] = linear_index / largestaddr;
       linear_index -= largestaddr * index[i];
       index[i] -= domain_min[dim-i-1];
     } 

     return __BrtFloat4((float) index[0], 
                        (float) index[1], 
                        (float) index[2], 
                        (float) index[3]);
  }

  void * StreamInterface::fetchItemPtr (void * data, 
                                        unsigned int * index) {
     const unsigned int * domain_min = getDomainMin();
     const unsigned int * extents = getExtents();
     unsigned int dim = getDimension();
     unsigned int size = getElementSize();

     assert(dim>0);

     unsigned int linearindex = 0;
     unsigned int offset = 1;

     for (int i=((int)dim)-1; i>=0; --i) {
        linearindex += (index[i] + domain_min[i]) * offset;
        offset *= extents[i];
     }

     return ((unsigned char *) data) + linearindex*size;
  }

  stream::stream()
    : _stream(0)
  {
  }

  stream::stream( const stream& inStream )
    : _stream(inStream._stream)
  {
    if( _stream ) _stream->acquireReference();
  }
  
  stream& stream::operator=( const stream& inStream )
  {
    Stream* s = inStream._stream;
    if( s ) s->acquireReference();
    if( _stream ) _stream->releaseReference();
    _stream = s;
    return *this;
  }

  stream::~stream() {
    if(_stream) _stream->releaseReference();
  }

  
  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  stream::stream(const StreamType* inElementTypes, ...)
    : _stream(0)
  {
    std::vector<StreamType> elementTypes;
    std::vector<unsigned int> extents;

    const StreamType* e = inElementTypes;
    while(*e != __BRTNONE)
    {
      elementTypes.push_back(*e);
      e++;
    }

    va_list args;
    va_start(args,inElementTypes);
    for(;;)
    {
      int extent = va_arg(args,int);
      if( extent == -1 ) break;
      extents.push_back(extent);
    }
    va_end(args);

    _stream = brook::Runtime::GetInstance()->CreateStream(
      elementTypes.size(), 
      (const StreamType *) &elementTypes[0], 
      extents.size(), 
      (const unsigned int *) &extents[0] );
  }

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  stream::stream(const unsigned int * extents,
                 unsigned int dims,
                 const StreamType *type, bool read_only)
    : _stream(0)
  {
     std::vector<StreamType>elementTypes;
     const StreamType * e = type;
     while (*e!=__BRTNONE) {
        elementTypes.push_back(*e);
        e++;
     }
    _stream = brook::Runtime::GetInstance()->CreateStream
       (elementTypes.size(), &elementTypes[0], dims, extents, read_only);
  }

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  stream::stream( const ::brook::iter& i )
    : _stream(0)
  {
    ::brook::Iter* iterator = i;

    StreamType elementType = iterator->getIndexedFieldType(0);
    int dimensionCount = iterator->getDimension();
    unsigned int* extents = (unsigned int *)(iterator->getExtents());

    _stream = brook::Runtime::GetInstance()->CreateStream( 1, 
                                                           &elementType, 
                                                           dimensionCount, 
                                                           extents );
    _stream->Read( iterator->getData( brook::Stream::READ ) );
    iterator->releaseData( brook::Stream::READ );
  }

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  iter::iter(StreamType type, ...)
    : _iter(0)
  {
    std::vector<unsigned int> extents;
    std::vector<float> ranges;
    va_list args;
    va_start(args,type);
    for(;;)
    {
      int extent = va_arg(args,int);
      if( extent == -1 ) break;
      extents.push_back(extent);
    }
    for (int i=0;i<type;++i) {
      float f = (float) va_arg(args,double);
      //     fprintf(stderr, "float %f\n",f);
      ranges.push_back(f);
      f = (float) va_arg(args,double);
      //     fprintf(stderr, "float %f\n",f);
      ranges.push_back(f);
    }
    va_end(args);

    _iter = brook::Runtime::GetInstance()->CreateIter( type, 
                                                      extents.size(), 
                                                      (unsigned int *) &extents[0],
                                                      &ranges[0]);
  }

  // o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
  kernel::kernel()
    : _runtime(NULL), _kernel(NULL)
  {
  }

  void kernel::initialize(const void* code[])
  {
    if(_runtime!=brook::Runtime::GetInstance()) {
      if( _kernel != 0 )
        _kernel->Release();
      _kernel = (_runtime=brook::Runtime::GetInstance())->CreateKernel( code );
    }
  }

  float getSentinel() {
    return (float)-1844632.18612444856320;
  } 
  
  class StreamSentinels {
  public:
    std::vector<stream *> sentinels;
    ~StreamSentinels() {
      while (!sentinels.empty()) {
        if (sentinels.back())
          delete sentinels.back();
        sentinels.pop_back();
      }
    }
  };

  stream *sentinelStream (int dim) {    
    static StreamSentinels s;
    float onehalf = 0.5f;
    BRTALIGNED float inf = 1.0f/(float)floor(onehalf);
  
    if (dim<(int)s.sentinels.size())
      if (s.sentinels[dim]!=0)
        return s.sentinels[dim];

    while ((int)s.sentinels.size()<=dim)
      s.sentinels.push_back(0);

    std::vector<unsigned int, brallocator<unsigned int> > extents;

    for (int i=0;i<dim;++i){
      extents.push_back(1);
    }

    s.sentinels[dim]=new brook::stream(&extents.front(),
                                       dim,
                                       brook::getStreamType((float*)0));   
    streamRead(*s.sentinels[dim],&inf);
    
    return s.sentinels[dim];
  }
    

}

void streamPrint(brook::StreamInterface * s, bool flatten) {
  flatten=false;
  unsigned int dims = s->getDimension();
   const unsigned int * extent = s->getExtents();
   unsigned int tot = s->getTotalSize();
   unsigned int numfloats = 0;
   unsigned int numfields = s->getFieldCount();
   for (unsigned int fields= 0; fields<numfields;++fields) {
     numfloats+=s->getIndexedFieldType(fields);
   }
   float * data = (float *)s->getData(brook::StreamInterface::READ);
   for (unsigned int i=0;i<tot;++i) {
     if (numfloats!=1)printf( "{");
     for (unsigned int j=0;j<numfloats;++j) {
       float x = data[i*numfloats+j];
       if (j!=0) {
         printf(",");
         printf(" ");
       }
       if (finite_flt(x)) {
         
         if (x==36893206672442393000.00)
           printf("inf");
         else if (x==-18446321861244485632.0f) {
           printf ("inf");
         }else if (fabs(x)<.000001)
           printf ("0.00");
         else
           printf("%3.2f",x);
       }
       else if (isnan_flt(x))
         printf("NaN");
       else 
         printf ("inf");
     }
     
     if (numfloats!=1)
       printf("}");
     else
       printf (" ");
     if (!flatten)
       if ((i+1)%extent[dims-1]==0)
         printf("\n");
   }
   s->releaseData(brook::StreamInterface::READ);
}

void readItem (brook::StreamInterface *s, void * p,...) {
  unsigned int dims = s->getDimension();
  std::vector<unsigned int> index;
  va_list args;
  va_start(args,p);
  for (unsigned int i=0;i<dims;++i) {
    index.push_back(va_arg(args,int));
  }
  va_end(args);
  s->readItem(p,&index[0]);
}

void __streamGatherOrScatterOp (::brook::StreamInterface *dst, 
                                ::brook::StreamInterface *index,
                                ::brook::StreamInterface *src,
                                void (*func) (void *, void *),
                                bool GatherNotScatter) {
  unsigned int i;
  unsigned int *curpos;
  ::brook::StreamType index_type;

  // Fetch the relavent stream properties
  // Get the Dimensions
  unsigned int index_dims = index->getDimension();
  unsigned int src_dims   = src->getDimension();
  unsigned int dst_dims   = dst->getDimension();

  // Get Domain properties.
  const unsigned int *index_dmin = index->getDomainMin();
  const unsigned int *index_dmax = index->getDomainMax();
  const unsigned int *dst_dmin   = dst->getDomainMin();
  const unsigned int *dst_dmax   = dst->getDomainMax();

  // Get element sizes
  unsigned int src_elemsize = src->getElementSize();
  unsigned int dst_elemsize = dst->getElementSize();

  // Get actual data
  float *        index_data = (float *) index->getData(::brook::Stream::READ);
  unsigned char *src_data;
  unsigned char *dst_data;

  if (GatherNotScatter) {
    src_data  = (unsigned char *) src->getData(::brook::Stream::READWRITE);
    dst_data  = (unsigned char *) dst->getData(::brook::Stream::WRITE);
  } else {
    src_data  = (unsigned char *) src->getData(::brook::Stream::READ);
    dst_data  = (unsigned char *) dst->getData(::brook::Stream::READWRITE);
  }

  // Where we'll do the op
  void *buf1;
  void *buf2;

  // Do some assertion tests here to make sure
  // that the gatherop is legal

  // Really, we should check to make sure that the 
  // dst and src streams should be the same time...
  // But for now, we are simply going to make sure
  // that the stream elements are of the same size.
  assert (src_elemsize == dst_elemsize);

  // The index type must be a float1-4 stream
  assert (index->getFieldCount() == 1);
  index_type = index->getIndexedFieldType(0);

  // The index type must match the number of dimensions
  // in the source, i.e. float4 index for a 4D stream
  assert (index_type == (int) src_dims);

  // The index dimensionality must match the 
  // output dimensionality.
  assert (index_dims == dst_dims);

  // Things must be 16 byte aligned if there are four elements
  //if(4==index_dims && (size_t)index_data & 0xf) { std::cerr<<"Index data is not 16 byte aligned!"<<std::endl; abort(); }
  if(16==src_elemsize && (size_t)src_data & 0xf) { std::cerr<<"Source data is not 16 byte aligned!"<<std::endl; abort(); }
  if(16==dst_elemsize && (size_t)dst_data & 0xf) { std::cerr<<"Destination data is not 16 byte aligned!"<<std::endl; abort(); }

  // The number of elements of the index must match the 
  // the number of elements of the destination
  for (i=0; i<index_dims; i++)
    assert (index_dmax[i] - index_dmin[i] ==
            dst_dmax[i] - dst_dmin[i]);

  // Create the counter
  curpos = (unsigned int *) brmalloc (index_dims * sizeof(unsigned int));
  
  // Create the buffer for gather op
  buf1 = brmalloc (src_elemsize);
  buf2 = brmalloc (src_elemsize);

  // Initialize it to zero
  for (i=0; i<index_dims; i++)
    curpos[i] = 0;

  // Perform the gatherop
  bool finished = false;
  do {
    unsigned int index_intptr[4];

     // get the index value
    float *index_ptr = (float *) index->fetchItemPtr(index_data,
                                                     curpos);

    // create an int vector from the float values
    // Note that we have to flip the vector since I 
    // do everything as z,y,x and the user specifies as
    // x,y,z
    for (i=0; i<(unsigned int) index_type; i++)
      index_intptr[i] = convertGatherIndexToInt( index_ptr[index_type - 1 - i] );

    // get the src value
    void *src_ptr = src->fetchItemPtr(src_data, 
                                      index_intptr);
    
    // get the dst value
    void *dst_ptr = dst->fetchItemPtr(dst_data,
                                      curpos);
    
    if (GatherNotScatter) {
      // Gather Path
      // copy the data over
      memcpy (dst_ptr, src_ptr, src_elemsize);
      
      // perform the op part of the gather op on src
      // XXX: Bug we need to know what which is the output 
      // and which is the input.
      // For now we will just take whichever output changes
      
      memcpy (buf1, src_ptr, src_elemsize);
      memcpy (buf2, src_ptr, src_elemsize);
      func   (buf1, buf2);
      
      if (memcmp(buf1, src_ptr, src_elemsize)) {
        assert(memcmp(buf2, src_ptr, src_elemsize)==0);
        memcpy(src_ptr, buf1, src_elemsize);
      } else 
        memcpy(src_ptr, buf2, src_elemsize);

    } else {
      // Scatter Path
      // XXX: Bug we need to know what which is the output 
      // and which is the input.
      // For now we assume that the reduce argument is second
      func(src_ptr, dst_ptr);
    }

    // Increment the curpos
    for (i=index_dims-1; i>=0; i--) {
      curpos[i]++;
      if (curpos[i] == (index_dmax[i]-index_dmin[i])) {
        if (i == 0) {
          finished = true;
          break;
        }
        curpos[i] = 0;
      } else
        break;
    }
  } while (!finished);
    
  index->releaseData(::brook::Stream::READ);
  if (GatherNotScatter) {
    src->releaseData(::brook::Stream::READWRITE);
    dst->releaseData(::brook::Stream::WRITE);
  } else {
    src->releaseData(::brook::Stream::READ);
    dst->releaseData(::brook::Stream::READWRITE);
  }

  brfree(curpos);
  brfree(buf1);
  brfree(buf2);
}


/* Always 32 byte aligned memory allocation */
void *brmalloc(size_t size)
{
    unsigned int *ret, *ret2;
    ret=ret2=(unsigned int *) malloc(size+28+8);
    if(!ret) return ret;
    ret+=2;
    while(((size_t)ret) & 0x1f) ret++;
    ret[-1]=*(unsigned int *)"BROK";
    ret[-2]=ret-ret2;
    return ret;
}
void *brcalloc(size_t no, size_t size)
{
    void *ret=brmalloc(no*size);
    if(!ret) return ret;
    memset(ret, 0, no*size);
    return ret;
}
void *brrealloc(void *ptr, size_t size)
{
    assert(brismalloc(ptr));
    unsigned int *p=(unsigned int *)ptr;
    p-=p[-2];
    unsigned int *ret, *ret2;
    ret=ret2=(unsigned int *) realloc(p, size+28+8);
    if(!ret) return ret;
    ret+=2;
    while(((size_t)ret) & 0x1f) ret++;
    ret[-1]=*(unsigned int *)"BROK";
    ret[-2]=ret-ret2;
    return ret;
}
bool brismalloc(void *ptr)
{
    unsigned int *p=(unsigned int *)ptr;
    return p[-1]==*(unsigned int *)"BROK";
}
void brfree(void *ptr)
{
    assert(brismalloc(ptr));
    unsigned int *p=(unsigned int *)ptr;
    p-=p[-2];
    free(p);
}

/* C API for runtime targets etc */
const char **brruntimes() { return brook::runtimeTargets(); }
void brsetruntime(const char *runtime) { brook::initialize(runtime); }
brint64 brmicroseconds() { return brook::microseconds(); }

// TIM: adding conditional magick for raytracer
void streamEnableWriteMask()
{
  using namespace brook;
  Runtime* runtime = Runtime::GetInstance();
  runtime->hackEnableWriteMask();
}

void streamDisableWriteMask()
{
  using namespace brook;
  Runtime* runtime = Runtime::GetInstance();
  runtime->hackDisableWriteMask();
}

void streamSetWriteMask( ::brook::stream& inStream )
{
  using namespace brook;
  Runtime* runtime = Runtime::GetInstance();
  runtime->hackSetWriteMask( (Stream*) inStream );
}

void streamBeginWriteQuery()
{
  using namespace brook;
  Runtime* runtime = Runtime::GetInstance();
  runtime->hackBeginWriteQuery();
}

int streamEndWriteQuery()
{
  using namespace brook;
  Runtime* runtime = Runtime::GetInstance();
  return runtime->hackEndWriteQuery();
}

