/*
 * codegen.h --
 *
 *      Interface to codegen-- which is the module responsible for taking
 *      parsed brook and spitting out the required gpu assembly and stubs.
 */
#ifndef __CODEGEN_H__
#define __CODEGEN_H__

extern char *
CodeGen_FP30GenerateCode(Type *retType, const char *name,
                         Decl **args, int nArgs, const char *body);

extern char *
CodeGen_PS20GenerateCode(Type *retType, const char *name,
                         Decl **args, int nArgs, const char *body);

extern char *
CodeGen_GenerateHeader(Type *retType, const char *name, Decl **args, int nArgs);

#endif
