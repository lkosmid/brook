/*
 * cgc.h --
 *
 *      Interface to cgc-- Used to call cgc.exe compiler
 */
#ifndef __CGC_H__
#define __CGC_H__

#include "codegen.h"

char *
compile_cgc (const char *shader, CodeGenTarget target);

#endif
