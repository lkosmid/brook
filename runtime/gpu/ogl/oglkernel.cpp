
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#include "oglcontext.hpp"
#include "oglfunc.hpp"
#include "oglcheckgl.hpp"
#include "ogltexture.hpp"
#include "oglwindow.hpp"

using namespace brook;

static const char passthrough_vertex[] = 
"not used";

static const char passthrough_pixel[] =
"!!ARBfp1.0\n"
"ATTRIB tex0 = fragment.texcoord[0];\n"
"OUTPUT oColor = result.color;\n"
"TEX oColor, tex0, texture[0], RECT;\n"
"END\n";


GPUContext::VertexShaderHandle 
OGLContext::getPassthroughVertexShader(void) {
#if 0
  if (!_passthroughVertexShader) {
    GLuint id;
    glGenProgramsARB(1, &id);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, id);
    glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(passthrough_vertex), 
                       (GLubyte *) passthrough_vertex);
    CHECK_GL();
    _passthroughVertexShader = (GPUContext::VertexShaderHandle) id;
  }
  return _passthroughVertexShader;
#else
  return (GPUContext::VertexShaderHandle) 1;
#endif
}

GPUContext::PixelShaderHandle 
OGLContext::getPassthroughPixelShader() {
  if (!_passthroughPixelShader) {
    GLuint id;
    glGenProgramsARB(1, &id);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, id);
    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                          strlen(passthrough_pixel), 
                          (GLubyte *) passthrough_pixel);
    _passthroughPixelShader = (GPUContext::PixelShaderHandle) id;
    CHECK_GL();
  }
  return _passthroughPixelShader;
}

GPUContext::PixelShaderHandle
OGLContext::createPixelShader( const char* shader ) 
{
  unsigned int id;

  // Allocate ids
  glGenProgramsARB(1, &id);
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, id);

  // Try loading the program
  glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, 
                     GL_PROGRAM_FORMAT_ASCII_ARB,
                     strlen(shader), (GLubyte *) shader);
  
  /* Check for program errors */
  if (glGetError() == GL_INVALID_OPERATION) {
    GLint pos;
    int i;
    int line, linestart;
    char *progcopy;

    progcopy = strdup (shader);
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &pos);
    
    line = 1;
    linestart = 0;
    for (i=0; i<pos; i++) {
      if (progcopy[i] == '\n') {
        line++;
        linestart = i+1;
      }
    }
    fprintf ( stderr, "GL: Program Error on line %d\n", line);
    for (i=linestart; progcopy[i] != '\0' && progcopy[i] != '\n'; i++);
    progcopy[i] = '\0';
    fprintf ( stderr, "%s\n", progcopy+linestart);
    for (i=linestart; i<pos; i++)
      fprintf ( stderr, " ");
    for (;progcopy[i] != '\0' && progcopy[i] != '\n'; i++)
      fprintf ( stderr, "^");
    fprintf ( stderr, "\n");
    free(progcopy);
    fprintf ( stderr, "%s\n",
              glGetString(GL_PROGRAM_ERROR_STRING_ARB));
    fflush(stderr);
    assert(0);
    exit(1);
  }

  return (GPUContext::PixelShaderHandle) id;
}

void 
OGLContext::bindConstant( PixelShaderHandle ps,
                          unsigned int inIndex, 
                          const float4& inValue ) {
  
  bindPixelShader(ps);
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
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, oglTexture->id());
  CHECK_GL();
}


void OGLContext::bindOutput( unsigned int inIndex, 
                             TextureHandle inTexture ) {
  OGLTexture *oglTexture = (OGLTexture *) inTexture;
  
  GPUAssert(oglTexture, "Null Texture");
  GPUAssert(inIndex == 0, "Backend does not support more than"
            " one shader output.");
  
  _outputTexture = oglTexture;
}

void 
OGLContext::bindPixelShader( GPUContext::PixelShaderHandle inPixelShader ) {
  glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 
                   (unsigned int) inPixelShader);
  CHECK_GL();
}

void 
OGLContext::bindVertexShader( GPUContext::VertexShaderHandle inVertexShader ) {
#if 0
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 
                   (unsigned int) inVertexShader);
  CHECK_GL();
#endif
}

void OGLContext::disableOutput( unsigned int inIndex ) {
 GPUAssert(inIndex == 0, "Backend does not support more than"
           " one shader output.");

 _outputTexture = NULL;
}

void
OGLContext::get1DInterpolant( const float4 &start, 
                              const float4 &end,
                              const unsigned int w,
                              GPUInterpolant &interpolant) const {

  if (w == 1) {
    interpolant.vertices[0] = start;
    interpolant.vertices[1] = start;
    interpolant.vertices[2] = start;
    return;
  }

  float4 f1, f2;
  float bias = 0.00001f;
  
  float x1 = start.x;
  float y1 = start.y;
  float z1 = start.z;
  float w1 = start.w;

  float x2 = end.x;
  float y2 = end.y;
  float z2 = end.z;
  float w2 = end.w;

  float sx = x2-x1;
  float sy = y2-y1;
  float sz = z2-z1;
  float sw = w2-w1;
  float ratiox = sx / w;
  float ratioy = sy / w;
  float ratioz = sz / w;
  float ratiow = sw / w;
  float shiftx = ratiox * 0.5f;
  float shifty = ratioy * 0.5f;
  float shiftz = ratioz * 0.5f;
  float shiftw = ratiow * 0.5f;

  f1.x = x1 - shiftx + bias;
  f1.y = y1 - shifty + bias;
  f1.z = z1 - shiftz + bias;
  f1.w = w1 - shiftw + bias;

  f2.x = (x1+2*sx) - shiftx + bias;
  f2.y = (y1+2*sy) - shifty + bias;
  f2.z = (z1+2*sz) - shiftz + bias;
  f2.w = (w1+2*sw) - shiftw + bias;

  interpolant.vertices[0] = f1;
  interpolant.vertices[1] = f2; 
  interpolant.vertices[2] = f1;
}


void
OGLContext::get2DInterpolant( const float2 &start, 
                              const float2 &end,
                              const unsigned int w,
                              const unsigned int h,
                              GPUInterpolant &interpolant) const {
  float2 f1, f2;
  
  float x1 = start.x;
  float y1 = start.y;
  float x2 = end.x;
  float y2 = end.y;
  float bias = 0.00001f;
  
  if (w==1 && h==1) {
    float4 v (start.x, start.y, 0.0f, 1.0f);
    interpolant.vertices[0] = v;
    interpolant.vertices[1] = v;
    interpolant.vertices[2] = v;
    return;
  }

  float sx = x2-x1;
  float sy = y2-y1;
  float ratiox = sx / w;
  float ratioy = sy / h;
  float shiftx = ratiox * 0.5f;
  float shifty = ratioy * 0.5f;

  f1.x = x1 - shiftx + bias;
  f1.y = y1 - shifty + bias;

  f2.x = (x1+2*sx) - shiftx + bias;
  f2.y = (y1+2*sy) - shifty + bias;

  if (h==1) {
//    interpolant.vertices[0] = float4(f1.x, f1.y, 0.0f, 1.0f);
//    interpolant.vertices[1] = float4(f2.x, f1.y, 0.0f, 1.0f);
//    interpolant.vertices[2] = interpolant.vertices[0];
    interpolant.vertices[0] = float4(f1.x, y1, 0.0f, 1.0f);
    interpolant.vertices[1] = float4(f2.x, y1, 0.0f, 1.0f);
    interpolant.vertices[2] = interpolant.vertices[0];
    return;
  }

  if (w==1) {
//    interpolant.vertices[0] = float4(f1.x, f1.y, 0.0f, 1.0f);
//    interpolant.vertices[1] = interpolant.vertices[0];
//    interpolant.vertices[2] = float4(f1.x, f2.y, 0.0f, 1.0f);
    interpolant.vertices[0] = float4(x1, f1.y, 0.0f, 1.0f);
    interpolant.vertices[1] = interpolant.vertices[0];
    interpolant.vertices[2] = float4(x1, f2.y, 0.0f, 1.0f);
    return;
  }

  interpolant.vertices[0] = float4(f1.x, f1.y, 0.0f, 1.0f);
  interpolant.vertices[1] = float4(f2.x, f1.y, 0.0f, 1.0f);
  interpolant.vertices[2] = float4(f1.x, f2.y, 0.0f, 1.0f);
}



float4 
OGLContext::getStreamIndexofConstant( TextureHandle inTexture ) const {
  return float4(1.0f, 1.0f, 0.0f, 0.0f);
}


float4
OGLContext::getStreamGatherConstant( TextureHandle inTexture ) const {
  return float4(1.0f, 1.0f, 0.005f, 0.005f);
}


void
OGLContext::getStreamInterpolant( const TextureHandle texture,
                                  const unsigned int w,
                                  const unsigned int h,
                                  GPUInterpolant &interpolant) const {

  OGLTexture *oglTexture = (OGLTexture *) texture;

  float2 start(0.005f, 0.005f);
  float2 end(oglTexture->width()+0.005f, 
             oglTexture->height()+0.005f);

  get2DInterpolant(  start, end, w, h, interpolant); 
}

void
OGLContext::getStreamOutputRegion( const TextureHandle texture,
                                   GPURegion &region) const {
  
/*  const OGLTexture *oglTexture = (OGLTexture *) texture;

  region.vertices[0].x = 0.0f;
  region.vertices[0].y = 0.0f;

  region.vertices[1].x = oglTexture->width()*2.0f;
  region.vertices[1].y = 0.0f;

  region.vertices[2].x = 0.0f;
  region.vertices[2].y = oglTexture->height()*2.0f;*/

  // note to future generations: Ian Buck is a lying bastard
  // the above code is just about as wrong as you can get :)
  region.vertices[0].x = -1.0f;
  region.vertices[0].y = -1.0f;

  region.vertices[1].x = 3.0f;
  region.vertices[1].y = -1.0f;

  region.vertices[2].x = -1.0f;
  region.vertices[2].y = 3.0f;
}

void 
OGLContext::getStreamReduceInterpolant( const TextureHandle inTexture,
                                        const unsigned int outputWidth,
                                        const unsigned int outputHeight, 
                                        const unsigned int minX,
                                        const unsigned int maxX, 
                                        const unsigned int minY,
                                        const unsigned int maxY,
                                        GPUInterpolant &interpolant) const
{
    float2 start(0.005f + minX, 0.005f + minY);
    float2 end(0.005f + maxX, 0.005f + maxY);

    get2DInterpolant( start, end, outputWidth, outputHeight, interpolant); 
}

void
OGLContext::getStreamReduceOutputRegion( const TextureHandle inTexture,
                                         const unsigned int minX,
                                         const unsigned int maxX, 
                                         const unsigned int minY,
                                         const unsigned int maxY,
                                         GPURegion &region) const
{
    OGLTexture* texture = (OGLTexture*) inTexture;
    unsigned int textureWidth = texture->width();
    unsigned int textureHeight = texture->height();

    float xmin = (float)minX / (float)textureWidth;
    float ymin = (float)minY / (float)textureHeight;
    float width = (float)(maxX - minX) / (float)textureWidth;
    float height = (float)(maxY - minY) / (float)textureHeight;
    
    float xmax = xmin + 2*width;
    float ymax = ymin + 2*height;

    // transform from texture space to surface space:
    xmin = 2*xmin - 1;
    xmax = 2*xmax - 1;
    ymin = 2*ymin - 1;
    ymax = 2*ymax - 1;

    region.vertices[0] = float4(xmin,ymin,0,1);
    region.vertices[1] = float4(xmax,ymin,0,1);
    region.vertices[2] = float4(xmin,ymax,0,1);
}

void 
OGLContext::drawRectangle( const GPURegion& outputRegion, 
                           const GPUInterpolant* interpolants, 
                           unsigned int numInterpolants ) {
  unsigned int w, h, i, v;

  w = _outputTexture->width();
  h = _outputTexture->height();

  wnd->bindPbuffer(_outputTexture->components());
  
  /*
   * We execute our kernel by using it to texture a triangle that
   * has vertices (-1, 3), (-1, -1), and (3, -1) which works out
   * nicely to contain the square (-1, -1), (-1, 1), (1, 1), (1, -1).
   */
  glViewport(0, 0, w, h);
  glBegin(GL_TRIANGLES);

  for (v=0; v<3; v++ )
  {
        GPULOG(1) << "vertex " << v;

        for (i=0; i<numInterpolants; i++) 
        {
            glMultiTexCoord4fvARB(GL_TEXTURE0_ARB+i,
                                (GLfloat *) &(interpolants[i].vertices[v]));

            GPULOG(1) << "tex" << i << " : " << interpolants[i].vertices[v].x
                << ", " << interpolants[i].vertices[v].y;
        }
        glVertex2fv((GLfloat *) &(outputRegion.vertices[v]));
        GPULOG(1) << "pos : " << outputRegion.vertices[v].x
            << ", " << outputRegion.vertices[v].y;
  }
  glEnd();
  CHECK_GL();
/*
  glViewport(0, 0, w, h);

  glBegin(GL_TRIANGLES);
  for (i=0; i<numInterpolants; i++) 
  {
    glMultiTexCoord4fvARB(GL_TEXTURE0_ARB+i,
                          (GLfloat *) &(interpolants[i].vertices[0]));
  }
  glVertex2f(-1.0f, -1.0f);
  for (i=0; i<numInterpolants; i++) 
    glMultiTexCoord4fvARB(GL_TEXTURE0_ARB+i,
                          (GLfloat *) &(interpolants[i].vertices[1]));
  glVertex2f(3.0f, -1.0f);
  for (i=0; i<numInterpolants; i++) 
    glMultiTexCoord4fvARB(GL_TEXTURE0_ARB+i,
                          (GLfloat *) &(interpolants[i].vertices[2]));
  glVertex2f(-1.0f, 3.0f);
  glEnd();
  CHECK_GL();
*/
  /* Copy the output to the texture */
  glActiveTextureARB(GL_TEXTURE0+_slopTextureUnit);
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, _outputTexture->id());
  glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, w, h);
  CHECK_GL();
}
