
#include "oglcontext"

static  
void copy_to_pbuffer(OGLTexture *texture) {
  
} 

bool
OGLContext::isFastSetPath( size_t inStrideBytes, 
                           size_t inElemCount ) const {
   return (inStrideBytes == _components*sizeof(float) &&
           inElemCount   == _width*_height);
}

bool
OGLContext::isFastGetPath( size_t inStrideBytes, 
                           size_t inElemCount ) const {
   return (inStrideBytes == _components*sizeof(float) &&
           inElemCount   == _width*_height);
}


void 
OGLContext::setTextureData(TextureHandle inTexture, 
                           const float* inData,
                           size_t inStrideBytes,
                           size_t  ) {
   OGLTexture *oglTexture = (OGLTexture *) inTexture;
   
   bool fastPath = oglTexture->isFastSetPath( inStrideBytes, 
                                              inComponentCount); 
   if (fastPath)
      t = inData;
   else {
      t = (float *) malloc (oglTexture->bytesize());
      oglTexture->copyToTextureFormat(inData, 
                                      inStrideBytes, 
                                      inComponentCount,
                                      t);
   }

   glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 
                   oglTexture->width(), 
                   oglTexture->height(), 
                   oglTexture->nativeFormat(),
                   GL_FLOAT, t);

   if (!fastPath) {
           free(t);
   }
}

void 
OGLContext::getTextureData( TextureHandle inTexture,
                            float* outData,
                            size_t inStrideBytes,
                            size_t inComponentCount ) {

   OGLTexture *oglTexture = (OGLTexture *) inTexture;
   
   bool fastPath = oglTexture->isFastGetPath( inStrideBytes, 
                                              inComponenetCount); 
   if (fastPath)
      t = outData;
   else 
      t = (float *) malloc (oglTexture->bytesize());

   copyToPbuffer(oglTexture);
   
   glReadPixels (0, 0,
                 oglTexture->width(),
                 oglTexture->height(), 
                 oglTexture->format(),
                 GL_FLOAT, t);

   if (!fastPath) {
      oglTexture->copyFromTextureFormat(t, 
                                        inStrideBytes, 
                                        inComponentCount,
                                        outData);
      free(t);
   }
}


static const char passthrough_vertex[] = 
"put code here";

static const char passthrough_pixel[] =
"!!ARBfp1.0\n"
"ATTRIB tex0 = fragment.texcoord[0];\n"
"OUTPUT oColor = result.color;\n"
"TEX oColor, tex0, texture[0], RECT;\n"
"END\n";

VertexShaderHandle 
OGLContext::getPassthroughVertexShader() {
  if (!_passthroughVertexShader) {
       glGenProgramsARB(1, &_passthroughVertexShader);
       glBindProgramARB(GL_VERTEX_PROGRAM_ARB, passthrough_id);
       glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                          strlen(passthrough_vertex), 
                          (GLubyte *) passthrough_vertex);
       CHECK_GL();
  }
  return _passthroughVertexShader;
}


PixelShaderHandle 
OGLContext::getPassthroughPixelShader() {
  if (!_passthroughPixelShader) {
       glGenProgramsARB(1, &_passthroughPixelShader);
       glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, passthrough_id);
       glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                          strlen(passthrough_fragment), 
                          (GLubyte *) passthrough_fragment);
       CHECK_GL();
  }
  return _passthroughVertexShader;
}



void 
OGLContext::bindConstant( unsigned int inIndex, 
                          const float4& inValue ) {
  
  glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, inIndex,
                                (const float *) &inValue);
  CHECK_GL();
}


void 
OGLContext::bindTexture( unsigned int inIndex, 
                         TextureHandle inTexture ) {
  OGLTexture *oglTexture = (OGLTexture *) inTexture;
  
  GPUAssert(oglTexture, "Null Texture");
  
  glActiveTextureARB(GL_TEXTURE0_ARB+inIndex);
  glBindTexture(GL_TEXTURE_RECTANGLE_EXT, oglTexture->id());
  CHECK_GL();
}


void OGLContext::bindOutput( unsigned int inIndex, 
                             TextureHandle inSurface ) {
  OGLTexture *oglTexture = (OGLTexture *) inTexture;
  
  GPUAssert(oglTexture, "Null Texture");
  GPUAssert(inIndex == 0, "Backend does not support more than"
            " one shader output.");
  
  _outputTexture = oglTexture;
}

void 
OGLContext::bindPixelShader( PixelShaderHandle inPixelShader ) {
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 
                   (unsigned int) inPixelShader);
  CHECK_GL();
}

void 
OGLContext::bindVertexShader( VertexShaderHandle inVertexShader ) {
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 
                   (unsigned int) inVertexShader);
  CHECK_GL();
}

void OGLContext::disableOutput( unsigned int inIndex ) {
 GPUAssert(inIndex == 0, "Backend does not support more than"
           " one shader output.");

 _outputTexture = NULL;
}
