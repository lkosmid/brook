#ifndef NV30GL_H
#define NV30GL_H

#include "../runtime.hpp"

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include "wglext.h"
#else
#include <X11/Xlib.h>
#define GL_GLEXT_LEGACY
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#endif

#include "nv30glext.h"

#ifdef WIN32   
extern PFNWGLCREATEPBUFFERARBPROC      wglCreatePbufferARB;
extern PFNWGLGETPBUFFERDCARBPROC       wglGetPbufferDCARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC  wglChoosePixelFormatARB;
extern PFNWGLBINDTEXIMAGEARBPROC       wglBindTexImageARB;
extern PFNWGLRELEASETEXIMAGEARBPROC    wglReleaseTexImageARB;
extern PFNWGLRELEASEPBUFFERDCARBPROC   wglReleasePbufferDCARB;
extern PFNWGLDESTROYPBUFFERARBPROC     wglDestroyPbufferARB;
extern PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD4FARBPROC     glMultiTexCoord4fARB;
extern PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB;
extern PFNGLGENPROGRAMSNVPROC          glGenProgramsNV;
extern PFNGLLOADPROGRAMNVPROC          glLoadProgramNV;
extern PFNGLBINDPROGRAMNVPROC          glBindProgramNV;
extern PFNGLPROGRAMNAMEDPARAMETER4FNVPROC
                            glProgramNamedParameter4fNV;
#endif 

namespace brook {

   class NV30GLRunTime;
   class NV30GLKernel;
   class NV30GLStream;
   class NV30GLIter;

   typedef struct{
      float x, y;
   } nvfloat2;

   typedef struct{
      float x, y, z;
   } nvfloat3;

   typedef struct{
      float x, y, z, w;
   } nvfloat4;

   const GLenum GLtype[] =   {0, GL_FLOAT_R32_NV, GL_FLOAT_RG32_NV, 
                              GL_FLOAT_RGB32_NV, GL_FLOAT_RGBA32_NV};
   
   // Is there a GL_RG?  I'm not sure that LUMINANCE_ALPHA is correct here
   const GLenum GLformat[] = {0, GL_RED, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};


void __check_gl(int line, char *file);
#define CHECK_GL() __check_gl(__LINE__, __FILE__);

   void initglfunc(void);

#define NV30GL_MAXCONSTS 128

#define NV30GL_MAX_TEXCOORDS 8

   extern const char* NV30GL_RUNTIME_STRING;

   class NV30GLKernel : public Kernel {
   public:
      NV30GLKernel(NV30GLRunTime * runtime,
                   const void *[] );
      void PushStream(Stream *s);
      void PushIter(Iter *s);
      void PushConstant(const float &val); 
      void PushConstant(const float2 &val);
      void PushConstant(const float3 &val); 
      void PushConstant(const float4 &val);
      void PushReduce(void * val, __BRTStreamType type);
      void PushGatherStream(Stream *s);
      void PushOutput(Stream *s);
      void Map();
      void Reduce();
      void Release() {delete this;}
      void ResetStateMachine();
    
      NV30GLRunTime * runtime;
      GLuint id;
      
      int sreg;
      int treg;
      int creg;
      int argcount;

      void *reduceVal;
      __BRTStreamType reduceType;
      int  reduceTreg;
      int  reduceSreg;
      NV30GLStream *inputReduceStream;
      int  inputReduceStreamTreg;
      int  inputReduceStreamSreg;
      
      void ReduceScalar();
      void ReduceStream();
      
      NV30GLStream *tmpReduceStream[5];
      NV30GLStream *sreg0;

      char **constnames;

      NV30GLStream *outstream;
      bool         *argumentUsesIndexof;
      NV30GLStream *sargs[NV30GL_MAX_TEXCOORDS];
      NV30GLIter   *iargs[NV30GL_MAX_TEXCOORDS];

      virtual ~NV30GLKernel();
  };

  class NV30GLStream : public Stream {
  public:
    NV30GLStream (NV30GLRunTime * runtime,
                  __BRTStreamType type, int dims, const int extents[]);
    void Read(const void* inData);
    void Write(void* outData);
    void * getData (unsigned int flags);
    void releaseData(unsigned int flags);
    const unsigned int * getExtents() const {return extents;}
    unsigned int getDimension() const {return dims;}

    virtual int getFieldCount() const {return 1;}
    virtual __BRTStreamType getIndexedFieldType(int i) const {
      assert(i == 0);
      return elementType;
    }

     void GLReadData (void *p);
     void GLWriteData (const void *p);

     __BRTStreamType elementType;
     unsigned int width, height;
     unsigned int extents[3];
     unsigned int dims;
     unsigned int ncomp;
     GLuint id;
     void *cacheptr;

     NV30GLStream *prev;
     NV30GLStream *next;

     NV30GLRunTime *runtime;
     virtual ~NV30GLStream ();
  };

  class NV30GLIter : public Iter {
  public:
    
     NV30GLIter (NV30GLRunTime * runtime, 
                 __BRTStreamType type,
                 int dims, 
                 int extents[],
                 float ranges[]);
     ~NV30GLIter();

     void * getData (unsigned int flags);
     void releaseData(unsigned int flags);
     const unsigned int * getExtents() const {return extents;}
     unsigned int getDimension() const {return dims;}

     unsigned int width, height;
     unsigned int ncomp;
     unsigned int dims;
     unsigned int extents[3];
     float4 min, max;
     float *cacheptr;
     NV30GLRunTime *runtime;
  };

  class NV30GLRunTime : public RunTime {
  public:
     NV30GLRunTime();
     Kernel* CreateKernel(const void*[]);
     Stream* CreateStream(int fieldCount, const __BRTStreamType fieldTypes[],
      int dims, const int extents[]);
     Iter* CreateIter(__BRTStreamType type,
                             int dims, int e[],float r[]);
     ~NV30GLRunTime();

     NV30GLStream *streamlist;

#ifdef _WIN32
     HWND hwnd;
     HDC  hdc_window;
     HGLRC hglrc_window;
     HGLRC hpbufferglrc;
     HPBUFFERARB hpbuffer;
     HDC hpbufferdc;
#else
     Display    *pDisplay;
     GLXPbuffer  glxPbuffer;
     GLXContext  glxContext;
#endif

     int pbuffer_ncomp;

     GLuint passthrough_id;

     enum WORKSPACESIZE{
        workspace = 2048
        //visual C++ 6.0 doesn't know about static const int
     };

     void createWindow(void);
     void createWindowGLContext(void);
     void createPBuffer(int ncomponents);

  };
}

#endif
