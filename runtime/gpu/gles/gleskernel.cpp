#include <string.h>
#include "glesfunc.hpp"
#include "glescontext.hpp"
#include "glescheckgl.hpp"
#include "glestexture.hpp"
#include "gleswindow.hpp"
using namespace brook;

static const char passthrough_vertex[] = 
"not used";

static const char passthrough_pixel[] =
"!!ARBfp1.0\n"
"ATTRIB tex0 = fragment.texcoord[0];\n"
"OUTPUT oColor = result.color;\n"
"TEX oColor, tex0, texture[0], RECT;\n"
"END\n";

GLESPixelShader::GLESPixelShader(unsigned int _id, const char * _program_string):
  id(_id), program_string(_program_string), largest_constant(0) {
  unsigned int i;
  
  for (i=0; i<(unsigned int) MAXCONSTANTS; i++) {
    constants[i] = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
}

GLESSLPixelShader::GLESSLPixelShader(unsigned int _id, const char *program_string, unsigned int _vid):
  GLESPixelShader(_id, program_string), programid(0), vid(_vid) {
  GLint status = 0;
  programid = glCreateProgram();
  //attach the trivial vertex shader
  glAttachShader(programid, vid);
  glAttachShader(programid, id);
  glLinkProgram(programid);

  glGetProgramiv(programid, GL_LINK_STATUS, &status);
  if(GL_TRUE!=status) {
    char *errlog;
    glGetProgramiv(programid, GL_INFO_LOG_LENGTH, &status);
    errlog=(char *) brmalloc(status);
    glGetProgramInfoLog(programid, status, NULL, errlog);
    fprintf ( stderr, "GL: Program Error. Linker output:\n%s\n", errlog);
    fflush(stderr);
    brfree(errlog);
    assert(0);
    exit(1);
  }

  // Fetch the constant names
  unsigned int highest=0;
  while (*program_string&&(program_string=strstr(program_string,"//var "))!=NULL) {
    const char *name;
    unsigned int len=0;
    program_string += strlen("//var ");         
    /* Looks like this:
				"//var float n : C0 :  : 0 : 0\n"
				"//var float a : C1 : pend_s3_a : 1 : 1\n"
				"//var float4 _CGCdash_output_0 : $vout.COLOR0 : COLOR0 : 4 : 1\n"
       Format seems to be:
                "//var <type> <cgc name> : [C<constant index>|$vout.COLOR<x>] : <GLSL name> : ? : ?\n"
    */
    do program_string++; while (*program_string !=':');
    do program_string++; while (*program_string ==' ');
    if (*program_string!='C' && strncmp(program_string, "$vout.COLOR", 11)) continue;
    bool isOutput=*program_string!='C';
    program_string+=isOutput ? 11 : 1;
    char * ptr=NULL;
    unsigned int constreg = strtol (program_string,&ptr,10);
    do program_string++; while (*program_string !=':');
    do program_string++; while (*program_string ==' ');
	if (*program_string==':') continue;     // No GLSL name
    name = program_string;
    do{program_string++; len++;}while (*program_string!='\0'&&*program_string != ' ');
    std::string const_name(name,len);
    if(isOutput)
        constreg+=highest+1+constreg;
    else if(constreg>highest)
        highest=constreg;
    if (constreg > (unsigned int)MAXCONSTANTS) {
      fprintf (stderr, "GL: Too many constant registers\n");
      exit(1);
    }       
    program_string=ptr;
    constant_names[constreg] = const_name;
  }
  // Work out the types for the ones we actually use
  int samplerreg=0;
  program_string=this->program_string;
  while (*program_string&&(program_string=strstr(program_string, "uniform "))!=NULL) {
    const char *name;
    unsigned int len=0;
    program_string += strlen("uniform ");         
    name = program_string; len=0;
    do{program_string++; len++;}while (*program_string!='\0'&&*program_string != ' ');
    std::string const_type(name,len);

    do program_string++; while (*program_string ==' ');
    /* set name to the ident */
    name = program_string; len=0;
    do{program_string++; len++;}while (*program_string!='\0'&&*program_string != ';');
    std::string const_name(name,len);
    if(!strncmp(const_type.c_str(), "sampler", 7))
      sampler_names[samplerreg++] = const_name;
    else {
      for(unsigned int constreg=0; constreg<MAXCONSTANTS; constreg++) {
        if(!constant_names[constreg].compare(const_name)) {
          constant_types[constreg] = const_type;
          break;
        }
      }
    }
  }
  CHECK_GL();
}

GLESSLPixelShader::~GLESSLPixelShader() {
  glDeleteProgram(programid);
  glDeleteShader(id);
  glDeleteShader(vid);
}  

void 
GLESSLPixelShader::bindConstant( unsigned int inIndex, const float4& inValue ) {
  bindPixelShader();
  GLint cid = glGetUniformLocation(programid, constant_names[inIndex].c_str());
  if(-1!=cid) {
    if(!strcmp(constant_types[inIndex].c_str(), "float"))
      glUniform1fv(cid, 1, (const GLfloat *) &inValue);
    else if(!strcmp(constant_types[inIndex].c_str(), "vec2"))
      glUniform2fv(cid, 1, (const GLfloat *) &inValue);
    else if(!strcmp(constant_types[inIndex].c_str(), "vec3"))
      glUniform3fv(cid, 1, (const GLfloat *) &inValue);
    else if(!strcmp(constant_types[inIndex].c_str(), "vec4"))
      glUniform4fv(cid, 1, (const GLfloat *) &inValue);
	//else { assert(0); }
  }
  CHECK_GL();

  if (inIndex >= largest_constant)
    largest_constant = inIndex+1;

  constants[inIndex] = inValue;
}

void 
GLESSLPixelShader::bindPixelShader() {
  //These are no longer required in GLES
  //glBindProgram(GL_FRAGMENT_SHADER, 0);
  //glDisable(GL_FRAGMENT_SHADER);
  glUseProgram(programid);
  CHECK_GL();
  // For compatibility with ARB, we simply set these incrementally
  int v=0;
  for(int i=0; !sampler_names[i].empty(); ++i) {
    std::string samplername(sampler_names[i]);
    char *br=strchr((char *) samplername.c_str(), '[');
    int items=!br ? 1 : (*br=0, strtoul(br+1, NULL, 10));
    std::vector<int> values(items);
    for(std::vector<int>::iterator it=values.begin(); it!=values.end(); ++it)
      *it=v++;
    glUniform1iv(glGetUniformLocation(programid, samplername.c_str()), items, &values.front());
	CHECK_GL();
  }
}



GPUContext::VertexShaderHandle 
GLESContext::getPassthroughVertexShader( const char* inShaderFormat ) {
#if 0
  if (!_passthroughVertexShader) {
    GLuint id;
    glGenProgramsARB(1, &id);
    glBindProgramARB(GL_VERTEX_PROGRAM, id);
    glProgramStringARB(GL_VERTEX_PROGRAM, GL_PROGRAM_FORMAT_ASCII,
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


//Probably this needs to be removed, since it seems ARB related
GPUContext::PixelShaderHandle 
GLESContext::getPassthroughPixelShader( const char* inShaderFormat ) {
assert(0);
 /* 
  //fprintf (stderr, "getPassthroughPixelShader: this=0x%p\n", this);

  if (!_passthroughPixelShader) {
    GLuint id;
    //these are no longer required with GLES
    //fprintf (stderr, "Calling glGenProgramsARB...\n");
    //glGenProgramsARB(1, &id);
    //fprintf (stderr, "Calling glBindProgramARB...\n");
    //glBindProgramARB(GL_FRAGMENT_SHADER, id);
    //fprintf (stderr, "Loading String: \n");
    //fprintf (stderr, "%s\n", passthrough_pixel);
    //glProgramStringARB(GL_FRAGMENT_SHADER, GL_PROGRAM_FORMAT_ASCII,
    //                      strlen(passthrough_pixel), 
    //                      (GLubyte *) passthrough_pixel);
    //fprintf (stderr, "Mallocing PixelShader\n");
    //_passthroughPixelShader = new GLESARBPixelShader(id,passthrough_pixel);
    //fprintf (stderr, "Checking GL\n");
    CHECK_GL();
  }

  //fprintf (stderr, "  returning 0x%p\n ", _passthroughPixelShader);
  return (GPUContext::PixelShaderHandle) _passthroughPixelShader;
*/}

unsigned int 
GLESContext::createShader( const char* shader, GLenum shaderType ) 
{
  unsigned int id;
  if(strncmp(shader, "!!ARBfp", 7)) {
    // This is a GLSL shader
    GLint status = 0, shaderlen = strlen(shader);

    id = glCreateShader(shaderType);
	glShaderSource(id, 1, &shader, &shaderlen);
    glCompileShader(id);
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	printf("Compiling %s shader with source:\n%s\n", 
			(shaderType==/*GL_FRAGMENT_PROGRAM_ARB*/GL_FRAGMENT_SHADER)?"Fragment":"Vertex", shader);
    if(GL_TRUE!=status) {
      char *errlog;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &status);
      errlog=(char *) brmalloc(status);
      glGetShaderInfoLog(id, status, NULL, errlog);
      fprintf ( stderr, "GL: Program Error. Compiler output:\n%s\n", errlog);
      fflush(stderr);
      brfree(errlog);
      assert(0);
      exit(1);
    }
    return id;
  }
  else assert(0);
}

GPUContext::PixelShaderHandle
GLESContext::createPixelShader( const char* shader ) 
{
	unsigned int id, vid;
	vid = createShader(vShader, GL_VERTEX_SHADER );
	id = createShader(shader, GL_FRAGMENT_SHADER );
    return (GPUContext::PixelShaderHandle) new GLESSLPixelShader(id,shader,vid);
}

void 
GLESContext::bindConstant( PixelShaderHandle ps,
                          unsigned int inIndex, 
                          const float4& inValue ) {
  
  GLESPixelShader *glesps = (GLESPixelShader *) ps;

  GPUAssert(glesps, "Missing shader");
  glesps->bindConstant(inIndex, inValue);
}


void 
GLESContext::bindTexture( unsigned int inIndex, 
                         TextureHandle inTexture ) {
  GLESTexture *glesTexture = (GLESTexture *) inTexture;
  
  GPUAssert(glesTexture, "Null Texture");
  GPUAssert(inIndex < _slopTextureUnit, 
            "Too many bound textures");

  glActiveTexture(GL_TEXTURE0+inIndex);
  CHECK_GL();
  glBindTexture(GL_TEXTURE_2D, glesTexture->id());
  CHECK_GL();

  _boundTextures[inIndex] = glesTexture;
}


void GLESContext::bindOutput( unsigned int inIndex, 
                             TextureHandle inTexture ) {
  GLESTexture *glesTexture = (GLESTexture *) inTexture;
  
  GPUAssert(glesTexture, "Null Texture");

  GPUAssert(inIndex <= _maxOutputCount , 
            "Backend does not support so many shader outputs.");

  _outputTextures[inIndex] = glesTexture;
}

void 
GLESContext::bindPixelShader( GPUContext::PixelShaderHandle inPixelShader ) {
  
  GLESPixelShader *ps = (GLESPixelShader *) inPixelShader;
  GPUAssert(ps, "Null pixel shader");

  ps->bindPixelShader();
  _boundPixelShader = ps;
}

void 
GLESContext::bindVertexShader( GPUContext::VertexShaderHandle inVertexShader ) {
#if 0
  glBindProgramARB(GL_VERTEX_PROGRAM, 
                   (unsigned int) inVertexShader);
  CHECK_GL();
#endif
}

void GLESContext::disableOutput( unsigned int inIndex ) {
 _outputTextures[inIndex] = NULL;
}

void
GLESContext::get1DInterpolant( const float4 &start, 
                              const float4 &end,
                              const unsigned int w,
                              GPUInterpolant &interpolant) const {
assert(0);
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
GLESContext::get2DInterpolant( const float2 &start, 
                              const float2 &end,
                              const unsigned int w,
                              const unsigned int h,
                              GPUInterpolant &interpolant) const {
  unsigned int _w= w ? w:1;
  unsigned int _h= h ? h:1;
  //bottom-left triangle
  interpolant.vertices[0] = float4(start.x/_w, end.y/_h,   0.0f, 1.0f);
  interpolant.vertices[1] = float4(start.x/_w, start.y/_h, 0.0f, 1.0f);
  interpolant.vertices[2] = float4(end.x/_w,   start.y/_h, 0.0f, 1.0f);
   //upper-right triangle
  interpolant.vertices[3] = float4(start.x/_w, end.y/_h,   0.0f, 1.0f);
  interpolant.vertices[4] = float4(end.x/_w,   end.y/_h,   0.0f, 1.0f);
  interpolant.vertices[5] = float4(end.x/_w,   start.y/_h, 0.0f, 1.0f);
}



float4 
GLESContext::getStreamIndexofConstant( TextureHandle inTexture ) const {
  return float4(1.0f, 1.0f, 0.0f, 0.0f);
}


float4
GLESContext::getStreamGatherConstant(
                                    unsigned int inRank, const unsigned int* inDomainMin,
                                    const unsigned int* inDomainMax, const unsigned int* inExtents ) const {
  float scaleX = 1.0f;
  float scaleY = 1.0f;
  float offsetX = 0.0f;
  float offsetY = 0.0f;
  if( inRank == 1 )
  {
    unsigned int base = inDomainMin[0];

    offsetX = base + 0.5f;
  }
  else
  {
    unsigned int baseX = inDomainMin[1];
    unsigned int baseY = inDomainMin[0];

    offsetX = baseX + 0.5f;
    offsetY = baseY + 0.5f;
  }
  return float4( scaleX, scaleY, offsetX, offsetY );
//  return float4(1.0f, 1.0f, 0.5f, 0.5f);
}


void
GLESContext::getStreamInterpolant( const TextureHandle texture,
                                  unsigned int rank,
                                  const unsigned int* domainMin,
                                  const unsigned int* domainMax,
                                  unsigned int w,
                                  unsigned int h,
                                  GPUInterpolant &interpolant) const {

  unsigned int minX, minY, maxX, maxY;
  if( rank == 1 )
  {
      minX = domainMin[0];
      minY = 0;
      maxX = domainMax[0];
      maxY = 0;
  }
  else
  {
      minX = domainMin[1];
      minY = domainMin[0];
      maxX = domainMax[1];
      maxY = domainMax[0];
  }

  float2 start(minX + 0.005f, minY + 0.005f);
  float2 end(maxX + 0.005f, maxY + 0.005f);

  get2DInterpolant(  start, end, w, h, interpolant); 
}

void
GLESContext::getStreamOutputRegion( const TextureHandle texture,
                                   unsigned int rank,
                                   const unsigned int* domainMin,
                                   const unsigned int* domainMax,
                                   GPURegion &region) const
{
  unsigned int minX, minY, maxX, maxY;
  if( rank == 1 )
  {
      minX = domainMin[0];
      minY = 0;
      maxX = domainMax[0];
      maxY = 1;
  }
  else
  {
      minX = domainMin[1];
      minY = domainMin[0];
      maxX = domainMax[1];
      maxY = domainMax[0];
  }

  //bottom-left triangle
  region.vertices[0].x = -1;
  region.vertices[0].y =  1;

  region.vertices[1].x = -1;
  region.vertices[1].y = -1;

  region.vertices[2].x =  1;
  region.vertices[2].y = -1;

  //upper-right triangle
  region.vertices[3].x = -1;
  region.vertices[3].y =  1;

  region.vertices[4].x =  1;
  region.vertices[4].y =  1;

  region.vertices[5].x =  1;
  region.vertices[5].y = -1;

  region.viewport.minX = minX;
  region.viewport.minY = minY;
  region.viewport.maxX = maxX;
  region.viewport.maxY = maxY;
}

void 
GLESContext::getStreamReduceInterpolant( const TextureHandle inTexture,
                                        const unsigned int outputWidth,
                                        const unsigned int outputHeight, 
                                        const unsigned int minX,
                                        const unsigned int maxX, 
                                        const unsigned int minY,
                                        const unsigned int maxY,
                                        GPUInterpolant &interpolant) const
{
assert(0);
    float2 start(0.005f + minX, 0.005f + minY);
    float2 end(0.005f + maxX, 0.005f + maxY);

    get2DInterpolant( start, end, outputWidth, outputHeight, interpolant); 
}

void
GLESContext::getStreamReduceOutputRegion( const TextureHandle inTexture,
                                         const unsigned int minX,
                                         const unsigned int maxX, 
                                         const unsigned int minY,
                                         const unsigned int maxY,
                                         GPURegion &region) const
{
  region.vertices[0].x = -1;
  region.vertices[0].y = -1;

  region.vertices[1].x = 3;
  region.vertices[1].y = -1;

  region.vertices[2].x = -1;
  region.vertices[2].y = 3;

  region.viewport.minX = minX;
  region.viewport.minY = minY;
  region.viewport.maxX = maxX;
  region.viewport.maxY = maxY;
}

void 
GLESContext::drawRectangle( const GPURegion& outputRegion, 
                           const GPUInterpolant* interpolants, 
                           unsigned int numInterpolants ) {
  unsigned int w, h, i, v;
  unsigned int numOutputs, maxComponent;
  GLESTexture *outputTextures[32];
  static GLenum outputEnums[32]={0};
  if(outputEnums[0]!=GL_COLOR_ATTACHMENT0)
    for(i=0; i<32; i++)
      outputEnums[i]=GL_COLOR_ATTACHMENT0+i;

  /* Here we assume that all of the outputs are of the same size */
  w = _outputTextures[0]->width();
  h = _outputTextures[0]->height();

  numOutputs = 0;
  maxComponent = 0;
  for (i=0; i<_maxOutputCount; i++) {
    if (_outputTextures[i]) {
      outputTextures[i]=_outputTextures[i];
      numOutputs = i+1;
      if (outputTextures[i]->components() > maxComponent)
        maxComponent = outputTextures[i]->components();
    }
  }
  for (i=0; i<32; i++) 
    if (_boundTextures[i]) {
      for(v=0; v<numOutputs; v++) {
        if(_boundTextures[i]==outputTextures[v]) {
          // We need a temporary copy for the output
          if(_outputTexturesCache[0]) {
            if(_outputTexturesCache[0]->width()!=outputTextures[v]->width()
                || _outputTexturesCache[0]->height()!=outputTextures[v]->height()
                || _outputTexturesCache[0]->format()!=outputTextures[v]->format()) {
              for(unsigned int n=0; n<_maxOutputCount && _outputTexturesCache[n]; n++) {
                delete _outputTexturesCache[n];
                _outputTexturesCache[n] = NULL;
              }
            }
          }
          if(_outputTexturesCache[0]) {
            outputTextures[v]=_outputTexturesCache[0];
            memmove(_outputTexturesCache, _outputTexturesCache+1, (_maxOutputCount-1)*sizeof(GLESTexture *));
            _outputTexturesCache[_maxOutputCount-1]=0;
          }
          else
            outputTextures[v]=new GLESTexture(this, outputTextures[v]->width(), outputTextures[v]->height(), outputTextures[v]->format());
        }
      }
    }
  CHECK_GL();

  _wnd->bindFBO();
  {

    // Bind the shader
    GPUAssert(_boundPixelShader, "Missing pixel shader");
    bindPixelShader((PixelShaderHandle) _boundPixelShader);
    
    // Bind the textures
    for (i=0; i<32; i++) 
      if (_boundTextures[i]) {
#ifdef GLES_PRINTOPS
        printf("Setting texture %u as input %d\n", _boundTextures[i]->id(), i);
#endif
        glActiveTexture(GL_TEXTURE0+i);
        CHECK_GL();
        glBindTexture(GL_TEXTURE_2D, _boundTextures[i]->id());
        CHECK_GL();
    }
    CHECK_GL();

    // Bind the outputs
    for(i=0; i<numOutputs; i++) {
#ifdef GLES_PRINTOPS
        printf("Setting texture %u as output %d\n", outputTextures[i]->id(), i);
#ifndef GLES3
	//OpenGL ES 2.0 doesn't support multiple outputs
        assert(numOutputs == 1);
#endif
        glFramebufferTexture2D(GL_FRAMEBUFFER, 
                 outputEnums[i], 
                 GL_TEXTURE_2D, outputTextures[i]->id(), 0);
		CHECK_GL();

		GLenum iResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		CHECK_GL();
		if(iResult != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Framebuffer incomplete at %s:%i\n", __FILE__, __LINE__);
			abort();
		}

    }
    CHECK_GL();

    // Bind the constants
    for (i=0; i<_boundPixelShader->largest_constant; i++) {
      bindConstant((PixelShaderHandle) _boundPixelShader,
                   i, _boundPixelShader->constants[i]);
    }

  }
  
  CHECK_GL();
#if 0
  // TIM: hacky magic magic
  if( _isUsingAddressTranslation && _isUsingOutputDomain )
  {
	  GPUAssert(0, "Combination of address translation and using output domain not supported");
#if 0
    // if we are writing to a domain of an address-translated
    // stream, then copy the output textures to the FBO
    // so that we can overwrite the proper domain

    // NOTE: this will fail if we try to optimize domain
    // handling by only drawing to a subrectangle - for
    // now we render to the whole thing, so copying in
    // the whole texture is correct
    for( i = 0; i < numOutputs; i++ )
    {
      GLESTexture* output = outputTextures[i];
      glDrawBuffer(outputEnums[i]);
      //copy_to_FBO(output);
	  assert(0);
#if !defined(_DEBUG) && !defined(DEBUG)
#warning Fixme!
#endif
    }

    // We need to rebind stuff since we messed up the state
    // of things

    // Rebind the shader
    GPUAssert(_boundPixelShader, "Missing pixel shader");
    bindPixelShader((PixelShaderHandle) _boundPixelShader);

    // Rebind the textures
    for (i=0; i<32; i++) 
    if (_boundTextures[i]) {
    glActiveTexture(GL_TEXTURE0+i);
    glBindTexture(GL_TEXTURE_2D, _boundTextures[i]->id());
    }

    // Rebind the constants
    for (i=0; i<_boundPixelShader->largest_constant; i++) {
    bindConstant((PixelShaderHandle) _boundPixelShader,
    i, _boundPixelShader->constants[i]);
    }
#endif
  }
#endif

//  assert(0);
//  glDrawBuffers (numOutputs, outputEnums); 
  printf("numOutputs:%d\n", numOutputs);

  CHECK_GL();
  
  /*
   * We execute our kernel by using it to a rectangular texture with normalised coordinates 
   * (-1, -1), (-1, 1), (1, 1), (1, -1), composed by two triangles since GLES only supports triangle primitives
   */

  int minX = outputRegion.viewport.minX;
  int minY = outputRegion.viewport.minY;
  int maxX = outputRegion.viewport.maxX;
  int maxY = outputRegion.viewport.maxY;
  int width = maxX - minX;
  int height = maxY - minY;

  CHECK_GL();

  glViewport( minX, minY, width, height );
  CHECK_GL();
printf("minX=%d, minY=%d, width=%d, height=%d\n",  minX, minY, width, height );

  //Immediate mode is not supported in GLES. We need to port this...
  //assert(0);
printf("Program id=%d\n", ((GLESSLPixelShader*)_boundPixelShader)->programid);

  GLint vertex_index=glGetAttribLocation(((GLESSLPixelShader*)_boundPixelShader)->programid, "vPosition"); 
  CHECK_GL();
  if(vertex_index == -1)
  {
        printf("Attribute not found at %s:%i\n", __FILE__, __LINE__);
        assert(0);
  }

  GLint * texture_locations=(GLint*)malloc(sizeof(void*)*numInterpolants); 

  for (i=0; i<numInterpolants; i++)
  {
  	  CHECK_GL();
	  char name[20];
	  //sprintf(name, "gl_MultiTexCoord%d", numInterpolants);
	  sprintf(name, "aTEX%d", i);
		printf("Looking for Attribute %s at %s:%i\n", name, __FILE__, __LINE__);
		printf("program id:%d\n", ((GLESSLPixelShader*)_boundPixelShader)->programid);
  	  texture_locations[i]=glGetAttribLocation(((GLESSLPixelShader*)_boundPixelShader)->programid, name); 
  	  CHECK_GL();
  	  if(texture_locations[i] == -1)
  	  {
		printf("Attribute not found at %s:%i\n", __FILE__, __LINE__);
        	assert(0);
  	  }
	  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	  //glActiveTexture(GL_TEXTURE0+i);
          glEnableVertexAttribArray( texture_locations[i]);
  	  CHECK_GL();
	  //glTexCoordPointer(4, GL_FLOAT, 0, (GLfloat*) (interpolants[i].vertices));
  	  glVertexAttribPointer( texture_locations[i], 4, GL_FLOAT, 0, 0, (GLfloat*) (interpolants[i].vertices));
for(int cn=0; cn<6; cn++ )
{
	printf("Text Coord[%d].x=%f\n",cn, interpolants[i].vertices[cn].x);
	printf("Text Coord[%d].y=%f\n",cn, interpolants[i].vertices[cn].y);
	printf("Text Coord[%d].z=%f\n",cn, interpolants[i].vertices[cn].z);
	printf("Text Coord[%d].w=%f\n",cn, interpolants[i].vertices[cn].w);
}

  	  CHECK_GL();
  }
  _wnd->bindFBO();
 
  //glEnableClientState(GL_VERTEX_ARRAY);
  glEnableVertexAttribArray( vertex_index);
  CHECK_GL();
  //glVertexPointer(2, GL_FLOAT, sizeof(float4), outputRegion.vertices);
  glVertexAttribPointer( vertex_index, 2, GL_FLOAT, 0, sizeof(float4), outputRegion.vertices);
  CHECK_GL();
for(int cn=0; cn<6; cn++ )
{
	printf("Vert[%d].x=%f\n",cn,  outputRegion.vertices[cn].x);
	printf("Vert[%d].y=%f\n",cn,  outputRegion.vertices[cn].y);
}
int dim[4];
glGetIntegerv(GL_VIEWPORT, dim);
CHECK_GL();
printf("Viewport.x=%d\n", dim[0]);
printf("Viewport.y=%d\n", dim[1]);
printf("Viewport.width=%d\n", dim[2]);
printf("Viewport.height=%d\n", dim[3]);
glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dim);
CHECK_GL();
printf("MAX Viewport.width=%d\n", dim[0]);
printf("MAX Viewport.height=%d\n", dim[1]);

  glDrawArrays(GL_TRIANGLES,0,6);
  CHECK_GL();
  //glDisableClientState(GL_VERTEX_ARRAY);
  glDisableVertexAttribArray( vertex_index);
  CHECK_GL();

  for (i=0; i<numInterpolants; i++)
  {
	  glActiveTexture(GL_TEXTURE0+i);
	  CHECK_GL();
	  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	  glDisableVertexAttribArray( texture_locations[i]);
	  CHECK_GL();
  }

  free(texture_locations);


//assert(0);
glBindFramebuffer(GL_FRAMEBUFFER,0);
CHECK_GL();
printf("Unbind framebuffer to stop drawing\n");

  _wnd->swapBuffers();

  /*
  CHECK_GL();

  glBegin(GL_TRIANGLES);

  for (v=0; v<3; v++ )
  {
        GPULOG(1) << "vertex " << v;

        for (i=0; i<numInterpolants; i++) 
        {
            glMultiTexCoord4fv(GL_TEXTURE0+i,
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

  */

#if defined(_DEBUG) && 0
  for (i=0; i<32; i++) 
    if (_boundTextures[i]) {
      printf("Unsetting texture %u from input %d\n", _boundTextures[i]->id(), i);
      glActiveTexture(GL_TEXTURE0+i);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
#endif
  for (i=0; i<numOutputs; i++) {
#if defined(_DEBUG) && 0
    if(outputTextures[i]) {
      printf("Unsetting texture %u from output %d\n", outputTextures[i]->id(), i);
      glFramebufferTexture2D(GL_FRAMEBUFFER, outputEnums[i], 
                 GL_TEXTURE_2D, 0, 0);
    }
#endif
    if(_outputTextures[i] && outputTextures[i]!=_outputTextures[i]) {
#ifdef GLES_PRINTOPS
      printf("Swapping textures %u and %u\n", _outputTextures[i]->id(), outputTextures[i]->id());
#endif
      _outputTextures[i]->swap(*outputTextures[i]);
      for(unsigned int n=0; n<_maxOutputCount; n++)
          if(!_outputTexturesCache[n]) { _outputTexturesCache[n]=outputTextures[i]; outputTextures[i]=NULL; break; }
      delete outputTextures[i];
    }
    _outputTextures[i] = NULL;
  }
}
