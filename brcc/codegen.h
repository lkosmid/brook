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
   CODEGEN_PS20 = 0,
   CODEGEN_FP30 = 1,
   CODEGEN_ARB  = 2
} CodeGenTarget;

struct ShaderResourceUsage
{
  int arithmeticInstructionCount;
  int textureInstructionCount;
  int samplerRegisterCount;
  int interpolantRegisterCount;
  int constantRegisterCount;
  int temporaryRegisterCount;
  int outputRegisterCount;
};

static inline const char *
CodeGen_TargetName(CodeGenTarget t) {
   return (t == CODEGEN_PS20 ? "ps20" :
           t == CODEGEN_FP30 ? "fp30" :
           "arb");
}

void CodeGen_Init(void);


extern char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body,
                     CodeGenTarget target);

void CodeGen_SplitAndEmitCode(FunctionDef* inFunctionDef,
                              CodeGenTarget target, std::ostream& inStream);

#endif
