// glestexture.hpp

#ifndef GLESTEXTURE_HPP
#define GLESTEXTURE_HPP

#include "../gpucontext.hpp"

namespace brook {

  class GLESContext;
   
  /* Virtual class for textures */
  class GLESTexture 
  {
  public:
    enum GLESElementFormats{
       GLES_FLOAT,
       GLES_SHORTFIXED,
       GLES_FIXED,
       GLES_CHAR,
       GLES_INT,
       GLES_NUMFORMATS
    };

    
    GLESTexture ( GLESContext *ctx, unsigned int width,
                 unsigned int height,
                 GPUContext::TextureFormat format);

    virtual ~GLESTexture ();

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
    copyToTextureFormat( const void *src, 
                         unsigned int inStrideBytes, 
                         unsigned int inElemCount,
                         void *dst) const;

    /* Same as copyToTextureFormat except for 
    ** copying the data out of the packed format
    */
    virtual void 
    copyFromTextureFormat( const void *src, 
                           unsigned int inStrideBytes, 
                           unsigned int inElemCount,
                           void *dst) const;


    /* Basic accessor functions */
    unsigned int width()                 const { return _width;    }
    unsigned int height()                const { return _height;   }
    unsigned int bytesize()              const { return _bytesize; }
    unsigned int atomsize()              const { return _atomsize; }
    unsigned int components()            const { return _components; }
    unsigned int elementType()            const { return _elementType; }
    unsigned int numInternalComponents() const {return _elemsize;}
    GPUContext::TextureFormat format()   const { return _format;   }
    unsigned int id()                    const { return _id;       }

    /* Returns the vendor specific "format" 
    ** parameter for glReadPixels and glTexImage2D
    ** for float textures.
    */
    int nativeFormat()       const { return _nativeFormat; }

    // Swaps the texture ids. Make SURE they are the same dimensions & format!
    void swap(GLESTexture &o) { unsigned int temp=_id; _id=o._id; o._id=temp; }

    // TIM: these are really janky functions...

    void getRectToCopy(
      unsigned int inRank, const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
      int& outMinX, int& outMinY, int& outMaxX, int& outMaxY, size_t& outBaseOffset, bool& outFullStream, bool inUsesAddressTranslation );

    void setATData(
      const void* inStreamData, unsigned int inStrideBytes, unsigned int inRank,
      const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
      void* ioTextureData );

    void getATData(
      void* outStreamData, unsigned int inStrideBytes, unsigned int inRank,
      const unsigned int* inDomainMin, const unsigned int* inDomainMax, const unsigned int* inExtents,
      const void* inTextureData );

    union __float_t_{	
       float f;
       unsigned int u32;
       struct{
          unsigned int mant:23;
          unsigned int exp:8;
          unsigned int sign:1;
       };
    };

    //Converts a floating point element from GPU to CPU representation
    void convert_fp_from_gpu(void *dst, const void *src) const;

    //Converts a floating point element from CPU to GPU representation
    void convert_fp_to_gpu(void *dst, const void *src) const;

  private:
    unsigned int _width, _height, _bytesize;
    unsigned int _components;
    GLESElementFormats _elementType;
    unsigned int _elemsize;
    unsigned int _atomsize;
    GPUContext::TextureFormat _format;
    unsigned int _id;
    unsigned int _nativeFormat;
  };

}
#endif


