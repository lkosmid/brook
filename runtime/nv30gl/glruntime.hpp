/*
 * glruntime.hpp -
 *
 *      The ARB and NV30GL (and perhaps a future ATI specific) runtimes are
 *      both largely derived from an generic OpenGL runtime infrastructure.
 *      This file is the interface for that infrastructure.
 */

#ifndef _GLRUNTIME_HPP_
#define _GLRUNTIME_HPP_
#include "../runtime.hpp"
#include "GLee.h"

#define GL_MAX_TEXCOORDS        8
#define GL_MAX_CONSTS           128


namespace brook {

   class GLRunTime;
   class GLKernel;
   class GLStream;
   class GLIter;

   typedef struct{
      float x, y;
   } glfloat2;

   typedef struct{
      float x, y, z;
   } glfloat3;

   typedef struct{
      float x, y, z, w;
   } glfloat4;

   enum GLArch {ARCH_ATI, ARCH_NV30, ARCH_UNKNOWN};
   
   GLArch getGLArch();

   void __check_gl(int line, char *file);
#define CHECK_GL() __check_gl(__LINE__, __FILE__);

#ifdef WIN32
   void initglfunc(void);
#endif

   class GLKernel : public Kernel {
   public:
      GLKernel(GLRunTime * runtime, const void *[]);
      virtual ~GLKernel();

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
      void Reduce() {
         reduceType == __BRTSTREAM ? ReduceStream() : ReduceScalar();
         ResetStateMachine();
      }

   protected:
      GLKernel(GLRunTime *r) : runtime(r) {
         /* Initialize() must be called before anything useful is done */
      }

      virtual void BindParameter(const float x, const float y,
                                 const float z, const float w) = 0;
      virtual void PushScaleBias(Stream *s) = 0;
      virtual void PushShape(Stream *s) = 0;
      void Initialize(GLRunTime *runtime, const void *sourcelist[],
                      const char *shaderLang);
      void ResetStateMachine();
      void ReduceScalar();
      void ReduceStream();
      void RecomputeTexCoords(unsigned int w, unsigned int h,
                              glfloat4 f1[], glfloat4 f2[]);
      void IssueMapTexCoords(glfloat4 f1[], glfloat4 f2[]);

      void (*ComputeTexCoords) (unsigned int w, unsigned int h,
                                bool is_iter, bool is_1D,
                                float x1, float y1, float z1, float w1,
                                float x2, float y2, float z2, float w2,
                                glfloat4 &f1, glfloat4 &f2);

      GLRunTime *runtime;

      GLuint *pass_id;
      unsigned int *pass_out;
      unsigned int npasses;

      unsigned int nout;
      int sreg, treg, creg;
      int argcount;

      void *reduceVal;
      __BRTStreamType reduceType;
      int  reduceTreg, reduceSreg;
      GLStream *inputReduceStream;
      int  inputReduceStreamTreg, inputReduceStreamSreg;

      GLStream *tmpReduceStream[5];
      GLStream *sreg0;

      char **constnames;

      GLStream **outstreams;
      bool     *argumentUsesIndexof;
      GLStream *sargs[GL_MAX_TEXCOORDS];
      GLIter   *iargs[GL_MAX_TEXCOORDS];
  };

  class GLStream : public Stream {
  public:
     GLStream(GLRunTime * runtime, int fieldCount,
              const __BRTStreamType type[], int dims, const int extents[]);
     virtual ~GLStream ();
     void Read(const void *inData);
     void Write(void *outData);

     void *getData (unsigned int flags);
     void releaseData(unsigned int flags);

     const unsigned int *getExtents() const { return extents; }
     unsigned int getDimension() const { return dims; }
     int getFieldCount() const { return nfields; }

     virtual __BRTStreamType getIndexedFieldType(int i) const {
       return (__BRTStreamType) ncomp[i];
     }

     GLStream *getNext(void) const { return next; }

     int printMemUsage(void);

     unsigned int width, height;
     unsigned int *ncomp;
     GLuint *id;

     GLenum GLtype(const int i) {
        static bool runonce;
        static GLenum lookup[5];

        if (!runonce) {
           switch (getGLArch()) {
           case ARCH_NV30:
              lookup[1] = GL_FLOAT_R32_NV;
              lookup[2] = GL_FLOAT_RG32_NV;
              lookup[3] = GL_FLOAT_RGB32_NV;
              lookup[4] = GL_FLOAT_RGBA32_NV;
              break;
           default: 
              fprintf(stderr, "Unknown ARCH, assuming ATI.\n");
           case ARCH_ATI:
              lookup[1] = GL_INTENSITY_FLOAT32_ATI;
              /* ATI does not have a RG type and cgc expects float2 to 
              ** be stored xy. */
              lookup[2] = GL_RGBA_FLOAT32_ATI;  
              lookup[3] = GL_RGB_FLOAT32_ATI;
              lookup[4] = GL_RGBA_FLOAT32_ATI;
           }
           runonce = true;
        }     
        return lookup[i];
     }


     GLenum GLformat(int i) {
        static bool runonce;
        static GLenum lookup[5];

        if (!runonce) {
           switch (getGLArch()) {
           case ARCH_NV30:
              lookup[1] = GL_RED;
              lookup[2] = GL_LUMINANCE_ALPHA;
              lookup[3] = GL_RGB;
              lookup[4] = GL_RGBA;
              break;
           default: 
              fprintf(stderr, "Unknown ARCH, assuming ATI.\n");
           case ARCH_ATI:
              lookup[1] = GL_RED;
              lookup[2] = GL_RGBA;
              lookup[3] = GL_RGB;
              lookup[4] = GL_RGBA;
           }
           runonce = true;
        }     
        return lookup[i];
     }


  protected:
     __BRTStreamType elementType;
     unsigned int extents[3];
     unsigned int dims;
     unsigned int *stride;
     unsigned int nfields;
     unsigned int elemsize;
     void *cacheptr;

     GLStream *prev;
     GLStream *next;

     GLRunTime *runtime;
  };

  class GLIter : public Iter {
  public:
     GLIter (GLRunTime * runtime,
             __BRTStreamType type,
             int dims,
             int extents[],
             float ranges[]);
     ~GLIter();

     void *getData (unsigned int flags);
     void releaseData(unsigned int flags);
     const unsigned int *getExtents() const {return extents;}
     unsigned int getDimension() const {return dims;}

     unsigned int dims;
     float4 min, max;

  protected:
     unsigned int width, height;
     unsigned int ncomp;
     unsigned int extents[3];
     float *cacheptr;
     GLRunTime *runtime;
  };

  class GLRunTime : public RunTime {
  public:
     GLRunTime();

     Stream *CreateStream(int fieldCount, const __BRTStreamType fieldTypes[],
                          int dims, const int extents[]) {
        return new GLStream(this, fieldCount, fieldTypes, dims, extents);
     }

     Iter *CreateIter(__BRTStreamType type, int dims, int extents[],float r[]) {
        return new GLIter(this, type, dims, extents, r);
     }

     void createPBuffer(int ncomponents);
     void printMemUsage();

     enum WORKSPACESIZE {
        workspace = 2048
        //visual C++ 6.0 doesn't know about static const int
     };

     unsigned int pbuffer_ncomp;
     GLuint passthrough_id;

     /*
      * This is only accessible so that the GLStream constructor can update
      * it.  Otherwise, hands off!
      */
     GLStream *streamlist;

     GLArch arch;

  protected:

#ifdef _WIN32
     void createPBufferWGL(int ncomponents);
     void createWindow(void);
     void createWindowGLContext(void);

     HWND hwnd;
     HDC  hdc_window;
     HGLRC hglrc_window;
     HGLRC hpbufferglrc;
     HPBUFFERARB hpbuffer;
     HDC hpbufferdc;
#else
     void createPBufferGLX(int ncomponents);
     Display    *pDisplay;
     GLXPbuffer  glxPbuffer;
     GLXContext  glxContext;
#endif
  };
}

#endif
