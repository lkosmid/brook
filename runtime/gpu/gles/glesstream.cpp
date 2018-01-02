#include <string.h>
#include "glesfunc.hpp"
#include "glescontext.hpp"
#include "glestexture.hpp"
#include "gleswindow.hpp"
#include "glesfunc.hpp"
#include "glescheckgl.hpp"

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
static void fastglReadPixels(GLESContext *ctx,
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
static void mapTexture(GLESTexture *glesTexture)
{
  int id;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &id);
  CHECK_GL();

#ifdef GLES_DEBUG
  printf("GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE in map texture:%x\n", id);
#endif
  if(id)
  {
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &id);
  CHECK_GL();
#ifdef GLES_DEBUG
  printf("id:%d, glesTexture->id():%d\n", id, glesTexture->id());
#endif
  }
  if(id!=(int) glesTexture->id())
  {
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
           GL_TEXTURE_2D, glesTexture->id(), 0);
  CHECK_GL();
  }
#ifdef _DEBUG
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &id);
  CHECK_GL();
  printf("id:%d, glesTexture->id():%d\n", id, glesTexture->id());
  assert(id==glesTexture->id());
#endif
}

// Writes data into a texture
void GLESContext::writeToTexture(GLESTexture *glesTexture, GLint x, GLint y, GLint w, GLint h, const void *inData)
{
  unsigned int elemsize=glesTexture->numInternalComponents();//we're always reading from a float pbuffer, therefore we have to give it a reasonable constant for FLOAT, not for BYTE... luminance is wrong here.
  glActiveTexture(GL_TEXTURE0);
  CHECK_GL();
  glBindTexture (GL_TEXTURE_2D, glesTexture->id());
  CHECK_GL();
  if(!_havePBOs) {
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w , h,
#ifdef GLES3
                 /* glesTexture->nativeFormat(), // */ elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), 
#else
                 //in OpenGL ES 2 all formats except chars consist of multiple values
                 //so pass all components always 
                 GL_RGBA,
                 GL_UNSIGNED_BYTE, 
#endif
                 inData);
    CHECK_GL();
  }
#ifdef GLES3
  else {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _PBOs[_PBOcount]);
    CHECK_GL();
    glBufferData(GL_PIXEL_UNPACK_BUFFER, glesTexture->bytesize(), NULL, GL_STREAM_DRAW);
    CHECK_GL();
    void *mem = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    CHECK_GL();
    GPUAssert (mem, "Failed to map buffer into memory");
#ifdef GLES_PRINTOPS
    printf("Writing to texture %u via PBO %u at %p\n", glesTexture->id(), _PBOs[_PBOcount], mem);
#endif
    memcpy(mem, inData, glesTexture->bytesize());
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    CHECK_GL();
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, 
                 /* glesTexture->nativeFormat(), // */ elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), 0);
    CHECK_GL();
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    if(++_PBOcount>sizeof(_PBOs)/sizeof(_PBOcount)) _PBOcount=0;
  }
#endif
}


void 
GLESContext::setTextureData(TextureHandle inTexture, 
                           const float* inData,
                           unsigned int inStrideBytes,
                           unsigned int inComponentCount,
                           unsigned int inRank,
                           const unsigned int* inDomainMin,
                           const unsigned int* inDomainMax,
                           const unsigned int* inExtents, bool inUsesAddressTranslation ) {
  void *t;
  
  GLESTexture *glesTexture = (GLESTexture *) inTexture;
  
  int minX, minY, maxX, maxY;
  size_t baseOffset;
  bool fullStream;
  glesTexture->getRectToCopy( inRank, inDomainMin, inDomainMax, inExtents,
    minX, minY, maxX, maxY, baseOffset, fullStream, inUsesAddressTranslation );
  int rectW = maxX - minX;
  int rectH = maxY - minY;

  bool fastPath = glesTexture->isFastSetPath( inStrideBytes, 
                                             rectW, rectH,
                                             inComponentCount );
  fastPath = fastPath && !inUsesAddressTranslation;

  //TODO: enable fastPath processing if possible. For the moment we disable it:
  fastPath = false;

#ifdef GLES_PRINTOPS
  printf("Writing to texture %u\n", glesTexture->id());
#endif
  if (fastPath) {
	  assert(0);
    writeToTexture(glesTexture, minX, minY,
        rectW, //glesTexture->width(), 
        rectH, //glesTexture->height(), 
        inData);
  CHECK_GL();
    goto out;
  }
  
  // TIM: could improve this in the domain case
  // by only allocating as much memory as the
  // domain needs
  //In OpenGL ES we read all 4 components when input is not char
  t = brmalloc (glesTexture->bytesize());
  CHECK_GL();
  if( !fullStream && inUsesAddressTranslation )
  {
	  assert(0);
    // TIM: hack to get the texture data into our buffer
    int texW = glesTexture->width();
    int texH = glesTexture->height();
    unsigned int texDomainMin[] = {0,0};
    unsigned int texDomainMax[] = { texH, texW };
    unsigned int texExtents[] = { texH, texW };
    getTextureData( glesTexture,(float*) t, inStrideBytes, texW*texH, 2,
      texDomainMin, texDomainMax, texExtents, false );
  CHECK_GL();

    glesTexture->setATData(
      inData, inStrideBytes, inRank, inDomainMin, inDomainMax, inExtents, t );
  CHECK_GL();

    writeToTexture(glesTexture, 0, 0, 
        texW, //glesTexture->width(), 
        texH, //glesTexture->height(),  
        t);

  }
  else
  {
#ifdef GLES_DEBUG
	  printf("inStrideBytes:%d\n", inStrideBytes);
#endif
    glesTexture->copyToTextureFormat(inData, 
                                    inStrideBytes, 
                                    inComponentCount,
                                    t);

    writeToTexture(glesTexture, minX, minY, 
        maxX - minX, //glesTexture->width(), 
        maxY - minY, //glesTexture->height(),  
        t);
  }
  brfree(t); t=0;
out:
//#if defined(_DEBUG) && 0
  if(inStrideBytes==glesTexture->atomsize()*glesTexture->components())
  {
    //In OpenGL ES we read all 4 components when input is not char
    void *t2 = brmalloc (glesTexture->bytesize());
    getTextureData(inTexture, (float *) t2, inStrideBytes, inComponentCount, inRank, inDomainMin, inDomainMax, inExtents, inUsesAddressTranslation);
    for(unsigned int n=0; n<inComponentCount; n++)
	{
		if(glesTexture->elementType() == GLESTexture::GLES_FLOAT)
		{
			printf("inData[%d]:%f\n",n, /*(int)*/((float *)inData)[n]);
			printf("t2[%d]:%f\n",n, /*(int)*/((float *)t2)[n]);
        if(fabs(((float *)inData)[n]-((float *)t2)[n])>0.0001)
        {
			printf("inData[%d]:%d\n",n, (int)((float *)inData)[n]);
			printf("t2[%d]:%f\n",n, (int)((float *)t2)[n]);
            printf("Texture contents do not match what was just written to it!\n");
            abort();
        }
		}
		else if((glesTexture->elementType() == GLESTexture::GLES_CHAR)
		|| (glesTexture->elementType() == GLESTexture::GLES_FIXED))
		{
			printf("inData[%d]:%d\n",n, ((unsigned char *)inData)[n]);
			printf("t2[%d]:%d\n",n, ((unsigned char *)t2)[n]);
 			if(((unsigned char *)inData)[n] != ((unsigned char *)t2)[n])
 			{
			   printf("inData[%d]:%d\n",n, (int)((unsigned char *)inData)[n]);
			   printf("t2[%d]:%d\n",n, (int)((unsigned char *)t2)[n]);
 			   printf("Texture contents do not match what was just written to it!\n");
 			   abort();
 			}
 		}
		else if(glesTexture->elementType() == GLESTexture::GLES_INT)
		{
			printf("inData[%d]:%d\n",n, ((unsigned int *)inData)[n]);
			printf("t2[%d]:%d\n",n, ((unsigned int *)t2)[n]);
 			if(((unsigned int *)inData)[n] != ((unsigned int *)t2)[n])
 			{
			   printf("inData[%d]:%d\n",n, ((unsigned int *)inData)[n]);
			   printf("t2[%d]:%d\n",n, ((unsigned int *)t2)[n]);
 			   printf("Texture contents do not match what was just written to it!\n");
 			   abort();
 			}
 		}
		else 
		{
 			   printf("Unknown format, cannot ensure that texture contents match what was just written to it!\n");
 			   abort();
 		}
	}
	brfree(t2);
  }
//#endif
  CHECK_GL();
}

void 
GLESContext::getTextureData( TextureHandle inTexture,
                            float* outData,
                            unsigned int inStrideBytes,
                            unsigned int inComponentCount,
                            unsigned int inRank,
                            const unsigned int* inDomainMin,
                            const unsigned int* inDomainMax,
                            const unsigned int* inExtents, bool inUsesAddressTranslation ) {
   void *t = outData;

   GLESTexture *glesTexture = (GLESTexture *) inTexture;

   int minX, minY, maxX, maxY;
   size_t baseOffset;
   bool fullStream;
   glesTexture->getRectToCopy( inRank, inDomainMin, inDomainMax, inExtents,
     minX, minY, maxX, maxY, baseOffset, fullStream, inUsesAddressTranslation );
   int rectW = maxX - minX;
   int rectH = maxY - minY;
   
   bool fastPath = glesTexture->isFastGetPath( inStrideBytes, 
                                              rectW, rectH,
                                              inComponentCount); 
   if (!fastPath)
   {
     //In OpenGL ES we read all 4 components when input is not char
     t = brmalloc (glesTexture->bytesize());
   }

   glPixelStorei(GL_PACK_ALIGNMENT,1);
   CHECK_GL();
   // read back the whole thing, 
   unsigned int elemsize=glesTexture->numInternalComponents();//we're always reading from a float pbuffer, therefore we have to give it a reasonable constant for FLOAT, not for BYTE... luminance is wrong here.

#ifdef GLES_PRINTOPS
   printf("Reading from texture %u\n", glesTexture->id());
#endif
   _wnd->bindFBO();
   mapTexture(glesTexture);
#ifdef GLES3
   glReadBuffer(GL_COLOR_ATTACHMENT0);
#endif
   //fastglReadPixels(this,
   glReadPixels (minX, minY,
                 rectW,
                 rectH, 
#ifdef GLES3
                 /* glesTexture->nativeFormat(), // */ elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), 
#else
                 //In OpenGL ES 2.0 all outputs except chars have multiple channels, so read all of them
                 GL_RGBA,
                 GL_UNSIGNED_BYTE, 
#endif
                 t);
   CHECK_GL();

   if (!fastPath) {
     if( !inUsesAddressTranslation || fullStream)
     {
       glesTexture->copyFromTextureFormat(t, 
         inStrideBytes, 
         inComponentCount,
         outData);
     }
     else
     {
       glesTexture->getATData(outData, inStrideBytes,
         inRank, inDomainMin, inDomainMax, inExtents, t );
     }
     brfree(t);
   }
}


void 
GLESContext::releaseTexture( TextureHandle inTexture ) {
  delete (GLESTexture *) inTexture;
}




#if 0
#if 1
  glActiveTexture(GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_RECTANGLE, glesTexture->id());
 
  if (fastPath) {
    glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, minX, minY, 
                    rectW, //glesTexture->width(), 
                    rectH, //glesTexture->height(), 
                    elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                    glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), inData);
    return;
  }
  
  // TIM: could improve this in the domain case
  // by only allocating as much memory as the
  // domain needs
  t = brmalloc (glesTexture->bytesize());
  if( !fullStream && inUsesAddressTranslation )
  {
    // TIM: hack to get the texture data into our buffer
    int texW = glesTexture->width();
    int texH = glesTexture->height();
    unsigned int texDomainMin[] = {0,0};
    unsigned int texDomainMax[] = { texH, texW };
    unsigned int texExtents[] = { texH, texW };
    getTextureData( glesTexture,(float*) t, inStrideBytes, texW*texH, 2,
      texDomainMin, texDomainMax, texExtents, false );

    glesTexture->setATData(
      inData, inStrideBytes, inRank, inDomainMin, inDomainMax, inExtents, t );

    glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 
      texW, //glesTexture->width(), 
      texH, //glesTexture->height(),  
      elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                    glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);

  }
  else
  {
    glesTexture->copyToTextureFormat(inData, 
                                    inStrideBytes, 
                                    inComponentCount,
                                    t);

    glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, minX, minY, 
                    maxX - minX, //glesTexture->width(), 
                    maxY - minY, //glesTexture->height(),  
                    elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                    glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);
  }
#else
 
  if (fastPath) {
    _wnd->bindFBO();
    glDisable(GL_FRAGMENT_PROGRAM);
    mapTexture(glesTexture);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glRasterPos2i(minX, minY);
    glDrawPixels(rectW, //glesTexture->width(), 
                 rectH, //glesTexture->height(), 
                 elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), inData);
    glEnable(GL_FRAGMENT_PROGRAM);
    return;
  }
  
  // TIM: could improve this in the domain case
  // by only allocating as much memory as the
  // domain needs
  t = brmalloc (glesTexture->bytesize());
  if( !fullStream && inUsesAddressTranslation )
  {
    // TIM: hack to get the texture data into our buffer
    int texW = glesTexture->width();
    int texH = glesTexture->height();
    unsigned int texDomainMin[] = {0,0};
    unsigned int texDomainMax[] = { texH, texW };
    unsigned int texExtents[] = { texH, texW };
    getTextureData( glesTexture,(float*) t, inStrideBytes, texW*texH, 2,
      texDomainMin, texDomainMax, texExtents, false );

    glesTexture->setATData(
      inData, inStrideBytes, inRank, inDomainMin, inDomainMax, inExtents, t );

    _wnd->bindFBO();
    glDisable(GL_FRAGMENT_PROGRAM);
    mapTexture(glesTexture);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glRasterPos2i(0, 0);
    glDrawPixels(texW, //glesTexture->width(), 
                 texH, //glesTexture->height(),  
                 elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);

    glEnable(GL_FRAGMENT_PROGRAM);
  }
  else
  {
    glesTexture->copyToTextureFormat(inData, 
                                    inStrideBytes, 
                                    inComponentCount,
                                    t);

    _wnd->bindFBO();
    glDisable(GL_FRAGMENT_PROGRAM);
    mapTexture(glesTexture);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glRasterPos2i(minX, minY);
    glDrawPixels(maxX - minX, //glesTexture->width(), 
                 maxY - minY, //glesTexture->height(),  
                 elemsize==1?GL_RED:(elemsize==3?GL_RGB:GL_RGBA),
                 glesTexture->elementType()==GLESTexture::GLES_FIXED?GL_UNSIGNED_BYTE:(glesTexture->elementType()==GLESTexture::GLES_SHORTFIXED?GL_UNSIGNED_SHORT:GL_FLOAT), t);
    glEnable(GL_FRAGMENT_PROGRAM);
  }
#endif
#endif
