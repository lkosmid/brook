#ifndef NV30GL_H
#define NV30GL_H

#include "../runtime.hpp"

#include <windows.h>
#include <GL/gl.h>

#include "glext.h"
#include "wglext.h"

namespace brook {

   class NV30GLRunTime;
   class NV30GLKernel;
   class NV30GLStream;
   class NV30GLIter;

   const GLenum GLtype[] =   {0, GL_FLOAT_R32_NV, GL_FLOAT_RG32_NV, 
                              GL_FLOAT_RGB32_NV, GL_FLOAT_RGBA32_NV};
   
   // Is there a GL_RG?  I'm not ssure that LUMINANCE_ALPHA is correct here
   const GLenum GLformat[] = {0, GL_RED, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};



void __check_gl(int line, char *file);
#define CHECK_GL() __check_gl(__LINE__, __FILE__);

   void initglfunc(void);

#define NV30GL_MAXCONSTS 128

#define NV30GL_MAX_TEXCOORDS 8

   extern const char* NV30GL_RUNTIME_STRING;
   
   extern PFNWGLCREATEPBUFFERARBPROC      wglCreatePbufferARB;
   extern PFNWGLGETPBUFFERDCARBPROC       wglGetPbufferDCARB;
   extern PFNWGLCHOOSEPIXELFORMATARBPROC  wglChoosePixelFormatARB;
   extern PFNWGLBINDTEXIMAGEARBPROC       wglBindTexImageARB;
   extern PFNWGLRELEASETEXIMAGEARBPROC    wglReleaseTexImageARB;
   extern PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB;
   extern PFNGLGENPROGRAMSNVPROC          glGenProgramsNV;
   extern PFNGLLOADPROGRAMNVPROC          glLoadProgramNV;
   extern PFNGLBINDPROGRAMNVPROC          glBindProgramNV;
   extern PFNGLPROGRAMNAMEDPARAMETER4FNVPROC
          glProgramNamedParameter4fNV;
   extern PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB;

   class NV30GLKernel : public Kernel {
   public:
      NV30GLKernel(NV30GLRunTime * runtime,
                   const void *[] );
      virtual void PushStream(Stream *s);
      virtual void PushIter(Iter *s);
      virtual void PushConstant(const float &val); 
      virtual void PushConstant(const float2 &val);
      virtual void PushConstant(const float3 &val); 
      virtual void PushConstant(const float4 &val);
      virtual void PushReduce(void * val, __BRTStreamType type);
      virtual void PushGatherStream(Stream *s);
      virtual void PushOutput(Stream *s);
      virtual void Map();
      virtual void Release() {}
    
      NV30GLRunTime * runtime;
      GLuint id;
      
      int sreg;
      int treg;
      int creg;
      
      char **constnames;

      NV30GLStream *outstream;
      
      NV30GLStream *sargs[NV30GL_MAX_TEXCOORDS];
      
      virtual ~NV30GLKernel();
  };

  class NV30GLStream : public Stream {
  public:
    NV30GLStream (NV30GLRunTime * runtime,
                  __BRTStreamType type, int dims, int extents[]);
    void Read(const void* inData);
    void Write(void* outData);
    void Release() {}
    void * getData (unsigned int flags);
    void releaseData(unsigned int flags);
    const unsigned int * getExtents() const {return extents;}
    unsigned int getDimension() const {return dims;}

     unsigned int width, height;
     unsigned int extents[3];
     unsigned int dims;
     unsigned int ncomp;
     GLuint id;
     void *cacheptr;
     NV30GLRunTime *runtime;
     virtual ~NV30GLStream ();
  };



  class NV30GLIter : public Iter {
     int dims;
     int extents[2];
     float ranges[4];//maximum possible values for dx
  public:
     NV30GLIter(class NV30GLRunTime * runtime,
             __BRTStreamType type,
             int dims,
             int extents[], 
             float ranges[]):Iter(type){
        if (dims>2)
           dims=2;//memory out of bounds check change to assert?
        this->dims=dims;
        for (int i=0;i<dims;++i) {
           this->extents[i]=extents[i];
        }
        unsigned int numranges=type*dims;
        if (numranges>4)
           numranges=4;//memory out of bounds check change to assert?
        memcpy(this->ranges,ranges,sizeof(float)*numranges);
     }
     virtual void * getData (unsigned int flags){assert(0);return 0;}
     virtual void releaseData(unsigned int flags){assert(0);0;}
     virtual const unsigned int * getExtents() const{assert(0);return 0;}
     virtual unsigned int getDimension() const {assert(0);return 0;}
     virtual __BRTStreamType getStreamType ()const{assert(0);return type;}
     virtual unsigned int getTotalSize() const {assert(0);return 0;}
  };

  class NV30GLRunTime : public RunTime {
  public:
    NV30GLRunTime();
    virtual Kernel* CreateKernel(const void*[]);
    virtual Stream* CreateStream(__BRTStreamType type,
                                 int dims, int extents[]);
    virtual Iter* CreateIter(__BRTStreamType type,
                             int dims, int e[],float r[]);
    virtual ~NV30GLRunTime();

     HWND hwnd;
     HGLRC hglrc_window;
     HGLRC hpbufferglrc;
     HPBUFFERARB hpbuffer;

     enum WORKSPACESIZE{
        workspace = 4096
        //visual C++ 6.0 doesn't know about static const int
     };

     void createWindow(void);
     void createWindowGLContext(void);
     void createPBuffer(void);

  };
}

#endif
