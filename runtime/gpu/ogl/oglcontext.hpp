// gpucontext.hpp
#ifndef OGL_CONTEXT_HPP
#define OGL_CONTEXT_HPP

#include "../gpucontext.hpp"

namespace brook {
   
  class OGLTexture;
  class OGLContext;
  
  /* Virtual class for textures */
  class OGLTexture {
  public:
    
    /* Since there is no standard float texture 
    ** this constructor is pure virtual. 
    */
    virtual OGLTexture ( size_t inWidth, 
                         size_t inHeight, 
                         TextureFormat inFormat) = 0;
    virtual ~OGLTexture () {} = 0;

    /* This function tests if a data pointer 
    ** with a particular element stride and 
    ** size can be directly passed into 
    ** glTexSubImage.
    */
    virtual bool
    isFastSetPath( size_t inStrideBytes, 
                   size_t inElemCount ) const;

    /* Same as isFastGetPath, but for
    ** returning data into glReadPixels
    */
    virtual bool
    isFastGetPath( size_t inStrideBytes, 
                   size_t inElemCount ) const;

    /* Perform a copy into the packed 
    ** texture format.  Individual backends
    ** may override this in case the masquerade
    ** float2 as RGBA texture, etc.
    */
    virtual void 
    copyToTextureFormat( const float *src, 
                         size_t inStrideBytes, 
                         size_t inElemCount,
                         float *dst) const;

    /* Same as copyToTextureFormat except for 
    ** copying the data out of the packed format
    */
    virtual void 
    copyFromTextureFormat( const float *src, 
                           size_t inStrideBytes, 
                           size_t inElemCount,
                           float *dst) const;

    /* Basic accessor functions */
    size_t width()         const { return _width;    }
    size_t height()        const { return _height;   }
    size_t bytesize()      const { return _bytesize; }
    TextureFormat format() const { return _format;   }

    /* Returns the vendor specific "format" 
    ** parameter for float textures.
    */  
    virtual int nativeFormat() const = 0;

  private:
    size_t _width, _height, _bytesize;
    size_t _components;
    TextureFormat _format;
  }    


  class OGLContext : public GPUContext
  {
  public:
    
    /* Creates a vendor specific backend */
    static OGLContext *oglContextFactory(void);

    /* Everybody supports at least 2048 
    ** Specific backends can change this if they want...
    */
    virtual bool 
    isTextureExtentValid( unsigned int inExtent ) 
    { return inExtent <= 2048; }

    virtual GPURect getStreamPositionInterpolant( TextureHandle* texture ) = 0;
    virtual GPURect getStreamOutputRectangle( TextureHandle* texture ) = 0;


    /* The vendor specific backend must create
    ** the float textures since there are no
    ** standard float textures. Hence, pure
    ** virtual 
    */
    virtual TextureHandle 
    createTexture2D( unsigned int inWidth, 
                     unsigned int inHeight, 
                     TextureFormat inFormat) = 0;
    
    /* I assume that the virtual deconstructor
    ** should do the right thing
    */
    virtual void 
    releaseTexture( TextureHandle inTexture ) {
      delete (OGLTexture) inTexture;
    }

    /* Calls glTexSubImage to set the texture
    ** data
    */
    void 
    setTextureData( TextureHandle inTexture,
                    const float* inData,
                    unsigned int inStrideBytes,
                    unsigned int inElemCount );

    /* Reads back the texture data by rendering
    ** to the pbuffer and calling glReadPixels
    */
    void 
    getTextureData( TextureHandle inTexture,
                    float* outData,
                    unsigned int inStrideBytes,
                    unsigned int inElemCount );

    /* Obviously a vendor specific operation */
    virtual PixelShaderHandle 
    createPixelShader( const char* inSource ) = 0;

    /* These can be ARB programs */
    virtual VertexShaderHandle getPassthroughVertexShader();
    virtual PixelShaderHandle getPassthroughPixelShader();

    /* OGL does not need these */
    void beginScene() { }
    void endScene() { }

    virtual void bindConstant( unsigned int inIndex, const float4& inValue ) = 0;
    virtual void bindTexture( unsigned int inIndex, TextureHandle inTexture ) = 0;
    virtual void bindOutput( unsigned int inIndex, SurfaceHandle inSurface ) = 0;
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
