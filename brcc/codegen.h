/*
 * codegen.h --
 *
 *      Interface to codegen-- which is the module responsible for taking
 *      parsed brook and spitting out the required gpu assembly and stubs.
 */
#ifndef __CODEGEN_H__
#define __CODEGEN_H__

struct arglisttype;

extern void
CodeGen_Kernel(Type *retType, const char *name,
               Decl **args, int nArgs, const char *body);

#endif
