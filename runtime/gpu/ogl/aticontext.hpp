// aticontext.hpp

#pragma once

#include "oglcontext.hpp"
#include "ogltexture.hpp"


namespace brook {

  class ATITexture : public OGLTexture {
  public:

    ATITexture ( unsigned int inWidth, 
                unsigned int inHeight, 
                GPUContext::TextureFormat inFormat);
    
    virtual int nativeFormat() const { return _nativeFormat; }

  private:
    int _nativeFormat;
  };


  class ATIContext : public OGLContext
  {
  public:

    static ATIContext * create();
    
    TextureHandle 
    createTexture2D( unsigned int inWidth, 
                     unsigned int inHeight, 
                     TextureFormat inFormat);

    bool isValidShaderNameString (const char *name) const;

    static bool
    isCompatibleContext ();

    static bool
    isVendorContext ();
    
    
  protected:
    ATIContext();
  };
}

