/*
 * codegen.h --
 *
 *      Interface to codegen-- which is the module responsible for taking
 *      parsed brook and spitting out the required gpu assembly and stubs.
 */
#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include "decl.h"

typedef enum {
   CODEGEN_PS20,
   CODEGEN_FP30,
   CODEGEN_ARB
} CodeGenTarget;

static inline const char *
CodeGen_TargetName(CodeGenTarget t) {
   return (t == CODEGEN_PS20 ? "ps20" :
           t == CODEGEN_FP30 ? "fp30" :
           "arb");
}

extern char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body,
                     CodeGenTarget target);

void CodeGen_SplitAndEmitCode(FunctionDef* inFunctionDef,
                              CodeGenTarget target, std::ostream& inStream);

#endif
