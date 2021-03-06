#include "oglfunc.hpp"
#include "../gpucontext.hpp"
#include "oglcheckgl.hpp"



void brook::initglfunc(void) {
   struct HasItem { GLboolean has; const char *description; } hasItems[6]={
      { GLEE_VERSION_1_3,            "OpenGL v1.3 or later" },
      { GLEE_ARB_fragment_program,   "ARB_fragment_program" },
      { GLEE_ARB_occlusion_query,    "ARB_occlusion_query" },
      { GLEE_ARB_texture_float,      "ARB_texture_float" },
      { GLEE_ARB_texture_rectangle,  "ARB_texture_rectangle" },
      { GLEE_EXT_framebuffer_object, "EXT_framebuffer_object"}
   };
   bool hasAll=true;
   for(unsigned int n=0; n<sizeof(hasItems)/sizeof(HasItem); n++)
   {
      if(!hasItems[n].has) {
         if(hasAll) fprintf(stderr, "The following mandatory extensions are not found for graphics card: %s\n", glGetString(GL_RENDERER));
         fprintf(stderr, "   %s\n", hasItems[n].description);
         hasAll=false;
      }
   }
   if(!hasAll) exit(1);
}
