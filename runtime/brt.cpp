#include <brook.hpp>
#include <brt.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <dx9/dx9.hpp>
#include <nv30gl/nv30gl.hpp>

// Perform runtime initialization
__BrookRunTime *__brt = RunTimeFactory();

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

__BrookIntArray::__BrookIntArray(...) {
  // XXX To Do
  v = (int*) 0;
}


__BrookIntArray::~__BrookIntArray() {
  if (v)
    delete v;
}
