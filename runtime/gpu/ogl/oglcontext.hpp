// oglcontext.hpp
#ifndef OGLCONTEXT_HPP
#define OGLCONTEXT_HPP
#include <string>
#include "../gpucontext.hpp"

#include "oglfunc.hpp"
#ifndef WIN32
typedef void *HGLRC; 
#endif

//#define OGL_PRINTOPS


namespace brook {
   
  class OGLTexture;
  class OGLWindow;

  class OGLPixelShader
  {
  protected:
    OGLPixelShader(unsigned int _id, const char *_program_string);
    virtual ~OGLPixelShader() { }
  public:
    virtual void bindConstant( unsigned int inIndex, const float4& inValue ) = 0;
    virtual void bindPixelShader() = 0;

    static const unsigned int MAXCONSTANTS = 256;
    unsigned int id;
	const char *program_string;
    float4 constants[256];
    std::string constant_names[256];
    unsigned int largest_constant;
  };
  class OGLARBPixelShader : public OGLPixelShader
  {
  public:
    OGLARBPixelShader(unsigned int id, const char *program_string);
    ~OGLARBPixelShader();
    virtual void bindConstant( unsigned int inIndex, const float4& inValue );
    virtual void bindPixelShader();
  };

  class OGLSLPixelShader : public OGLPixelShader
  {
    unsigned int programid;
    std::string constant_types[256], sampler_names[32];
  public:
    OGLSLPixelShader(unsigned int id, const char *program_string);
    ~OGLSLPixelShader();
    virtual void bindConstant( unsigned int inIndex, const float4& inValue );
    virtual void bindPixelShader();
  };


  class OGLContext : public GPUContext
  {
  public:
    static OGLContext * create(const char* device);

    bool onATI() const { return _isATI; }
    bool onNVidia() const { return _isNVidia; }
    virtual bool isTextureExtentValid( unsigned int inExtent ) const;
    virtual unsigned int getMaximumOutputCount() const;

    virtual float4 getStreamIndexofConstant( TextureHandle inTexture ) const;
    virtual float4 getStreamGatherConstant(
      unsigned int inRank, const unsigned int* inDomainMin,
      const unsigned int* inDomainMax, const unsigned int* inExtents ) const;

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
    getStreamPassThroughInterpolant( const TextureHandle texture,
    const unsigned int Width,
    const unsigned int Height, 
    GPUInterpolant &interpolant) const
    {
       getStreamReduceInterpolant( texture, Width, Height, 
       0, Width, 0, Height, interpolant); 
    }

    virtual void
    getStreamReduceOutputRegion( const TextureHandle texture,
                                const unsigned int minX,
                                const unsigned int maxX, 
                                const unsigned int minY,
                                const unsigned int maxY,
                                GPURegion &region) const; 

    virtual bool
    needsNormalizedTexCoords(void) const { return false; }

    virtual TextureHandle 
    createTexture2D( unsigned int inWidth,
                     unsigned int inHeight,
                     TextureFormat inFormat,
                     bool read_only=false);
    
    virtual int getShaderFormatRank (const char *name) const;

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
                    unsigned int inElemCount,
                    unsigned int inRank,
                    const unsigned int* inDomainMin,
                    const unsigned int* inDomainMax,
                    const unsigned int* inExtents, bool inUsesAddressTranslation );

    void 
    getTextureData( TextureHandle inTexture,
                    float* outData,
                    unsigned int inStrideBytes,
                    unsigned int inElemCount,
                    unsigned int inRank,
                    const unsigned int* inDomainMin,
                    const unsigned int* inDomainMax,
                    const unsigned int* inExtents, bool inUsesAddressTranslation );

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
    virtual VertexShaderHandle getPassthroughVertexShader( const char* inShaderFormat = NULL );
    virtual PixelShaderHandle getPassthroughPixelShader( const char* inShaderFormat = NULL );

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

    virtual void disableOutput( unsigned int inIndex );

    virtual void setAddressTranslationMode( bool inUsingAddressTranslation ) {
      _isUsingAddressTranslation = inUsingAddressTranslation;
    }

    virtual void setOutputDomainMode( bool inUsingOutputDomain ) {
      _isUsingOutputDomain = inUsingOutputDomain;
    }

    virtual void drawRectangle( const GPURegion &outputRegion, 
                                const GPUInterpolant *interpolants, 
                                unsigned int numInterpolants );

    /* hacky functions for rendering - will be deprecated soon */
    virtual void* getTextureRenderData( TextureHandle inTexture );

    virtual void synchronizeTextureRenderData( TextureHandle inTexture ) {
    }

    void finish();
    void unbind();
    void bind();
    IWriteQuery *createWriteQuery();

    void shareLists(HGLRC                     inContext );

    virtual ~OGLContext();

  protected:        
    OGLContext();

    /* Creates a context and FBO */
    void init(const char* device);

  private:
    bool _isATI, _isNVidia, _havePBOs, _supportsFP40;
    VertexShaderHandle _passthroughVertexShader;
    OGLPixelShader *_passthroughPixelShader;
    OGLTexture *_outputTextures[32], *_outputTexturesCache[32];
    unsigned int _slopTextureUnit;
    unsigned int _maxTextureExtent, _maxOutputCount;
    
    static const int MAXBOUNDTEXTURES = 32;
    OGLTexture *_boundTextures[32];
    OGLPixelShader *_boundPixelShader;

    GLuint _PBOs[32], _PBOcount;

    OGLWindow *_wnd;
    bool _isUsingAddressTranslation, _isUsingOutputDomain;

    void writeToTexture(OGLTexture *oglTexture, GLint x, GLint y, GLint w, GLint h, const void *inData);
  };

}


#endif

