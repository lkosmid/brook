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
    virtual OGLTexture ( unsigned int inWidth, 
                         unsigned int inHeight, 
                         TextureFormat inFormat) = 0;
    virtual ~OGLTexture () {} = 0;

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
    unsigned int width()     const { return _width;    }
    unsigned int height()    const { return _height;   }
    unsigned int bytesize()  const { return _bytesize; }
    TextureFormat format()   const { return _format;   }
    unsigned int id()        const { return _id;       }

    /* Returns the vendor specific "format" 
    ** parameter for glReadPixels and glTexImage2D
    ** for float textures.
    */  
    virtual int nativeFormat() const = 0;

  private:
    unsigned int _width, _height, _bytesize;
    unsigned int _components;
    TextureFormat _format;
    unsigned int _id;
  }    


  class OGLContext : public GPUContext
  {
  public:
    
    /* Creates a vendor specific backend */
    static OGLContext *oglContextFactory(void);

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

    /* Each samples slightly differently so these are pure virtual */
    virtual GPURect 
    getStreamPositionInterpolant( TextureHandle* texture
                                  TextureHandle* dstTexture = NULL) = 0;
    
    virtual GPURect 
    getStreamOutputRectangle( TextureHandle* texture ) = 0;

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
      delete (OGLTexture) inTexture;
    }

    /* Calls glTexSubImage to set the texture data
    */
    void 
    setTextureData( TextureHandle inTexture,
                    const float* inData,
                    unsigned int inStrideBytes,
                    unsigned int inElemCount );

    /* Reads back the texture data by rendering to the pbuffer and
    ** calling glReadPixels
    */
    void 
    getTextureData( TextureHandle inTexture,
                    float* outData,
                    unsigned int inStrideBytes,
                    unsigned int inElemCount );

    /* Obviously a vendor specific operation */
    virtual PixelShaderHandle 
    createPixelShader( const char* inSource ) = 0;

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

    virtual void drawRectangle(
      const GPUFatRect& inVertexRectangle, 
      const GPUFatRect* inTextureRectangles, 
      unsigned int inTextureRectangleCount ) = 0;

  private:
    VertexShaderHandle _passthroughVertexShader;
    PixelShaderHandle _passthroughPixelShader;
    OGLTexture _outputTexture;
  };
}

#endif
