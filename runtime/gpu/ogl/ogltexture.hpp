// ogltexture.hpp

#ifndef OGLTEXTURE_HPP
#define OGLTEXTURE_HPP

#include "../gpucontext.hpp"

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
                   unsigned int inWidth,
                   unsigned int inHeight,
                   unsigned int inElemCount ) const;

    /* Same as isFastGetPath, but for
    ** returning data into glReadPixels
    */
    virtual bool
    isFastGetPath( unsigned int inStrideBytes, 
                   unsigned int inWidth,
                   unsigned int inHeight,
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

    // TIM: these are really janky functions...

    void getRectToCopy(
      unsigned int inRank, const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
      int& outMinX, int& outMinY, int& outMaxX, int& outMaxY, size_t& outBaseOffset, bool& outFullStream, bool inUsesAddressTranslation );

    void setATData(
      const float* inStreamData, unsigned int inStrideBytes, unsigned int inRank,
      const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
      float* ioTextureData );

    void getATData(
      float* outStreamData, unsigned int inStrideBytes, unsigned int inRank,
      const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
      const float* inTextureData );


  private:
    unsigned int _width, _height, _bytesize;
    unsigned int _components;
    unsigned int _elemsize;
    GPUContext::TextureFormat _format;
    unsigned int _id;
    unsigned int _nativeFormat;
  };

}

#endif


