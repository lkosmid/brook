/*
 * codegen.h --
 *
 *      Interface to codegen-- which is the module responsible for taking
 *      parsed brook and spitting out the required gpu assembly and stubs.
 */
#ifndef __CODEGEN_H__
#define __CODEGEN_H__

extern void
CodeGen_CheckSemantics (Type *retType, Decl **args, int nArgs);

extern char *
CodeGen_GenerateStub(Type *retType, const char *name, Decl **args, int nArgs);

extern char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body);

extern char *
CodeGen_GenerateHeader(Type *retType, const char *name, Decl **args, int nArgs);

#endif
