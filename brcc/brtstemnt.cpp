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
#include "brtreduce.h"
#include "brtdecl.h"
#include "brtexpress.h"
#include "brtscatter.h"
#include "project.h"
#include "codegen.h"
#include "main.h"


//FIXME eventually we'll want to code-transform to do the following 2 functions
bool recursiveIsGather(Type * form) {
  bool ret=(form->type==TT_Array)&&(form->getQualifiers()&TQ_Reduce)==0;
  bool isarray=ret;
  Type * t=form;
  while (isarray) {
    t =static_cast<ArrayType *>(t)->subType;
    isarray= (t->type==TT_Array);
  }
  return ret&&t->type!=TT_Stream;
}
bool recursiveIsStream(Type* form) {
	return (form->type==TT_Stream);
}


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
#define PRINT_IF_NEEDED(var, type) \
   if (globals.target & TARGET_##type) {                                \
      BRTKernelCode *var;                                               \
      var = decl->isReduce() ? new BRT##type##ReduceCode(*this) :       \
                                new BRT##type##KernelCode(*this);       \
      out << *var << std::endl;                                         \
      delete var;                                                       \
   } else {                                                             \
      out << "static const char *__"                                    \
          << *FunctionName() << "_" #var " = NULL;\n";                  \
   }

   if (Project::gDebug) {
      out << "/* BRTKernelDef:" ;
      location.printLocation(out) ;
      out << " */" << std::endl;
   }

   PRINT_IF_NEEDED(fp30, FP30);
   PRINT_IF_NEEDED(ps20, PS20);
   PRINT_IF_NEEDED(cpu, CPU);
   
   printStub(out);
#undef PRINT_IF_NEEDED
   if (decl->isReduce()) {
     BRTCPUReduceCode crc(*this);
      BRTScatterDef sd(*crc.fDef);
      sd.print(out,0);
   }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTKernelDef::printStub(std::ostream& out) const
{
   FunctionType *fType;
   int i;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;
   unsigned int num_templates=0;
   for (i = 0; i < fType->nArgs; i++) {
      if ((fType->args[i]->form->getQualifiers()&TQ_Reduce)!=0) {
         num_templates++;
      }
   }
   if (num_templates) {//to do codegen
      out << "template <";
      for (unsigned int i=0;i<num_templates;++i) {
         if (i!=0)
            out << ", ";
         out << "class _T"<<i;
      }
      out <<">"<<std::endl;
   }
   fType->subType->printType(out, NULL, true, 0);
   out << " " << *FunctionName() << " (";
   num_templates=0;
   for (i = 0; i < fType->nArgs; i++) {
      if (i) out << ",\n\t\t";

      if ((fType->args[i]->form->getQualifiers()&TQ_Reduce)!=0){
         out << "_T"<<num_templates++<<" &" << fType->args[i]->name->name;
      } else if ((fType->args[i]->form->getQualifiers() & TQ_Iter)!=0) {
         out << "const __BRTIter& " << *fType->args[i]->name;
      } else if (recursiveIsStream(fType->args[i]->form) ||
                 recursiveIsGather(fType->args[i]->form)) {
         out << "const __BRTStream& " << *fType->args[i]->name;
      } else {
         out << "const ";
         Symbol name;name.name = fType->args[i]->name->name;
         //XXX -- C++ backend needs values to be passed by value...
         // It's a one time per kernel call hit--worth it to keep
         // Values from being aliased --Daniel
         //hence we only do the & for reduction vars
         fType->args[i]->form->printType(out,&name,true,0);
      }
   }
   out << ") {\n";

   out << "  static const void *__" << *FunctionName() << "_fp[] = {";
   out << std::endl;
   out << "     \"fp30\", __" << *FunctionName() << "_fp30," << std::endl;
   out << "     \"ps20\", __" << *FunctionName() << "_ps20," << std::endl;
   out << "     \"cpu\", (void *) __" << *FunctionName() << "_cpu,"<<std::endl;
   if (this->decl->isReduce()||reduceNeeded(this)) {
      out << "     \"ndcpu\", (void *) __" << *FunctionName() << "_ndcpu,"<<std::endl;
      out << "     \"combine\", (void *) __";
      out << *FunctionName() << "__combine_cpu,";
      out << std::endl;
   }
   out << "     NULL, NULL };"<<std::endl;

   out << "  static __BRTKernel k("
       << "__" << *FunctionName() << "_fp);\n\n";
   for (i=0; i < fType->nArgs; i++) {
      if (recursiveIsStream(fType->args[i]->form) &&
          (fType->args[i]->form->getQualifiers()&TQ_Out)!=0) {
            out << "  k->PushOutput(" << *fType->args[i]->name << ");\n";
      } else if ((fType->args[i]->form->getQualifiers() & TQ_Reduce)!=0) {
         out << "  k->PushReduce(&" << *fType->args[i]->name;
         out << ", __BRTReductionType(&" << *fType->args[i]->name <<"));\n";
      } else if ((fType->args[i]->form->getQualifiers() & TQ_Iter)!=0) {
         out << "  k->PushIter(" << *fType->args[i]->name << ");\n";
      } else if (recursiveIsStream(fType->args[i]->form)) {
         out << "  k->PushStream(" << *fType->args[i]->name << ");\n";
      } else if (recursiveIsGather(fType->args[i]->form)) {
         out << "  k->PushGatherStream(" << *fType->args[i]->name << ");\n";
      } else {
         out << "  k->PushConstant(" << *fType->args[i]->name << ");\n";
      }
   }
   if (decl->isReduce()) {
      out << "  k->Reduce();\n";
   }else {
      out << "  k->Map();\n";
   }
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

      if ((fType->args[i]->form->getQualifiers() & TQ_Out) != 0) {
         if (outArg) {
            std::cerr << "Multiple outputs not supported: ";
            outArg->print(std::cerr, true);
            std::cerr << ", ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }
         outArg = fType->args[i];

         if (!recursiveIsStream(outArg->form)) {
            std::cerr << "Output is not a stream: ";
            outArg->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }

         if ((outArg->form->getQualifiers() & TQ_Iter) != 0) {
            std::cerr << "Output cannot be an iterator: ";
            outArg->print(std::cerr, true);
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
      std::cerr << "Warning, kernel " << *FunctionName() << " has no output.\n";
   }

   return true;
}
