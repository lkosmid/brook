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

    /* Test to see if we can create a texture with
    ** a dimension of this size
    */
    virtual bool 
    isTextureExtentValid( unsigned int inExtent ) const { return true; }    
    
    /* Returns the number of shader outputs supported by
    ** the hardware.
    */
    virtual unsigned int 
    getMaximumOutputCount() const { return 1; }
    
    virtual void
    set1DInterpolant( const float4 &start, 
                      const float4 &end,
                      const unsigned int outputWidth,
                      GPUInterpolant *interpolant) const = 0;

    virtual void 
    set2DInterpolant( const float4 &start, 
                      const float4 &end,
                      const unsigned int outputWidth,
                      const unsigned int outputHeight, 
                      GPUInterpolant *interpolant) const = 0;

    virtual void 
    setStreamInterpolant( const TextureHandle texture,
                          const unsigned int outputWidth,
                          const unsigned int outputHeight, 
                          GPUInterpolant *interpolant) const = 0;
    
    virtual void
    setStreamOutputRegion( const TextureHandle texture,
                           GPUOutputRegion *region) const = 0; 


    /* Create a texture */
    virtual TextureHandle 
    createTexture2D( unsigned int inWidth, 
                     unsigned int inHeight, 
                     TextureFormat inFormat ) = 0;
    
    /* Free a texture */
    virtual void 
    releaseTexture( TextureHandle inTexture ) = 0;

    /* Load up the texture with data */
    virtual void 
    setTextureData( TextureHandle inTexture, 
                    const float* inData, 
                    unsigned int inStrideBytes, 
                    unsigned int inElemCount ) = 0;
    
    /* Fetch the texture data */
    virtual void 
    getTextureData( TextureHandle inTexture, 
                    float* outData, 
                    unsigned int inStrideBytes, 
                    unsigned int inElemCount ) = 0;
    
    /* Create a pixel shader */
    virtual PixelShaderHandle 
    createPixelShader( const char* inSource ) = 0;

    /* Handles to the passthrough shaders */
    virtual VertexShaderHandle getPassthroughVertexShader() = 0;
    virtual PixelShaderHandle  getPassthroughPixelShader() = 0;

    /* To use around rendering operations */
    virtual void beginScene() = 0;
    virtual void endScene() = 0;

    /* Binding commands */
    virtual void bindConstant( unsigned int inIndex, 
                               const float4& inValue ) = 0;
    virtual void bindTexture( unsigned int inIndex, 
                              TextureHandle inTexture ) = 0;
    virtual void bindOutput( unsigned int inIndex, 
                             SurfaceHandle inSurface ) = 0;
    virtual void bindPixelShader( PixelShaderHandle inPixelShader ) = 0;
    virtual void bindVertexShader( VertexShaderHandle inVertexShader ) = 0;
    
    /* Turn off a shader output */
    virtual void disableOutput( unsigned int inIndex ) = 0;

    virtual void drawRectangle(
      const GPUOutputRegion& outputRegion, 
      const GPUInterpolant* interpolants, 
      unsigned int numInterpolants ) = 0;
  };
}

#endif
