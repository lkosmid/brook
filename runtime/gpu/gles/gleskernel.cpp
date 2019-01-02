#include <string.h>
#include "glesfunc.hpp"
#include "glescontext.hpp"
#include "glescheckgl.hpp"
#include "glestexture.hpp"
#include "gleswindow.hpp"
using namespace brook;
#define MAX(x,y) (((x)>=(y))?(x):(y))

static const char passthrough_vertex[] = 
"not used";

static const char passthrough_pixel[] =
"precision highp float;\n"
"uniform highp sampler2D textureUnit0;\n"
"varying vec4 TEX0;\n"
"void main()\n"
"{\n"
"  gl_FragColor = texture2D(textureUnit0, TEX0.xy);\n"
"}\n";

#define reconstruct_unsigned_int \
      "#define reconstruct_unsigned_int(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp float tmp;"\
      "  tmp = floor(256.0*u_split.x - (u_split.x/255.0));"\
      "  reconstructed = tmp;"\
      "  tmp = floor(256.0*u_split.y - (u_split.y/255.0))*256.0;"\
      "  reconstructed += tmp;"\
      "  tmp = floor(256.0*u_split.z - (u_split.z/255.0))*256.0*256.0;"\
      "  reconstructed += tmp;"\
      "  tmp = floor(256.0*u_split.w - (u_split.w/255.0))*256.0*256.0*256.0;"\
      "  reconstructed += tmp;"\
      "}\n" 

#define reconstruct_unsigned_char \
      "#define reconstruct_unsigned_char(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp float tmp;"\
      "  tmp = floor(u_split.x*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_char \
      "#define reconstruct_char(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp float tmp;"\
      "  if(u_split.x > 0.5) u_split.x -= 1.00392156862745098;"\
      "  tmp = floor(u_split.x*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_char2 \
      "#define reconstruct_char2(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp vec2 tmp;"\
      "  bvec2 IsNegative = greaterThan(u_split.xy, vec2(0.5));"\
      "  u_split.xy -= 1.00392156862745098*step( 1.0, vec2(IsNegative));"\
      "  tmp = floor(u_split.xy*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_char3 \
      "#define reconstruct_char3(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp vec3 tmp;"\
      "  bvec3 IsNegative = greaterThan(u_split.xyz, vec3(0.5));"\
      "  u_split.xyz -= 1.00392156862745098*step( 1.0, vec3(IsNegative));"\
      "  tmp = floor(u_split.xyz*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_char4 \
      "#define reconstruct_char4(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp vec4 tmp;"\
      "  bvec4 IsNegative = greaterThan(u_split.xyzw, vec4(0.5));"\
      "  u_split.xyzw -= 1.00392156862745098*step( 1.0, vec4(IsNegative));"\
      "  tmp = floor(u_split.xyzw*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_uchar \
      "#define reconstruct_uchar(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp float tmp;"\
      "  tmp = floor(u_split.x*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_uchar2 \
      "#define reconstruct_uchar2(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp vec2 tmp;"\
      "  tmp = floor(u_split.xy*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_uchar3 \
      "#define reconstruct_uchar3(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp vec3 tmp;"\
      "  tmp = floor(u_split.xyz*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_uchar4 \
      "#define reconstruct_uchar4(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp vec4 tmp;"\
      "  tmp = floor(u_split.xyzw*255.996078431372549);"\
      "  reconstructed = tmp;"\
      "}\n" 

#define reconstruct_int\
      "#define reconstruct_int(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp float tmp;"\
      "  tmp = floor(256.0*u_split.x - (u_split.x/255.0));"\
      "  reconstructed = tmp;"\
      "  tmp = floor(256.0*u_split.y - (u_split.y/255.0))*256.0;"\
      "  reconstructed += tmp;"\
      "  tmp = floor(256.0*u_split.z - (u_split.z/255.0))*256.0*256.0;"\
      "  reconstructed += tmp;"\
      "  tmp = floor(256.0*u_split.w - (u_split.w/255.0))*256.0*256.0*256.0;"\
      "  reconstructed += tmp;"\
      "  if(u_split.w > 0.5) reconstructed -= 4294967296.0;"\
      "}\n" 

#define reconstruct_float_header\
      "#define reconstruct_float(reconstructed, textureUnit0, vTexCoord0)"\
      "{"\
      "  highp vec4 u_split= texture2D(textureUnit0, vTexCoord0);"\
      "  highp vec4 tmp;"\
      "  tmp.xyzw = floor(u_split.xyzw*255.996078431372549 );"\
      "  tmp.zyx = tmp.zyx*vec3(0.0078125, 0.000030517578125, 0.00000011920928955078);"\
      "  reconstructed = tmp.z ;"\
      "  if(tmp.w != 0.0 ) reconstructed += step(reconstructed, 0.9921875) ;"\
      "  tmp.wy  = tmp.wy + vec2(- 127.0, tmp.x);"\
      "  highp float exponent  = exp2(tmp.w);"\
      "  reconstructed += tmp.y;"\
      "  reconstructed = exponent*reconstructed;"\
      "  if(u_split.z > 0.5) reconstructed = -reconstructed;"\
      "}\n" 

#define reconstruct_float_highp\
      ""

#define reconstruct_float_epilogue\
      "" 

#define encode_output_unsigned_int\
      "#define encode_output_unsigned_int(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  u_split.x = ((reconstructed - 256.0*floor(reconstructed*0.00390625))*0.00392156862745098) ;"\
      "  u_split.y = (floor((reconstructed - 256.0*256.0*floor(reconstructed*1.52587890625e-05))*0.00390625)*0.00392156862745098) ;"\
      "  u_split.z = (floor((reconstructed - 256.0*256.0*256.0*floor(reconstructed*0.00000005960464477539))*1.52587890625e-05)*0.00392156862745098) ;"\
      "  u_split.w = (floor((reconstructed - 256.0*256.0*256.0*256.0*floor(reconstructed*2.328306436538696289e-10))*0.00000005960464477539)*0.00392156862745098) ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_unsigned_char\
      "#define encode_output_unsigned_char(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  u_split.x = (reconstructed - 256.0*floor(reconstructed*0.00390625))*0.00392156862745098 ;"\
      "  u_split.yzw = vec3(0.0) ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_char\
      "#define encode_output_char(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  if(reconstructed < 0.0) reconstructed += 256.0;"\
      "  u_split.x = (reconstructed - 256.0*floor(reconstructed*0.00390625))*0.00392156862745098 ;"\
      "  u_split.yzw = vec3(0.0) ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_char2\
      "#define encode_output_char2(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  bvec2 IsNegative = lessThan(reconstructed.xy, vec2(0.0));"\
      "  reconstructed.xy += 256.0*step( 1.0, vec2(IsNegative));"\
      "  u_split.xy = (reconstructed.xy - 256.0*floor(reconstructed.xy*0.00390625))*0.00392156862745098 ;"\
      "  u_split.zw = vec2(0.0) ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_char3\
      "#define encode_output_char3(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  bvec3 IsNegative = lessThan(reconstructed.xyz, vec3(0.0));"\
      "  reconstructed.xyz += 256.0*step( 1.0, vec3(IsNegative));"\
      "  u_split.xyz = (reconstructed.xyz - 256.0*floor(reconstructed.xyz*0.00390625))*0.00392156862745098 ;"\
      "  u_split.w = 0.0 ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_char4\
      "#define encode_output_char4(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  bvec4 IsNegative = lessThan(reconstructed.xyzw, vec4(0.0));"\
      "  reconstructed.xyzw += 256.0*step( 1.0, vec4(IsNegative));"\
      "  u_split.xyzw = (reconstructed.xyzw - 256.0*floor(reconstructed.xyzw*0.00390625))*0.00392156862745098 ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_uchar\
      "#define encode_output_uchar(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  u_split.x = (reconstructed - 256.0*floor(reconstructed*0.00390625))*0.00392156862745098 ;"\
      "  u_split.yzw = vec3(0.0) ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_uchar2\
      "#define encode_output_uchar2(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  u_split.xy = (reconstructed.xy - 256.0*floor(reconstructed.xy*0.00390625))*0.00392156862745098 ;"\
      "  u_split.zw = vec2(0.0) ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_uchar3\
      "#define encode_output_uchar3(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  u_split.xyz = (reconstructed.xyz - 256.0*floor(reconstructed.xyz*0.00390625))*0.00392156862745098 ;"\
      "  u_split.w = 0.0 ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_uchar4\
      "#define encode_output_uchar4(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  u_split.xyzw = (reconstructed.xyzw - 256.0*floor(reconstructed.xyzw*0.00390625))*0.00392156862745098 ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_int\
      "#define encode_output_int(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  highp float _reconstructed=reconstructed;"\
      "  if(_reconstructed < 0.0) _reconstructed += 4294967296.0;"\
      "  u_split.x = ((_reconstructed - 256.0*floor(_reconstructed*0.00390625))*0.00392156862745098) ;"\
      "  u_split.y = ( floor( ( _reconstructed - 256.0*256.0*floor(_reconstructed*1.52587890625e-05)) *0.00390625)*0.00392156862745098) ;"\
      "  u_split.z = ( floor( ( _reconstructed - 256.0*256.0*256.0*floor(_reconstructed*0.00000005960464477539)) *1.52587890625e-05)*0.00392156862745098) ;"\
      "  u_split.w = ((reconstructed/exp2(24.0))*0.00392156862745098) ;"\
      "  gl_FragColor = u_split;"\
      "}\n" 

#define encode_output_float_header\
      "#define encode_output_float(reconstructed)"\
      "{" \
      "  highp vec4 u_split;"\
      "  highp float sign_value=1.0;"\
      "  float exponent;"\
      "  highp float tmp;"\
      "  highp vec4 tmp2;"\
      "  highp vec4 tmp3;"\
      "  tmp3.w = abs(reconstructed);"\
      "  tmp = floor(log2(tmp3.w));"\
      "  tmp2.wzy = exp2( vec3(-125.0, -tmp, 23.0) ) ;"\
      "  tmp3.z = (tmp + 127.0) ;"\
      "  tmp2.w = step(tmp2.w,tmp3.w) ;"\
      "  tmp2.wz = tmp2.wz*tmp3.zw ;"\
      "  tmp = clamp(tmp2.z -1.0, 0.0, 1.0);"\
      "  if(reconstructed < 0.0 ) sign_value = tmp2.y;"\
      "  tmp2.zy = tmp*tmp2.y + vec2(sign_value,0.0) ;"\
      "  tmp3.wzyx = floor(tmp2.wzyy*vec4(0.00390625, 0.00000005960464477539, 1.52587890625e-05, 0.00390625)) ;"\
      "  tmp3.wzyx = tmp3.wzyx*256.0 ;"\
      "  tmp3.wzyx = tmp2.wzyy*vec4(1.0, 1.52587890625e-05, 0.00390625, 1.0)  - tmp3.wzyx ;"\
      "  tmp3.zy = floor(tmp3.zy) ;"\
      "  tmp3.wzyx =  tmp3.wzyx * 0.00392156862745098 ;"\
      "  u_split = tmp3 ;"

#define encode_output_float_highp\
      ""

#define encode_output_float_lowp\
      ""

#define encode_output_float_epilogue\
      "  gl_FragColor = u_split;"\
      "}\n" 

static const std::string reconstruct_char_str(reconstruct_char);
static const std::string reconstruct_char2_str(reconstruct_char2);
static const std::string reconstruct_char3_str(reconstruct_char3);
static const std::string reconstruct_char4_str(reconstruct_char4);
static const std::string reconstruct_unsigned_char_str(reconstruct_unsigned_char);
static const std::string reconstruct_uchar_str(reconstruct_uchar);
static const std::string reconstruct_uchar2_str(reconstruct_uchar2);
static const std::string reconstruct_uchar3_str(reconstruct_uchar3);
static const std::string reconstruct_uchar4_str(reconstruct_uchar4);
static const std::string reconstruct_int_str(reconstruct_int);
static const std::string reconstruct_unsigned_int_str(reconstruct_unsigned_int);
static const std::string reconstruct_float_header_str(reconstruct_float_header);
static const std::string reconstruct_float_highp_str(reconstruct_float_highp);
static const std::string reconstruct_float_epilogue_str(reconstruct_float_epilogue);

static const std::string encode_output_char_str(encode_output_char);
static const std::string encode_output_char2_str(encode_output_char2);
static const std::string encode_output_char3_str(encode_output_char3);
static const std::string encode_output_char4_str(encode_output_char4);
static const std::string encode_output_unsigned_char_str(encode_output_unsigned_char);
static const std::string encode_output_uchar_str(encode_output_uchar);
static const std::string encode_output_uchar2_str(encode_output_uchar2);
static const std::string encode_output_uchar3_str(encode_output_uchar3);
static const std::string encode_output_uchar4_str(encode_output_uchar4);
static const std::string encode_output_int_str(encode_output_int);
static const std::string encode_output_unsigned_int_str(encode_output_unsigned_int);
static const std::string encode_output_float_header_str(encode_output_float_header);
static const std::string encode_output_float_highp_str(encode_output_float_highp);
static const std::string encode_output_float_lowp_str(encode_output_float_lowp);
static const std::string encode_output_float_epilogue_str(encode_output_float_epilogue);

GLESPixelShader::GLESPixelShader(unsigned int _id, const char * _program_string):
  id(_id), program_string(_program_string), largest_constant(0) {
  unsigned int i;
  
  for (i=0; i<(unsigned int) MAXCONSTANTS; i++) {
    constants[i] = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
}

GLESSLPixelShader::GLESSLPixelShader(unsigned int _id, const char *program_string, unsigned int _vid):
  GLESPixelShader(_id, program_string), programid(0), vid(_vid), vShader(trivial_GLSLES_vshader) {

  std::string unmodified_program(program_string);
  std::string custom_program;

  program_string=this->program_string;
  bool float_input=false;
  bool char_input=false;
  bool char2_input=false;
  bool char3_input=false;
  bool char4_input=false;
  bool uchar_input=false;
  bool uchar2_input=false;
  bool uchar3_input=false;
  bool uchar4_input=false;
  bool uint_input=false;
  bool int_input=false;
  //Check the input stream types and add their helper functions in the shader source
  while (*program_string&&(program_string=strstr(program_string,"reconstruct_"))!=NULL) {
    program_string+=12;
    if(!float_input && (strncmp(program_string, "float", 5)==0))
    {
       custom_program+=reconstruct_float_header_str;
#ifdef GLES_HIGH_FP
       custom_program+=reconstruct_float_highp_str;
#endif
       custom_program+=reconstruct_float_epilogue_str;
       float_input=true;
    }
    else if(!char_input && (strncmp(program_string, "char", 4)==0))
    {
       custom_program+=reconstruct_char_str;
       char_input=true;
    }
    else if(!char2_input && (strncmp(program_string, "char2", 5)==0))
    {
       custom_program+=reconstruct_char2_str;
       char2_input=true;
    }
    else if(!char3_input && (strncmp(program_string, "char3", 5)==0))
    {
       custom_program+=reconstruct_char3_str;
       char3_input=true;
    }
    else if(!char4_input && (strncmp(program_string, "char4", 5)==0))
    {
       custom_program+=reconstruct_char4_str;
       char4_input=true;
    }
    else if(!uchar_input && (strncmp(program_string, "unsigned_char", 13)==0))
    {
       custom_program+=reconstruct_unsigned_char_str;
       uchar_input=true;
    }
    else if(!uchar2_input && (strncmp(program_string, "uchar2", 6)==0))
    {
       custom_program+=reconstruct_uchar2_str;
       uchar2_input=true;
    }
    else if(!uchar3_input && (strncmp(program_string, "uchar3", 6)==0))
    {
       custom_program+=reconstruct_uchar3_str;
       uchar3_input=true;
    }
    else if(!uchar4_input && (strncmp(program_string, "uchar4", 6)==0))
    {
       custom_program+=reconstruct_uchar4_str;
       uchar4_input=true;
    }
    else if(!int_input && (strncmp(program_string, "int", 3)==0))
    {
       custom_program+=reconstruct_int_str;
       int_input=true;
    }
    else if(!uint_input && (strncmp(program_string, "unsigned_int", 12)==0))
    {
       custom_program+=reconstruct_unsigned_int_str;
       uint_input=true;
    }
  }

  program_string=this->program_string;
  //Check the input stream types and add their helper functions in the shader source
  while (*program_string&&(program_string=strstr(program_string,"encode_output_"))!=NULL) {
    program_string+=14;
    if(strncmp(program_string, "float", 5)==0)
    {
       custom_program+=encode_output_float_header_str;
#ifdef GLES_HIGH_FP
       custom_program+=encode_output_float_highp_str;
#else
       custom_program+=encode_output_float_lowp_str;
#endif
       custom_program+=encode_output_float_epilogue_str;
       break; //only one input supported in GLES and no need to check since we did it during compilation
    }
    else if(strncmp(program_string, "char", 4)==0)
    {
       switch(program_string[4])
       {
        case '2':
          custom_program+=encode_output_char2_str;
          break;
        case '3':
          custom_program+=encode_output_char3_str;
          break;
        case '4':
          custom_program+=encode_output_char4_str;
          break;
        default:
          custom_program+=encode_output_char_str;
       }
       break; 
    }
    else if(strncmp(program_string, "unsigned_char", 13)==0)
    {
          custom_program+=encode_output_unsigned_char_str;
    }
    else if(strncmp(program_string, "uchar", 5)==0)
    {
       switch(program_string[5])
       {
        case '2':
          custom_program+=encode_output_uchar2_str;
          break;
        case '3':
          custom_program+=encode_output_uchar3_str;
          break;
        case '4':
          custom_program+=encode_output_uchar4_str;
          break;
        default:
          custom_program+=encode_output_uchar_str;
       }
       break; 
    }
    else if(strncmp(program_string, "int", 3)==0)
    {
       custom_program+=encode_output_int_str;
       break; 
    }
    else if(strncmp(program_string, "unsigned_int", 12)==0)
    {
       custom_program+=encode_output_unsigned_int_str;
       break; 
    }
  }

  custom_program+=unmodified_program;

  // Fetch the constant names
  unsigned int highest=0;
  program_string=this->program_string;
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

  this->program_string=custom_program.c_str();
  this->vid = createShader(vShader, GL_VERTEX_SHADER );
  this->id = createShader(custom_program.c_str(), GL_FRAGMENT_SHADER );
  GLint status = 0;
  programid = glCreateProgram();
  CHECK_GL();
  //attach the trivial vertex shader
  glAttachShader(programid, vid);
  CHECK_GL();
  glAttachShader(programid, id);
  CHECK_GL();
  //Make sure that the vertices are in attribute 0
  glBindAttribLocation ( programid, 0, "vPosition" );
  CHECK_GL();
  glLinkProgram(programid);
  CHECK_GL();

  glGetProgramiv(programid, GL_LINK_STATUS, &status);
  CHECK_GL();
  if(GL_TRUE!=status) {
    char *errlog;
    glGetProgramiv(programid, GL_INFO_LOG_LENGTH, &status);
    CHECK_GL();
    errlog=(char *) brmalloc(status);
    glGetProgramInfoLog(programid, status, NULL, errlog);
    CHECK_GL();
    fprintf ( stderr, "GL: Program Error. Linker output:\n%s\n", errlog);
    fflush(stderr);
    brfree(errlog);
    fprintf( stderr, "Shader source:\n");
    //print the source with line numbers to help debugging
    const char * line_end;
    char * line=(char *) brmalloc(custom_program.size());
    unsigned int line_counter=0;
    const char * source=custom_program.c_str();
    while(line_end=strstr(source, "\n"))
    {
        line_counter++;
//TODO there is a warning here, it needs to be fixed
        snprintf(line, line_end - source+1, source);
        fprintf ( stderr, "%d:%s\n", line_counter, line);
        source=line_end+1;
    }
    assert(0);
    exit(1);
  }
}

GLESSLPixelShader::~GLESSLPixelShader() {
  glDeleteProgram(programid);
  CHECK_GL();
  glDeleteShader(id);
  CHECK_GL();
  glDeleteShader(vid);
  CHECK_GL();
}  

void 
GLESSLPixelShader::bindConstant( unsigned int inIndex, const float4& inValue ) {
  bindPixelShader();
  GLint cid = glGetUniformLocation(programid, constant_names[inIndex].c_str());
  CHECK_GL();
  if(-1!=cid) {
    if(!strcmp(constant_types[inIndex].c_str(), "float"))
      glUniform1fv(cid, 1, (const GLfloat *) &inValue);
    else if(!strcmp(constant_types[inIndex].c_str(), "vec2"))
      glUniform2fv(cid, 1, (const GLfloat *) &inValue);
    else if(!strcmp(constant_types[inIndex].c_str(), "vec3"))
      glUniform3fv(cid, 1, (const GLfloat *) &inValue);
    else if(!strcmp(constant_types[inIndex].c_str(), "vec4"))
      glUniform4fv(cid, 1, (const GLfloat *) &inValue);
    else { assert(0); }
    CHECK_GL();
  }

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
#ifdef GLES_DEBUG
printf("I have to check for uniform types\n");
#endif
    glUniform1iv(glGetUniformLocation(programid, samplername.c_str()), items, &values.front());
	CHECK_GL();
  }
}



GPUContext::VertexShaderHandle 
GLESContext::getPassthroughVertexShader( const char* inShaderFormat ) {
  return (GPUContext::VertexShaderHandle) 1;
}


GPUContext::PixelShaderHandle 
GLESContext::getPassthroughPixelShader( const char* inShaderFormat ) {
  if (!_passthroughPixelShader) {
    _passthroughPixelShader = new GLESSLPixelShader(0,passthrough_pixel,0);
  }

  return (GPUContext::PixelShaderHandle) _passthroughPixelShader;
}

unsigned int 
GLESContext::createShader( const char* shader, GLenum shaderType ) 
{
return 0;
}

unsigned int 
GLESSLPixelShader::createShader( const char* shader, GLenum shaderType ) 
{
  unsigned int id;
  if(strncmp(shader, "!!ARBfp", 7)) {
    // This is a GLSL shader
    GLint status = 0, shaderlen = strlen(shader);

    id = glCreateShader(shaderType);
    CHECK_GL();
    glShaderSource(id, 1, &shader, &shaderlen);
    CHECK_GL();
    glCompileShader(id);
    CHECK_GL();
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    CHECK_GL();
#ifdef GLES_DEBUG
	printf("Compiling %s shader with source:\n%s\n", 
			(shaderType==/*GL_FRAGMENT_PROGRAM_ARB*/GL_FRAGMENT_SHADER)?"Fragment":"Vertex", shader);
#endif
    if(GL_TRUE!=status) {
      char *errlog;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &status);
      CHECK_GL();
      errlog=(char *) brmalloc(status);
      glGetShaderInfoLog(id, status, NULL, errlog);
      CHECK_GL();
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
    return (GPUContext::PixelShaderHandle) new GLESSLPixelShader(0,shader,0);
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
  assert(w);
  //OpenGL ES works with normalised coordinates, so we don't care about the output size, but we need the actual texture size
  //Assume all inputs have the same size //TODO make the common gpu runtime aware of this, so that it passes the correct values
  //In the case of an iterator, there is no bound texture, so use the passed values
  //In the case of reductions the size is always smaller than the input sizes
  unsigned int _w = _boundTextures[0]? _boundTextures[0]->width() : MAX(w,end.x);
  unsigned int _h= 1;
  assert(_w);
  //bottom-left triangle
  interpolant.vertices[0] = float4(start.x/_w, end.y/_h,   0.0f, 1.0f);
  interpolant.vertices[1] = float4(start.x/_w, start.y/_h, 0.0f, 1.0f);
  interpolant.vertices[2] = float4(end.x/_w,   start.y/_h, 0.0f, 1.0f);
   //upper-right triangle
  interpolant.vertices[3] = float4(start.x/_w, end.y/_h,   0.0f, 1.0f);
  interpolant.vertices[4] = float4(end.x/_w,   end.y/_h,   0.0f, 1.0f);
  interpolant.vertices[5] = float4(end.x/_w,   start.y/_h, 0.0f, 1.0f);
}


void
GLESContext::get2DInterpolant( const float2 &start, 
                              const float2 &end,
                              const unsigned int w,
                              const unsigned int h,
                              GPUInterpolant &interpolant) const {
  assert(w);
  assert(h);
  //OpenGL ES works with normalised coordinates, so we don't care about the output size, but we need the actual texture size
  //Assume all inputs have the same size //TODO make the common gpu runtime aware of this, so that it passes the correct values
  //In the case of an iterator, there is no bound texture, so use the passed values
  //In the case of reductions the size is always smaller than the input sizes
  unsigned int _w = _boundTextures[0]? _boundTextures[0]->width() : MAX(w,end.x);
  unsigned int _h = _boundTextures[0]? _boundTextures[0]->height() : MAX(h,end.y);
  assert(_w);
  assert(_h);

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

  float2 start(minX /*+ 0.005f*/, minY /*+ 0.005f*/);
  float2 end(maxX /*+ 0.005f*/, maxY /*+ 0.005f*/);

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
//assert(0);
    //GLES uses normalised texture coordinates
    //In order get2DInterpolant to be able to compute them properly, it needs
    //the input texture size, since the output is always smaller
    GLESTexture *glesTexture = (GLESTexture *) inTexture;
    assert(glesTexture);
    float2 start(minX, minY);
    float2 end(maxX, maxY);
/*
  unsigned int _w ;
  unsigned int _h ;
if(_boundTextures[0])
{
  _w = glesTexture->width();
  _h = glesTexture->height();
}
else
{
  _w = glesTexture->width();
  _h = glesTexture->height();
}*/
  unsigned int _w = glesTexture->width();
  unsigned int _h = glesTexture->height();
  assert(_w);
  assert(_h);

  //bottom-left triangle
  interpolant.vertices[0] = float4(start.x, end.y,   0.0f, 1.0f);
  interpolant.vertices[1] = float4(start.x, start.y, 0.0f, 1.0f);
  interpolant.vertices[2] = float4(end.x,   start.y, 0.0f, 1.0f);
   //upper-right triangle
  interpolant.vertices[3] = float4(start.x, end.y,   0.0f, 1.0f);
  interpolant.vertices[4] = float4(end.x,   end.y,   0.0f, 1.0f);
  interpolant.vertices[5] = float4(end.x,   start.y, 0.0f, 1.0f);
}

void 
GLESContext::getStreamPassThroughInterpolant( const TextureHandle texture,
                                         const unsigned int Width,
                                         const unsigned int Height, 
                                         GPUInterpolant &interpolant) const 
{
  assert(Width);
  assert(Height);
  GLESTexture *glesTexture = (GLESTexture *) texture;
  unsigned int _w = glesTexture->width();
  unsigned int _h = glesTexture->height(); 
  assert(_w);
  assert(_h);

  //bottom-left triangle
  interpolant.vertices[0] = float4(0.0f, Height/_h,   0.0f, 1.0f);
  interpolant.vertices[1] = float4(0.0f, 0.0f, 0.0f, 1.0f);
  interpolant.vertices[2] = float4(Width/_w,   0.0f, 0.0f, 1.0f);
   //upper-right triangle
  interpolant.vertices[3] = float4(0.0f, Height/_h,   0.0f, 1.0f);
  interpolant.vertices[4] = float4(Width/_w,   Height/_h,   0.0f, 1.0f);
  interpolant.vertices[5] = float4(Width/_w,   0.0f, 0.0f, 1.0f);
}

void
GLESContext::getStreamReduceOutputRegion( const TextureHandle inTexture,
                                         const unsigned int minX,
                                         const unsigned int maxX, 
                                         const unsigned int minY,
                                         const unsigned int maxY,
                                         GPURegion &region) const
{
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
GLESContext::drawRectangle( const GPURegion& outputRegion, 
                           const GPUInterpolant* interpolants, 
                           unsigned int numInterpolants ) {
  unsigned int w, h, i, v, output_id;
  unsigned int numOutputs, maxComponent;
#ifndef GLES3
#define NOUTPUTS 1
#else
#define NOUTPUTS 32 //TODO: to be checked with GLES3 limits
#endif
  GLint vertex_index=0;
  GLESTexture *outputTextures[NOUTPUTS];
  static GLenum outputEnums[NOUTPUTS]={0};
  if(outputEnums[0]!=GL_COLOR_ATTACHMENT0)
    for(i=0; i<NOUTPUTS; i++)
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
  for (i=0; i<NOUTPUTS; i++) 
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

  _wnd->bindFBO();
  {

    // Bind the shader
    GPUAssert(_boundPixelShader, "Missing pixel shader");
    bindPixelShader((PixelShaderHandle) _boundPixelShader);
    
    // Bind the textures
    for (i=0; i</*32*/7 /*TODO we should make it MAXTEXTURE UNITS-1, so that the last one is assigned to output*/; i++) 
      if (_boundTextures[i]) {
#ifdef GLES_PRINTOPS
        printf("Setting texture %u as input %d with size:%d,%d\n", _boundTextures[i]->id(), i, _boundTextures[i]->width(), _boundTextures[i]->height());
#endif
        glActiveTexture(GL_TEXTURE0+i);
        CHECK_GL();
        glBindTexture(GL_TEXTURE_2D, _boundTextures[i]->id());
        CHECK_GL();
    }

    // Bind the outputs
    for(i=0; i<numOutputs; i++) {
#ifdef GLES_PRINTOPS
        printf("Setting texture %u as output %d with size:%d,%d\n", outputTextures[i]->id(), i, _outputTextures[i]->width(), _outputTextures[i]->height());
#endif
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

    // Bind the constants
    for (i=0; i<_boundPixelShader->largest_constant; i++) {
      bindConstant((PixelShaderHandle) _boundPixelShader,
                   i, _boundPixelShader->constants[i]);
    }

  }
  
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
#ifdef GLES_DEBUG
  printf("numOutputs:%d\n", numOutputs);
#endif

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

  glViewport( minX, minY, width, height );
  CHECK_GL();
#ifdef GLES_DEBUG
printf("minX=%d, minY=%d, width=%d, height=%d\n",  minX, minY, width, height );

  //Immediate mode is not supported in GLES. We need to port this...
  //assert(0);
printf("Program id=%d\n", ((GLESSLPixelShader*)_boundPixelShader)->programid);
#endif

  GLint * texture_locations=(GLint*)malloc(sizeof(void*)*numInterpolants); 

  for (i=0; i<numInterpolants; i++)
  {
      //the vertices are bound to position 1, so after that we have the texture coordinates
      texture_locations[i]= i+1; 
      glVertexAttribPointer( texture_locations[i], 4, GL_FLOAT, 0, 0, (GLfloat*) (interpolants[i].vertices));
      CHECK_GL();
      glActiveTexture(GL_TEXTURE0+i);
      glEnableVertexAttribArray( texture_locations[i]);
      CHECK_GL();
#ifdef GLES_DEBUG
for(int cn=0; cn<6; cn++ )
{
	printf("Text Coord[%d].x=%f\n",cn, interpolants[i].vertices[cn].x);
	printf("Text Coord[%d].y=%f\n",cn, interpolants[i].vertices[cn].y);
	printf("Text Coord[%d].z=%f\n",cn, interpolants[i].vertices[cn].z);
	printf("Text Coord[%d].w=%f\n",cn, interpolants[i].vertices[cn].w);
}
#endif
  }
 
  glVertexAttribPointer( vertex_index, 2, GL_FLOAT, 0, sizeof(float4), outputRegion.vertices);
  CHECK_GL();
  glEnableVertexAttribArray( vertex_index);
  CHECK_GL();
#ifdef GLES_DEBUG
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
#endif

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
	  CHECK_GL_NOT_FATAL();
  }

  free(texture_locations);


//assert(0);
glBindFramebuffer(GL_FRAMEBUFFER,0);
CHECK_GL();
#ifdef GLES_DEBUG
printf("Unbind framebuffer to stop drawing\n");
#endif

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
  for (i=0; i<7 /*TODO see earlier comment*/; i++) 
    if (_boundTextures[i]) {
      printf("Unsetting texture %u from input %d\n", _boundTextures[i]->id(), i);
      glActiveTexture(GL_TEXTURE0+i);
      CHECK_GL();
      glBindTexture(GL_TEXTURE_2D, 0);
      CHECK_GL();
    }
#endif
  for (i=0; i<numOutputs; i++) {
#if defined(_DEBUG) && 0
    if(outputTextures[i]) {
      printf("Unsetting texture %u from output %d\n", outputTextures[i]->id(), i);
      glFramebufferTexture2D(GL_FRAMEBUFFER, outputEnums[i], 
                 GL_TEXTURE_2D, 0, 0);
      CHECK_GL();
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
