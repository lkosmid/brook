/*
 * brtstemnt.cpp
 *
 *      Brook extensions to cTool's stemnt.cpp.  Specifically, contains the
 *      BRTKernelDef class, which represents a kernel definition.
 */
#include <cstring>
#include <cassert>
#include <sstream>

#include "brtstemnt.h"
#include "project.h"
#include "codegen.h"


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTKernelDef::BRTKernelDef(const FunctionDef& fDef)
            : FunctionDef(fDef.location)
{
   FunctionType *fType;
   LabelVector::const_iterator j;

   assert (fDef.decl->form->type == TT_Function);
   fType = (FunctionType *) fDef.decl->form;
   CodeGen_CheckSemantics(fType->subType, fType->args, fType->nArgs);

   type = ST_BRTKernel;
   decl = fDef.decl->dup();

   for (Statement *stemnt=fDef.head; stemnt; stemnt=stemnt->next) {
      add(stemnt->dup());
   }

   for (j=fDef.labels.begin(); j != fDef.labels.end(); j++) {
      addLabel((*j)->dup());
   }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTKernelDef::~BRTKernelDef()
{
   /* Nothing.  We have no addition storage beyond our ancerstor classes */
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Statement*
BRTKernelDef::dup0() const
{
   /* Pass ourselves (as a FunctionDef) to our own constructor */
   assert(0); /* We're actually pure virtual */
   return NULL;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTKernelDef::print(std::ostream& out, int) const
{
   if (Project::gDebug) {
      out << "/* BRTKernelDef:" ;
      location.printLocation(out) ;
      out << " */" << std::endl;
   }

   printCode(out);
   printStub(out);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTKernelDef::printStub(std::ostream& out) const
{
   FunctionType *fType;
   char *stub;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   stub = CodeGen_GenerateStub(fType->subType, FunctionName()->name.c_str(),
                               fType->args, fType->nArgs);
   out << stub;
   free(stub);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTGPUKernelDef::printCode(std::ostream& out) const
{
   FunctionType *fType;
   std::ostringstream wrapOut;
   char *fpcode;

   Block::print(wrapOut, 0);
   if (Project::gDebug) {
      out << "***Wrapping\n";
      decl->print(out, true);
      out << std::endl << wrapOut.str() << "\n***\n";
   }

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   fpcode = CodeGen_GenerateCode(fType->subType, FunctionName()->name.c_str(),
                               fType->args, fType->nArgs, wrapOut.str().c_str());
   out << fpcode;
   free(fpcode);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTCPUKernelDef::printCode(std::ostream& out) const
{
   out << "#error \"Don't know how to generate CPU kernel code for "
       << FunctionName()->name << "().\"\n";

   /* We've already transformed everything, so just print ourselves */
   FunctionDef::print(out, 0);
}
