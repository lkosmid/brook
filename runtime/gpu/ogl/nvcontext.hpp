// nvcontext.hpp
#ifndef NV_CONTEXT_HPP
#define NV_CONTEXT_HPP

#include "../gpucontext.hpp"

namespace brook {

  class NVTexture;
  class NVContext;

  class NVTexture:OGLTexture {
  public:

    virtual NVTexture ( size_t inWidth, 
                         size_t inHeight, 
                         TextureFormat inFormat);

    virtual ~NVTexture () {};

    virtual bool
    isFastSetPath( size_t inStrideBytes, 
                   size_t inElemCount );

    virtual bool
    isFastGetPath( size_t inStrideBytes, 
                   size_t inElemCount );

    virtual void 
    copyToTextureFormat( const float *src, 
                         size_t inStrideBytes, 
                         size_t inElemCount,
                         float *dst) const;

    virtual void 
    copyFromTextureFormat( const float *src, 
                           size_t inStrideBytes, 
                           size_t inElemCount,
                           float *dst) const;

    virtual int nativeFormat() const;
  }    


  class NVContext : public OGLContext
  {
  public:
    
    NVContext();

    TextureHandle 
    createTexture2D( size_t inWidth, 
                     size_t inHeight, 
                     TextureFormat inFormat);

    virtual PixelShaderHandle 
    createPixelShader( const char* inSource );

    virtual void bindConstant( size_t inIndex, const float4& inValue ) = 0;
    virtual void bindTexture( size_t inIndex, TextureHandle inTexture ) = 0;
    virtual void bindOutput( size_t inIndex, SurfaceHandle inSurface ) = 0;
    virtual void disableOutput( size_t inIndex ) = 0;

    virtual void bindPixelShader( PixelShaderHandle inPixelShader ) = 0;
    virtual void bindVertexShader( VertexShaderHandle inVertexShader ) = 0;

    virtual void drawRectangle(
      const GPUFatRect& inVertexRectangle, const GPUFatRect* inTextureRectangles, size_t inTextureRectangleCount ) = 0;
  };
}

#endif
