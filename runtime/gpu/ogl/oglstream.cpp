
#include "oglfunc.hpp"
#include "oglcontext.hpp"
#include "ogltexture.hpp"
#include "oglwindow.hpp"
#include "oglfunc.hpp"
#include "oglcheckgl.hpp"

using namespace brook;

#if 0
/* There was a case on ATI that storing in GL_RGBA meant that glReadPixels() was
horrendously slow and fetching each component individually was far faster. The
workaround of using GL_BGRA internally makes this code obsolete for now. */
template<typename type, unsigned int size> static void copyPixels(void *_dst, void *_src, unsigned int components)
{
    type *dst=(type *) _dst, *src=(type *) _src;
    for(unsigned int n=0; n<components; n++)
    {
        dst[0]=src[0];
        if(size>1) dst[1]=src[components];
        if(size>2) dst[2]=src[components*2];
        if(size>3) dst[3]=src[components*3];
        dst++; src++;
        if(size>1) dst++;
        if(size>2) dst++;
        if(size>3) dst++;
    }
}
// Transfers components individually to greatly accelerate glReadPixels
static void fastglReadPixels(OGLContext *ctx,
                 GLint x,
			     GLint y,
			     GLsizei width,
			     GLsizei height,
			     GLenum format,
			     GLenum type,
			     GLvoid *pixels)
{
	GLuint components=(GL_RGBA==format)?4:(GL_RGB==format)?3:(GL_LUMINANCE_ALPHA==format)?2:(GL_RED==format)?1:0;
    if(components>1 && ctx->onATI()) // && width*height<3072*3072)
    {   // ATI's glReadPixels() is far faster transferring components individually
        // due to some bug in the driver
	    GLvoid *componentdata;
	    GLuint n;
	    GLuint componentsize=(GL_FLOAT==type)?4:(GL_UNSIGNED_SHORT==type)?2:(GL_UNSIGNED_BYTE==type)?1:0;
	    GLuint componentdatasize=width*height*componentsize;
	    assert(components);
	    assert(componentsize);
  	    componentdata=brmalloc(components*componentdatasize);
	    assert(componentdata);
	    for(n=0; n<components; n++)
		    glReadPixels(x, y, width, height, GL_RED+n, type, (GLvoid *)(((char *)componentdata)+n*componentdatasize));
	    CHECK_GL();
        // For a fast copy routine, specialise each case
        if(4==componentsize)
            (GL_FLOAT==type)          ? copyPixels<float,         4>(pixels, componentdata, width*height) :
            (GL_UNSIGNED_SHORT==type) ? copyPixels<unsigned short,4>(pixels, componentdata, width*height) :
                                        copyPixels<unsigned char, 4>(pixels, componentdata, width*height);
        else if(3==componentsize)
            (GL_FLOAT==type)          ? copyPixels<float,         3>(pixels, componentdata, width*height) :
            (GL_UNSIGNED_SHORT==type) ? copyPixels<unsigned short,3>(pixels, componentdata, width*height) :
                                        copyPixels<unsigned char, 3>(pixels, componentdata, width*height);
        else if(2==componentsize)
            (GL_FLOAT==type)          ? copyPixels<float,         2>(pixels, componentdata, width*height) :
            (GL_UNSIGNED_SHORT==type) ? copyPixels<unsigned short,2>(pixels, componentdata, width*height) :
                                        copyPixels<unsigned char, 2>(pixels, componentdata, width*height);
        else
            (GL_FLOAT==type)          ? copyPixels<float,         1>(pixels, componentdata, width*height) :
            (GL_UNSIGNED_SHORT==type) ? copyPixels<unsigned short,1>(pixels, componentdata, width*height) :
                                        copyPixels<unsigned char, 1>(pixels, componentdata, width*height);
	    brfree(componentdata);
    }
    else
    	glReadPixels(x, y, width, height, format, type, pixels);
}
#endif

// Helper routine to map a given texture into the FBO
static void mapTexture(OGLTexture *oglTexture)
{
  int id;
  glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT, &id);
  if(id)
    glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT, &id);
  if(id!=(int) oglTexture->id())
    glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
           GL_TEXTURE_RECTANGLE_ARB, oglTexture->id(), 0);
  CHECK_GL();
#ifdef _DEBUG
  glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT, &id);
  assert(id==oglTexture->id());
#endif
}

// Writes data into a texture
void OGLContext::writeToTexture(OGLTexture *oglTexture, GLint x, GLint y, GLint w, GLint h, const void *inData)
{
  unsigned int elemsize=oglTexture->numInternalComponents();//we're always reading from a float pbuffer, therefore we have to give it a reasonable constant for FLOAT, not for BYTE... luminance is wrong here.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, oglTexture->id());
  if(!_havePBOs) {
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, x, y, w, h,
                 /* oglTexture->nativeFormat(), // */ elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), inData);
  }
  else {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, _PBOs[_PBOcount]);
    CHECK_GL();
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, oglTexture->bytesize(), NULL, GL_STREAM_DRAW);
    CHECK_GL();
    void *mem = glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
    CHECK_GL();
    GPUAssert (mem, "Failed to map buffer into memory");
#ifdef OGL_PRINTOPS
    printf("Writing to texture %u via PBO %u at %p\n", oglTexture->id(), _PBOs[_PBOcount], mem);
#endif
    memcpy(mem, inData, oglTexture->bytesize());
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);
    CHECK_GL();
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, x, y, w, h, 
                 /* oglTexture->nativeFormat(), // */ elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), 0);
    CHECK_GL();
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    if(++_PBOcount>sizeof(_PBOs)/sizeof(_PBOcount)) _PBOcount=0;
  }
}


void 
OGLContext::setTextureData(TextureHandle inTexture, 
                           const float* inData,
                           unsigned int inStrideBytes,
                           unsigned int inComponentCount,
                           unsigned int inRank,
                           const unsigned int* inDomainMin,
                           const unsigned int* inDomainMax,
                           const unsigned int* inExtents, bool inUsesAddressTranslation ) {
  void *t;
  
  OGLTexture *oglTexture = (OGLTexture *) inTexture;
  
  int minX, minY, maxX, maxY;
  size_t baseOffset;
  bool fullStream;
  oglTexture->getRectToCopy( inRank, inDomainMin, inDomainMax, inExtents,
    minX, minY, maxX, maxY, baseOffset, fullStream, inUsesAddressTranslation );
  int rectW = maxX - minX;
  int rectH = maxY - minY;

  bool fastPath = oglTexture->isFastSetPath( inStrideBytes, 
                                             rectW, rectH,
                                             inComponentCount );
  fastPath = fastPath && !inUsesAddressTranslation;

#ifdef OGL_PRINTOPS
  printf("Writing to texture %u\n", oglTexture->id());
#endif
  if (fastPath) {
    writeToTexture(oglTexture, minX, minY,
        rectW, //oglTexture->width(), 
        rectH, //oglTexture->height(), 
        inData);
    goto out;
  }
  
  // TIM: could improve this in the domain case
  // by only allocating as much memory as the
  // domain needs
  t = brmalloc (oglTexture->bytesize());
  if( !fullStream && inUsesAddressTranslation )
  {
    // TIM: hack to get the texture data into our buffer
    int texW = oglTexture->width();
    int texH = oglTexture->height();
    unsigned int texDomainMin[] = {0,0};
    unsigned int texDomainMax[] = { texH, texW };
    unsigned int texExtents[] = { texH, texW };
    getTextureData( oglTexture,(float*) t, inStrideBytes, texW*texH, 2,
      texDomainMin, texDomainMax, texExtents, false );

    oglTexture->setATData(
      inData, inStrideBytes, inRank, inDomainMin, inDomainMax, inExtents, t );

    writeToTexture(oglTexture, 0, 0, 
        texW, //oglTexture->width(), 
        texH, //oglTexture->height(),  
        t);

  }
  else
  {
    oglTexture->copyToTextureFormat(inData, 
                                    inStrideBytes, 
                                    inComponentCount,
                                    t);

    writeToTexture(oglTexture, minX, minY, 
        maxX - minX, //oglTexture->width(), 
        maxY - minY, //oglTexture->height(),  
        t);
  }
  brfree(t); t=0;
out:
#if defined(_DEBUG) && 0
  CHECK_GL();
  if(inStrideBytes==oglTexture->atomsize()*oglTexture->components())
  {
    void *t2 = brmalloc (oglTexture->bytesize());
    getTextureData(inTexture, (float *) t2, inStrideBytes, inComponentCount, inRank, inDomainMin, inDomainMax, inExtents, inUsesAddressTranslation);
    for(unsigned int n=0; n<inComponentCount; n++)
        if(fabs(((float *)inData)[n]-((float *)t2)[n])>0.0001)
        {
            printf("Texture contents do not match what was just written to it!\n");
            abort();
        }
	brfree(t2);
  }
#endif
  CHECK_GL();
}

void 
OGLContext::getTextureData( TextureHandle inTexture,
                            float* outData,
                            unsigned int inStrideBytes,
                            unsigned int inComponentCount,
                            unsigned int inRank,
                            const unsigned int* inDomainMin,
                            const unsigned int* inDomainMax,
                            const unsigned int* inExtents, bool inUsesAddressTranslation ) {
   void *t = outData;

   OGLTexture *oglTexture = (OGLTexture *) inTexture;

   int minX, minY, maxX, maxY;
   size_t baseOffset;
   bool fullStream;
   oglTexture->getRectToCopy( inRank, inDomainMin, inDomainMax, inExtents,
     minX, minY, maxX, maxY, baseOffset, fullStream, inUsesAddressTranslation );
   int rectW = maxX - minX;
   int rectH = maxY - minY;
   
   bool fastPath = oglTexture->isFastGetPath( inStrideBytes, 
                                              rectW, rectH,
                                              inComponentCount); 
   if (!fastPath)
     t = brmalloc (oglTexture->bytesize());

   glPixelStorei(GL_PACK_ALIGNMENT,1);
   // read back the whole thing, 
   unsigned int elemsize=oglTexture->numInternalComponents();//we're always reading from a float pbuffer, therefore we have to give it a reasonable constant for FLOAT, not for BYTE... luminance is wrong here.

#ifdef OGL_PRINTOPS
   printf("Reading from texture %u\n", oglTexture->id());
#endif
   _wnd->bindFBO();
   mapTexture(oglTexture);
   glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
   //fastglReadPixels(this,
   glReadPixels (minX, minY,
                 rectW,
                 rectH, 
                 /* oglTexture->nativeFormat(), // */ elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);
   CHECK_GL();

   if (!fastPath) {
     if( !inUsesAddressTranslation || fullStream)
     {
       oglTexture->copyFromTextureFormat(t, 
         inStrideBytes, 
         inComponentCount,
         outData);
     }
     else
     {
       oglTexture->getATData(outData, inStrideBytes,
         inRank, inDomainMin, inDomainMax, inExtents, t );
     }
     brfree(t);
   }
  CHECK_GL();
}


void 
OGLContext::releaseTexture( TextureHandle inTexture ) {
  delete (OGLTexture *) inTexture;
}




#if 0
#if 1
  glActiveTexture(GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, oglTexture->id());
 
  if (fastPath) {
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, minX, minY, 
                    rectW, //oglTexture->width(), 
                    rectH, //oglTexture->height(), 
                    elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                    oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), inData);
    return;
  }
  
  // TIM: could improve this in the domain case
  // by only allocating as much memory as the
  // domain needs
  t = brmalloc (oglTexture->bytesize());
  if( !fullStream && inUsesAddressTranslation )
  {
    // TIM: hack to get the texture data into our buffer
    int texW = oglTexture->width();
    int texH = oglTexture->height();
    unsigned int texDomainMin[] = {0,0};
    unsigned int texDomainMax[] = { texH, texW };
    unsigned int texExtents[] = { texH, texW };
    getTextureData( oglTexture,(float*) t, inStrideBytes, texW*texH, 2,
      texDomainMin, texDomainMax, texExtents, false );

    oglTexture->setATData(
      inData, inStrideBytes, inRank, inDomainMin, inDomainMax, inExtents, t );

    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 
      texW, //oglTexture->width(), 
      texH, //oglTexture->height(),  
      elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                    oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);

  }
  else
  {
    oglTexture->copyToTextureFormat(inData, 
                                    inStrideBytes, 
                                    inComponentCount,
                                    t);

    glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, minX, minY, 
                    maxX - minX, //oglTexture->width(), 
                    maxY - minY, //oglTexture->height(),  
                    elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                    oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);
  }
#else
 
  if (fastPath) {
    _wnd->bindFBO();
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    mapTexture(oglTexture);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glRasterPos2i(minX, minY);
    glDrawPixels(rectW, //oglTexture->width(), 
                 rectH, //oglTexture->height(), 
                 elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), inData);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    return;
  }
  
  // TIM: could improve this in the domain case
  // by only allocating as much memory as the
  // domain needs
  t = brmalloc (oglTexture->bytesize());
  if( !fullStream && inUsesAddressTranslation )
  {
    // TIM: hack to get the texture data into our buffer
    int texW = oglTexture->width();
    int texH = oglTexture->height();
    unsigned int texDomainMin[] = {0,0};
    unsigned int texDomainMax[] = { texH, texW };
    unsigned int texExtents[] = { texH, texW };
    getTextureData( oglTexture,(float*) t, inStrideBytes, texW*texH, 2,
      texDomainMin, texDomainMax, texExtents, false );

    oglTexture->setATData(
      inData, inStrideBytes, inRank, inDomainMin, inDomainMax, inExtents, t );

    _wnd->bindFBO();
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    mapTexture(oglTexture);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glRasterPos2i(0, 0);
    glDrawPixels(texW, //oglTexture->width(), 
                 texH, //oglTexture->height(),  
                 elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);

    glEnable(GL_FRAGMENT_PROGRAM_ARB);
  }
  else
  {
    oglTexture->copyToTextureFormat(inData, 
                                    inStrideBytes, 
                                    inComponentCount,
                                    t);

    _wnd->bindFBO();
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    mapTexture(oglTexture);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glRasterPos2i(minX, minY);
    glDrawPixels(maxX - minX, //oglTexture->width(), 
                 maxY - minY, //oglTexture->height(),  
                 elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 oglTexture->elementType()==OGLTexture::OGL_FIXED?GL_UNSIGNED_BYTE:(oglTexture->elementType()==OGLTexture::OGL_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
  }
#endif
#endif
