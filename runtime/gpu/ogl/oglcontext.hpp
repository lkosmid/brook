// gpucontext.hpp
#ifndef OGL_CONTEXT_HPP
#define OGL_CONTEXT_HPP

#include "../gpucontext.hpp"

#ifdef WIN32
#include <windows.h>
#include "wglext.h"
#endif

#include <GL/gl.h>
#include "glext.h"

namespace brook {
   
  /* Virtual class for textures */
  class OGLTexture 
  {
  public:
    
    OGLTexture ( unsigned int inWidth, 
                 unsigned int inHeight, 
                 GPUContext::TextureFormat inFormat ) :
      _width(inWidth), _height(inHeight), _format(inFormat)
    { }

    /* This guy is the real constructor */
    OGLTexture ( unsigned int width,
                 unsigned int height,
                 GPUContext::TextureFormat format,
                 const unsigned int glFormat[4],
                 const unsigned int glType[4],
                 const unsigned int sizeFactor[4]);

    virtual ~OGLTexture ();

    /* This function tests if a data pointer 
    ** with a particular element stride and 
    ** size can be directly passed into 
    ** glTexSubImage.
    */
    virtual bool
    isFastSetPath( unsigned int inStrideBytes, 
                   unsigned int inElemCount ) const;

    /* Same as isFastGetPath, but for
    ** returning data into glReadPixels
    */
    virtual bool
    isFastGetPath( unsigned int inStrideBytes, 
                   unsigned int inElemCount ) const;

    /* Perform a copy into the packed 
    ** texture format.  Individual backends
    ** may override this in case the masquerade
    ** float2 as RGBA texture, etc.
    */
    virtual void 
    copyToTextureFormat( const float *src, 
                         unsigned int inStrideBytes, 
                         unsigned int inElemCount,
                         float *dst) const;

    /* Same as copyToTextureFormat except for 
    ** copying the data out of the packed format
    */
    virtual void 
    copyFromTextureFormat( const float *src, 
                           unsigned int inStrideBytes, 
                           unsigned int inElemCount,
                           float *dst) const;

    /* Basic accessor functions */
    unsigned int width()                 const { return _width;    }
    unsigned int height()                const { return _height;   }
    unsigned int bytesize()              const { return _bytesize; }
    unsigned int components()            const { return _components; }
    GPUContext::TextureFormat format()   const { return _format;   }
    unsigned int id()                    const { return _id;       }

    /* Returns the vendor specific "format" 
    ** parameter for glReadPixels and glTexImage2D
    ** for float textures.
    */
    int nativeFormat()       const { return _nativeFormat; }


  private:
    unsigned int _width, _height, _bytesize;
    unsigned int _components;
    GPUContext::TextureFormat _format;
    unsigned int _id;
    unsigned int _nativeFormat;
  };


  class OGLContext : public GPUContext
  {
  public:
    
    /* Creates a vendor specific backend */
    static OGLContext *oglContextFactory(void);

    /* Creates a context and pbuffer */
    void init();

    /* Everybody supports at least 2048.  Specific backends can change
    ** this if they want...
    */
    virtual bool 
    isTextureExtentValid( unsigned int inExtent ) const
    { return (inExtent <= 2048); }

    /* Everybody supports at least 1.
    */
    virtual unsigned int 
    getMaximumOutputCount() const { return 1; }

    virtual void
    get1DInterpolant( const float4 &start, 
                      const float4 &end,
                      const unsigned int outputWidth,
                      GPUInterpolant &interpolant) const;


    virtual void 
    get2DInterpolant( const float2 &start, 
                      const float2 &end,
                      const unsigned int outputWidth,
                      const unsigned int outputHeight, 
                      GPUInterpolant &interpolant) const;

    virtual void 
    getStreamInterpolant( const TextureHandle texture,
                          const unsigned int outputWidth,
                          const unsigned int outputHeight, 
                          GPUInterpolant &interpolant) const;
    
    virtual void
    getStreamOutputRegion( const TextureHandle texture,
                           GPURegion &region) const; 


    /* The vendor specific backend must create the float textures
    ** since there are no standard float textures. Hence, pure virtual
    */
    virtual TextureHandle 
    createTexture2D( unsigned int inWidth,
                     unsigned int inHeight,
                     TextureFormat inFormat) = 0;
    
    /* I assume that the virtual deconstructor should do the right
    ** thing.
    */
    virtual void 
    releaseTexture( TextureHandle inTexture ) {
      delete (OGLTexture *) inTexture;
    }

    /* Calls glTexSubImage to set the texture data
    */
    void 
    setTextureData( TextureHandle inTexture,
                    const float* inData,
                    unsigned int inStrideBytes,
                    unsigned int inElemCount );

    void 
    getTextureData( TextureHandle inTexture,
                    float* outData,
                    unsigned int inStrideBytes,
                    unsigned int inElemCount );

    /* Creates a shader */
    virtual PixelShaderHandle 
    createPixelShader( const char* inSource );

    /* These are ARB programs */
    virtual VertexShaderHandle getPassthroughVertexShader();
    virtual PixelShaderHandle getPassthroughPixelShader();

    /* OGL does not need these */
    void beginScene() { }
    void endScene() { }

    /* These are the ARB versions */
    virtual void bindConstant( unsigned int inIndex, const float4& inValue );
    virtual void bindTexture( unsigned int inIndex, TextureHandle inTexture );
    virtual void bindOutput( unsigned int inIndex, TextureHandle inSurface );
    virtual void bindPixelShader( PixelShaderHandle inPixelShader );
    virtual void bindVertexShader( VertexShaderHandle inVertexShader );

    /* Not really sure what this should do... */
    virtual void disableOutput( unsigned int inIndex );

    virtual void drawRectangle( const GPURegion &outputRegion, 
                                const GPUInterpolant *interpolants, 
                                unsigned int numInterpolants );

  protected:

    void initPbuffer();
    void bindPbuffer(unsigned int numComponents);
#ifdef WIN32
    virtual void getVendorAttribs(const int   (**iAttrib)[4][64],
                                  const float (**fAttrib)[4][16],
                                  const int   (**piAttrib)[4][16]) {
      *iAttrib = NULL;
      *fAttrib = NULL;
      *piAttrib = NULL;
    }
#endif

  private:
    VertexShaderHandle _passthroughVertexShader;
    PixelShaderHandle _passthroughPixelShader;
    OGLTexture *_outputTexture;
    unsigned int _slopTextureUnit;
    int currentPbufferComponents;

#ifdef WIN32
    void appendVendorAttribs();
    void initPbufferWGL();
    void bindPbufferWGL(unsigned int numComponents);
    HGLRC hglrc;
    HPBUFFERARB hpbuffer;
    HDC hwindowdc;
    HDC hpbufferdc;
    int   iAttribList[4][64];
    float fAttribList[4][16];
    int   piAttribList[4][16];
    int   pixelformat[4];
#else
    void initPbufferGLX();
    void bindPbufferGLX(unsigned int numComponents);
#endif

  };


#ifdef WIN32
/*
 * For some inexplicable reason, on Windows hosts, the OpenGL libraries
 * don't actually export some of their symbols.  Instead, you have to use
 * wglGetProcAddress() to pry them out.  This doesn't stop their header
 * files from defining said symbols, but you get awkward compiler complaints
 * if you actually allow them to be defined.  So, we fake it all here
 * instead of defining either WGL_WGLEXT_PROTOTYPES or GL_GLEXT_PROTOTYPES.
 */

#define RUNTIME_BONUS_GL_FNS \
   XXX(PFNWGLCREATEPBUFFERARBPROC,     wglCreatePbufferARB)            \
   XXX(PFNWGLGETPBUFFERDCARBPROC,      wglGetPbufferDCARB)             \
   XXX(PFNWGLRELEASEPBUFFERDCARBPROC,  wglReleasePbufferDCARB)         \
   XXX(PFNWGLDESTROYPBUFFERARBPROC,    wglDestroyPbufferARB)           \
   XXX(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB)        \
   XXX(PFNWGLBINDTEXIMAGEARBPROC,      wglBindTexImageARB)             \
   XXX(PFNWGLRELEASETEXIMAGEARBPROC,   wglReleaseTexImageARB)          \
                                                                       \
   XXX(PFNGLMULTITEXCOORD2FVARBPROC,   glMultiTexCoord2fvARB)          \
   XXX(PFNGLMULTITEXCOORD4FVARBPROC,   glMultiTexCoord4fvARB)          \
   XXX(PFNGLACTIVETEXTUREARBPROC,      glActiveTextureARB)             \
   XXX(PFNGLGENPROGRAMSARBPROC,        glGenProgramsARB)               \
   XXX(PFNGLBINDPROGRAMARBPROC,        glBindProgramARB)               \
   XXX(PFNGLPROGRAMSTRINGARBPROC,      glProgramStringARB)             \
   XXX(PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, glProgramLocalParameter4fvARB) \


#define RUNTIME_BONUS_NV_FNS \
   XXX(PFNGLGENPROGRAMSNVPROC,         glGenProgramsNV)                \
   XXX(PFNGLLOADPROGRAMNVPROC,         glLoadProgramNV)                \
   XXX(PFNGLBINDPROGRAMNVPROC,         glBindProgramNV)                \
   XXX(PFNGLPROGRAMNAMEDPARAMETER4FNVPROC, glProgramNamedParameter4fNV)\

#define XXX(type, fn) \
   extern type fn;

RUNTIME_BONUS_GL_FNS
RUNTIME_BONUS_NV_FNS
#undef XXX
#endif

  void initglfunc(void);

#define CHECK_GL() __check_gl(__LINE__, __FILE__)
  void __check_gl(int line, char *file);

}

#endif
