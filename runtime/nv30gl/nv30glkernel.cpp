
#include <stdio.h>

#include "nv30gl.hpp"

using namespace brook;

NV30GLKernel::NV30GLKernel(NV30GLRunTime * runtime,
                           const void *sourcelist[]) :
   runtime(runtime)
{
   int i;
   const char *source;
   
   assert (sourcelist);
      
   for( i=0; sourcelist[i] != NULL; i+=2 ) {
      
      const char* nameString = (const char*) sourcelist[i];
      assert (nameString);

      source = (const char*) sourcelist[i+1];
      
      if( strncmp( nameString, 
                   NV30GL_RUNTIME_STRING, 
                   strlen(NV30GL_RUNTIME_STRING))  == 0 )
         break;
  }
   
   if (sourcelist[i] == NULL ||
       source == NULL) { 
      fprintf (stderr, "NV30GL: No kernel source found\n");
      exit(1);
   }

      
  /* Load the program code */
  CHECK_GL();

  glGenProgramsNV (1, &id);
  glLoadProgramNV (GL_FRAGMENT_PROGRAM_NV, id, strlen(source), 
		   (const GLubyte*) source);
  glBindProgramNV (GL_FRAGMENT_PROGRAM_NV, id);

  /* Check for program errors */
  if (glGetError() == GL_INVALID_OPERATION) {
    GLint pos;
    int i;
    int line, linestart;
    char *progcopy = strdup (source);
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &pos);
    
    line = 1;
    linestart = 0;
    for (i=0; i<pos; i++) {
      if (progcopy[i] == '\n') {
	line++;
	linestart = i+1;
      }
    }
    fprintf ( stderr, "NV30GL: Program Error on line %d\n", line);
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

    exit(1);
  }
}

void NV30GLKernel::PushStream(Stream *s) {
   // XXX: TODO
   abort();
}

void NV30GLKernel::PushIter(Iter *s) {
   // XXX: TODO
   abort();
}

void NV30GLKernel::PushConstant(const float &val) {
  // XXX: TODO
   abort();
}

void NV30GLKernel::PushConstant(const float2 &val) {
  // XXX: TODO
   abort();
}

void NV30GLKernel::PushConstant(const float3 &val) {
  // XXX: TODO
   abort();
}

void NV30GLKernel::PushConstant(const float4 &val) {
  // XXX: TODO
   abort();
}
void NV30GLKernel::PushReduce(void * val, __BRTStreamType) {
  // XXX: TODO
   abort();
}

void NV30GLKernel::PushGatherStream(Stream *s) {
  // XXX: TODO
   abort();
}

void NV30GLKernel::PushOutput(Stream *s) {
  // XXX: TODO
   abort();
}

void NV30GLKernel::Map() {
  // XXX: TODO
   abort();
}

NV30GLKernel::~NV30GLKernel() {
  // Does nothing
   abort();
}
