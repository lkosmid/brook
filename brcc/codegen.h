/*
 * codegen.h --
 *
 *      Interface to codegen-- which is the module responsible for taking
 *      parsed brook and spitting out the required gpu assembly and stubs.
 */
#ifndef __CODEGEN_H__
#define __CODEGEN_H__

extern char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body,
                     bool ps20_not_fp30);

void CodeGen_SplitAndEmitCode( FunctionDef* inFunctionDef, bool inIsDirectX, std::ostream& inStream );

#endif
