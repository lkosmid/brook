// gpucontext.hpp
#ifndef GPU_CONTEXT_HPP
#define GPU_CONTEXT_HPP

#include "gpubase.hpp"

namespace brook {

  /*
  This is the abstract interface that each GPU
  implementation platform must target.
  */
  class GPUContext
  {
  public:
    typedef void* TextureHandle;
    typedef void* PixelShaderHandle;
    typedef void* VertexShaderHandle;

    enum TextureFormat
    {
      kTextureFormat_Float1,
      kTextureFormat_Float2,
      kTextureFormat_Float3,
      kTextureFormat_Float4
    };

    virtual bool isTextureExtentValid( unsigned int inExtent ) { return true; }    
    virtual size_t  getMaximumOutputCount() { return 1; }
    virtual GPURect getStreamPositionInterpolant( TextureHandle* texture ) = 0;
    virtual GPURect getStreamOutputRectangle( TextureHandle* texture ) = 0;

    virtual TextureHandle createTexture2D( unsigned int inWidth, 
                                           unsigned int inHeight, 
                                           TextureFormat inFormat ) = 0;
    virtual void releaseTexture( TextureHandle inTexture ) = 0;

    virtual void setTextureData( TextureHandle inTexture, 
                                 const float* inData, 
                                 unsigned int inStrideBytes, 
                                 unsigned int inElemCount ) = 0;
    virtual void getTextureData( TextureHandle inTexture, 
                                 float* outData, 
                                 unsigned int inStrideBytes, 
                                 unsigned int inElemCount ) = 0;

    virtual PixelShaderHandle createPixelShader( const char* inSource ) = 0;

    virtual VertexShaderHandle getPassthroughVertexShader() = 0;
    virtual PixelShaderHandle getPassthroughPixelShader() = 0;

    virtual void beginScene() = 0;
    virtual void endScene() = 0;

    virtual void bindConstant( unsigned int inIndex, 
                               const float4& inValue ) = 0;
    virtual void bindTexture( unsigned int inIndex, 
                              TextureHandle inTexture ) = 0;
    virtual void bindOutput( unsigned int inIndex, 
                             SurfaceHandle inSurface ) = 0;
    virtual void disableOutput( unsigned int inIndex ) = 0;

    virtual void bindPixelShader( PixelShaderHandle inPixelShader ) = 0;
    virtual void bindVertexShader( VertexShaderHandle inVertexShader ) = 0;

    virtual void drawRectangle(
      const GPUFatRect& inVertexRectangle, 
      const GPUFatRect* inTextureRectangles, 
      unsigned int inTextureRectangleCount ) = 0;
  };
}

#endif
