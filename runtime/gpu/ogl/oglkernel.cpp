
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#include "oglcontext.hpp"

using namespace brook;

static const char passthrough_vertex[] = 
"put code here";

static const char passthrough_pixel[] =
"!!ARBfp1.0\n"
"ATTRIB tex0 = fragment.texcoord[0];\n"
"OUTPUT oColor = result.color;\n"
"TEX oColor, tex0, texture[0], RECT;\n"
"END\n";

GPUContext::VertexShaderHandle 
OGLContext::getPassthroughVertexShader(void) {
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
  glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 
                   (unsigned int) inVertexShader);
  CHECK_GL();
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

  f1.x = x1 - shiftx;
  f1.y = y1 - shifty;
  f1.z = z1 - shiftz;
  f1.w = w1 - shiftw;

  f2.x = (2*x2-x1) + shiftx;
  f2.y = (2*y2-y1) + shifty;
  f2.z = (2*z2-z1) + shiftz;
  f2.w = (2*w2-w1) + shiftw;

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
  float ratioy = sy / w;
  float shiftx = ratiox * 0.5f;
  float shifty = ratioy * 0.5f;

  f1.x = x1 - shiftx;
  f1.y = y1 - shifty;

  f2.x = (2*x2-x1) + shiftx;
  f2.y = (2*y2-y1) + shifty;

  if (w==1) {
    interpolant.vertices[0] = float4(f1.x, f1.y, 0.0f, 1.0f);
    interpolant.vertices[1] = float4(f2.x, f1.y, 0.0f, 1.0f);
    interpolant.vertices[2] = interpolant.vertices[0];
    return;
  }

  if (h==1) {
    interpolant.vertices[0] = float4(f1.x, f1.y, 0.0f, 1.0f);
    interpolant.vertices[1] = interpolant.vertices[0];
    interpolant.vertices[2] = float4(f1.x, f2.y, 0.0f, 1.0f);
    return;
  }

  interpolant.vertices[0] = float4(f1.x, f1.y, 0.0f, 1.0f);
  interpolant.vertices[1] = float4(f2.x, f1.y, 0.0f, 1.0f);
  interpolant.vertices[2] = float4(f1.x, f2.y, 0.0f, 1.0f);
}


void
OGLContext::getStreamInterpolant( const TextureHandle texture,
                                  const unsigned int w,
                                  const unsigned int h,
                                  GPUInterpolant &interpolant) const {

  OGLTexture *oglTexture = (OGLTexture *) texture;

  float2 start(0.5f, 0.5f);
  float2 end(w-0.5f, h-0.5f);

  get2DInterpolant(  start, end, w, h, interpolant); 
}

void
OGLContext::getStreamOutputRegion( const TextureHandle texture,
                                   GPURegion &region) const {
  
  const OGLTexture *oglTexture = (OGLTexture *) texture;

  region.vertices[0].x = 0.0f;
  region.vertices[0].y = 0.0f;

  region.vertices[1].x = oglTexture->width()*2.0f;
  region.vertices[1].y = 0.0f;

  region.vertices[2].x = 0.0f;
  region.vertices[2].y = oglTexture->height()*2.0f;
}

void 
OGLContext::drawRectangle( const GPURegion& outputRegion, 
                           const GPUInterpolant* interpolants, 
                           unsigned int numInterpolants ) {
  unsigned int w, h, i;

  w = _outputTexture->width();
  h = _outputTexture->height();

  bindPbuffer(_outputTexture->components());
  
  /*
   * We execute our kernel by using it to texture a triangle that
   * has vertices (-1, 3), (-1, -1), and (3, -1) which works out
   * nicely to contain the square (-1, -1), (-1, 1), (1, 1), (1, -1).
   */

  glViewport(0, 0, w, h);

  glBegin(GL_TRIANGLES);
  for (i=0; i<numInterpolants; i++) 
    glMultiTexCoord4fvARB(GL_TEXTURE0_ARB+i,
                          (GLfloat *) &(interpolants[i].vertices[0]));
  glVertex2f(-1.0f, -1.0f);
  for (i=0; i<numInterpolants; i++) 
    glMultiTexCoord4fvARB(GL_TEXTURE0_ARB+i,
                          (GLfloat *) &(interpolants[i].vertices[1]));
  glVertex2f(3.0f, 1.0f);
  for (i=0; i<numInterpolants; i++) 
    glMultiTexCoord4fvARB(GL_TEXTURE0_ARB+i,
                          (GLfloat *) &(interpolants[i].vertices[2]));
  glVertex2f(-1.0f, 3.0f);
  glEnd();

  /* Copy the output to the texture */
  glActiveTextureARB(_slopTextureUnit);
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, _outputTexture->id());
  glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, w, h);
  CHECK_GL();
}
