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
   BRTFP30KernelCode fp30(*this);
   BRTPS20KernelCode ps20(*this);

   if (Project::gDebug) {
      out << "/* BRTKernelDef:" ;
      location.printLocation(out) ;
      out << " */" << std::endl;
   }
   out << fp30 << std::endl << ps20 << std::endl;
   BRTCPUKernelCode cpu(*this);
   out << cpu << std::endl;

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

      if (recursiveIsStream(fType->args[i]->form) || recursiveIsGather(fType->args[i]->form)) {
         out << "const __BRTStream& " << *fType->args[i]->name;
      } else {
         bool reduction = (fType->args[i]->form->getQualifiers()&TQ_Reduce)!=0;
         if (!reduction)
            out << "const ";
         Symbol name;name.name = fType->args[i]->name->name;
         //XXX -- C++ backend needs values to be passed by value...
         // It's a one time per kernel call hit--worth it to keep
         // Values from being aliased --Daniel
         //hence we only do the & for reduction vars
         if (reduction&&fType->args[i]->form->type!=TT_Array)
            //arrays are automatically pass by ref. - Daniel
            name.name= "& "+name.name;
         fType->args[i]->form->printType(out,&name,true,0);
      }
   }
   out << ") {\n";

   out << "  static const void *__" << *FunctionName() << "_fp[] = {";
   out << std::endl;
   out << "     \"fp30\", __" << *FunctionName() << "_fp30," << std::endl;
   out << "     \"ps20\", __" << *FunctionName() << "_ps20," << std::endl;
   out << "     \"cpu\", (void *) __" << *FunctionName() << "_cpu,"<<std::endl;
   if (reduceNeeded(this)) {
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
      } else if (recursiveIsStream(fType->args[i]->form)) {
         out << "  k->PushStream(" << *fType->args[i]->name << ");\n";
      } else if (recursiveIsGather(fType->args[i]->form)) {
         out << "  k->PushGatherStream(" << *fType->args[i]->name << ");\n";
      } else if ((fType->args[i]->form->getQualifiers()&TQ_Reduce)!=0) {
         out << "  k->PushReduce(&" << *fType->args[i]->name;
         BaseTypeSpec bs= fType->args[i]->form->getBase()->typemask;
         switch (FloatDimension(bs)){
         case 2:
            out <<", __BRTFLOAT2";
            break;
         case 3:
            out <<", __BRTFLOAT3";
            break;
         case 4:
            out <<", __BRTFLOAT4";
            break;
         default:
            out <<", __BRTFLOAT";
         }
         out <<");\n";
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

      if ((fType->args[i]->form->getQualifiers() & TQ_Out)!=0) {
         if (outArg) {
            std::cerr << "Multiple outputs not supported: ";
            outArg->print(std::cerr, true);
            std::cerr << ", ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }
         outArg = fType->args[i];

         if (!recursiveIsStream(fType->args[i]->form)) {
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
