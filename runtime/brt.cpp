#include <brook.hpp>
#include <brt.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include <dx9/dx9.hpp>
#include <nv30gl/nv30gl.hpp>

// Perform runtime initialization
__BrookRunTime *__brt()
{
    static __BrookRunTime* result = RunTimeFactory();
    return result;
}

__BrookRunTime *RunTimeFactory(void) {
  char *env = getenv(RUNTIME_ENV_VAR);

  if (!env) 
    return new NV30GLRunTime();

  if (!strcmp(env, DX9_RUNTIME_STRING))
    return new DX9RunTime();

  if (!strcmp(env, NV30GL_RUNTIME_STRING))
    return new NV30GLRunTime();

  fprintf (stderr, "Unknown runtime requested: %s", env);
  abort();
}

__BrookStream* CreateStream(const char type[], ... )
{
  int dimensions = 0;
  int extents[__MAXSTREAMDIMS];

  va_list args;
  va_start(args,type);
  int extent = 0;
  for(;;)
  {
    extent = va_arg(args,int);
    if( extent <= 0 ) break;
    extents[dimensions++] = extent;
  }
  va_end(args);

  return __brt()->CreateStream(type,dimensions,extents);
}
