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
#include "brtkernel.h"
#include "brtdecl.h"
#include "brtexpress.h"
#include "project.h"
#include "codegen.h"


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTKernelDef::BRTKernelDef(const FunctionDef& fDef)
            : FunctionDef(fDef.location)
{
   LabelVector::const_iterator j;

   type = ST_BRTKernel;
   decl = fDef.decl->dup();

   for (Statement *stemnt=fDef.head; stemnt; stemnt=stemnt->next) {
      add(stemnt->dup());
   }

   for (j=fDef.labels.begin(); j != fDef.labels.end(); j++) {
      addLabel((*j)->dup());
   }

   if (!CheckSemantics()) {
      assert(false);
   }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTKernelDef::~BRTKernelDef()
{
   /* Nothing.  We have no addition storage beyond our ancerstor classes */
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTKernelDef::print(std::ostream& out, int) const
{
   BRTFP30KernelCode fp30(*this);
   BRTPS20KernelCode ps20(*this);
   BRTCPUKernelCode cpu(*this);

   if (Project::gDebug) {
      out << "/* BRTKernelDef:" ;
      location.printLocation(out) ;
      out << " */" << std::endl;
   }

   out << fp30 << std::endl << ps20 << std::endl << cpu << std::endl;
   printStub(out);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTKernelDef::printStub(std::ostream& out) const
{
   FunctionType *fType;
   int i;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   fType->subType->printType(out, NULL, true, 0);
   out << " " << *FunctionName() << " (";

   for (i = 0; i < fType->nArgs; i++) {
      if (i) out << ",\n\t\t";

      if (fType->args[i]->isStream() || fType->args[i]->isArray()) {
         out << "const __BRTStream& " << *fType->args[i]->name;
      } else {
         out << "const ";
         fType->args[i]->form->printBase(out, 0);
         fType->args[i]->form->printBefore(out, NULL, 0);
         out << "& " << *fType->args[i]->name;
      }
   }
   out << ") {\n";

   out << "  static const void *__" << *FunctionName() << "_fp[] = {\n";
   out << "     \"fp30\", __" << *FunctionName() << "_fp30,\n";
   out << "     \"ps20\", __" << *FunctionName() << "_ps20,\n";
   out << "     \"cpu\", (void *) __" << *FunctionName() << "_cpu,\n";
   out << "     NULL, NULL };\n";

   out << "  static __BRTKernel k("
       << "__" << *FunctionName() << "_fp);\n\n";

   for (i=0; i < fType->nArgs; i++) {
      if (fType->args[i]->isStream() &&
          fType->args[i]->form->getQualifiers() & TQ_Out) {
            out << "  k->PushOutput(" << *fType->args[i]->name << ");\n";
      } else if (fType->args[i]->isStream()) {
         out << "  k->PushStream(" << *fType->args[i]->name << ");\n";
      } else if (fType->args[i]->isArray()) {
         out << "  k->PushGatherStream(" << *fType->args[i]->name << ");\n";
      } else {
         out << "  k->PushConstant(" << *fType->args[i]->name << ");\n";
      }
   }
   out << "  k->Map();\n";
   out << "\n}\n\n";
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool
BRTKernelDef::CheckSemantics() const
{
   FunctionType *fType;
   Decl *outArg = NULL;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   for (int i = 0; i < fType->nArgs; i++) {
      BaseTypeSpec baseType;

      if (fType->args[i]->form->getQualifiers() & TQ_Out) {
         if (outArg) {
            std::cerr << "Multiple outputs not supported: ";
            outArg->print(std::cerr, true);
            std::cerr << ", ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }
         outArg = fType->args[i];

         if (!fType->args[i]->isStream()) {
            std::cerr << "Output is not a stream: ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }
      }

      baseType = fType->args[i]->form->getBase()->typemask;
      if (baseType < BT_Float || baseType > BT_Float4) {
         std::cerr << "Illegal type in ";
         fType->args[i]->print(std::cerr, true);
         std::cerr << ". (Must be floatN).\n";
         return false;
      }
   }

   /* check kernel return type */
   if (!fType->subType->isBaseType() ||
      ((BaseType *) fType->subType)->typemask != BT_Void) {
      std::cerr << "Illegal kernel return type: " << fType->subType
                << ". Must be void.\n";
      return false;
   }

   if (outArg == NULL) {
      std::cerr << "Warning, kernel has no output.\n";
   }

   return true;
}
