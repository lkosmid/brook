// oglcontext.hpp
#ifndef OGLCONTEXT_HPP
#define OGLCONTEXT_HPP

#include "../gpucontext.hpp"

#include "oglfunc.hpp"

namespace brook {
   
  class OGLTexture;
  class OGLWindow;

  class OGLContext : public GPUContext
  {
  public:

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

    virtual float4 getStreamIndexofConstant( TextureHandle inTexture ) const;
    virtual float4 getStreamGatherConstant( TextureHandle inTexture ) const;

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
                          unsigned int rank,
                          const unsigned int* domainMin,
                          const unsigned int* domainMax,
                          const unsigned int outputWidth,
                          const unsigned int outputHeight, 
                          GPUInterpolant &interpolant) const;
    
    virtual void
    getStreamOutputRegion( const TextureHandle texture,
                           unsigned int rank,
                           const unsigned int* domainMin,
                           const unsigned int* domainMax,
                           GPURegion &region) const; 

    virtual void 
    getStreamReduceInterpolant( const TextureHandle texture,
                                const unsigned int outputWidth,
                                const unsigned int outputHeight, 
                                const unsigned int minX,
                                const unsigned int maxX, 
                                const unsigned int minY,
                                const unsigned int maxY,
                                GPUInterpolant &interpolant) const; 

    virtual void
    getStreamReduceOutputRegion( const TextureHandle texture,
                                const unsigned int minX,
                                const unsigned int maxX, 
                                const unsigned int minY,
                                const unsigned int maxY,
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
    releaseTexture( TextureHandle inTexture );

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

    /* Returns true if all of the GL extensions are 
    ** available for this context
    */
    static bool
    isCompatibleContext () { return false; }

    /* Returns true if all of the vendor string
    ** matches this context
    */
    static bool
    isVendorContext () { return false; }

    /* These are ARB programs */
    virtual VertexShaderHandle getPassthroughVertexShader();
    virtual PixelShaderHandle getPassthroughPixelShader();

    /* OGL does not need these */
    void beginScene() { }
    void endScene() { }

    /* These are the ARB versions */
    virtual void bindConstant( PixelShaderHandle ps, 
                               unsigned int inIndex, const float4& inValue );
    virtual void bindTexture( unsigned int inIndex, TextureHandle inTexture );
    virtual void bindOutput( unsigned int inIndex, TextureHandle inSurface );
    virtual void bindPixelShader( PixelShaderHandle inPixelShader );
    virtual void bindVertexShader( VertexShaderHandle inVertexShader );

    /* Not really sure what this should do... */
    virtual void disableOutput( unsigned int inIndex );

    virtual void drawRectangle( const GPURegion &outputRegion, 
                                const GPUInterpolant *interpolants, 
                                unsigned int numInterpolants );

    virtual ~OGLContext();

  protected:        
    OGLContext();

    /* Creates a context and pbuffer */
    void init(const int   (*viAttribList)[4][64],
              const float (*vfAttribList)[4][16],
              const int   (*vpiAttribList)[4][16]);

  private:
    VertexShaderHandle _passthroughVertexShader;
    PixelShaderHandle _passthroughPixelShader;
    OGLTexture *_outputTexture;
    unsigned int _slopTextureUnit;
    int currentPbufferComponents;

    void copy_to_pbuffer(OGLTexture *texture);

    OGLWindow *wnd;

  };

}


#endif

