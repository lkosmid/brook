/*
 * brtstemnt.cpp
 *
 *      Brook extensions to cTool's stemnt.cpp.  Specifically, contains the
 *      BRTKernelDef class, which represents a kernel definition.
 */
#include <cstring>
#include <cassert>
#include <sstream>
#include "brtdecl.h"
#include "brtstemnt.h"
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
   int i;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   fType->subType->printType(out, NULL, true, 0);
   out << " " << *FunctionName() << " (";

   for (i = 0; i < fType->nArgs; i++) {
      if (i) out << ",\n\t\t";

      if (fType->args[i]->isStream()) {
         out << "const __BRTStream& " << *fType->args[i]->name;
      } else {
         out << "const ";
         fType->args[i]->form->printBase(out, 0);
         fType->args[i]->form->printBefore(out, NULL, 0);
         out << "& " << *fType->args[i]->name;
      }
   }
   out << ") {\n";

   out << "  static __BRTKernel k("
       << "__" << *FunctionName() << "_fp);\n\n";

   for (i=0; i < fType->nArgs; i++) {
      if (fType->args[i]->isStream()) {
         if (fType->args[i]->form->getQualifiers() & TQ_Out) {
            out << "  k->PushOutput(" << *fType->args[i]->name << ");\n";
         } else {
            out << "  k->PushStream(" << *fType->args[i]->name << ");\n";
         }
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

   fpcode = CodeGen_HLSLGenerateCode(fType->subType, FunctionName()->name.c_str(),
                                     fType->args, fType->nArgs, wrapOut.str().c_str());
   out << fpcode;
   free(fpcode);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void printType (std::ostream & out, Type * t, bool addIndirection) {
		Symbol sym;
		sym.name="";
	
		t->printBase(out,0);
		t->printBefore(out,&sym,0);
		if (addIndirection) out << "*";			
		t->printAfter(out);
}
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTCPUKernelDef::printCode(std::ostream& out) const
{
//   out << "#error \"Don't know how to generate CPU kernel code for "
//       << FunctionName()->name << "().\"\n";

  /* We've already transformed everything, so just print ourselves */
  Type * form = decl->form;
  assert (form->isFunction());
  FunctionType* func = static_cast<FunctionType *>(form->dup());
    
    out << "void ";//we don't want to automatically print this for it would say "kernel void" which means Nothing
	Symbol enhanced_name;
	enhanced_name.name = "_cpu_"+decl->name->name;
    func->printBefore(out,&enhanced_name,0);
    
    out << "(";
    for (int j=0; j < func->nArgs; j++) {
	if (j!=0)
	    out << ", ";
	Type * form = func->args[j]->form;
	bool copy_on_write=false;
	if (form->type==TT_Stream||form->type==TT_Array) {
	    if (copy_on_write&&form->type==TT_Array) {
		form = new CPUGatherType(*static_cast<ArrayType *>(form));
	    }else if (form->type==TT_Stream){
		form = static_cast<ArrayType *>(form)->subType;
	    }
	}
	
	TypeQual tq= form->getQualifiers();
	if (!copy_on_write) {
		if ((tq&TQ_Const)==0&&(tq&TQ_Out)==0){
			out << "const ";//kernels are only allowed to touch out params
		}
		if (/*(tq&TQ_Out)==0&&*/form->type!=TT_Array/*no gather*/)
			func->args[j]->name->name=std::string("&")+func->args[j]->name->name;
	}
	func->args[j]->form = form;
	func->args[j]->print(out,true);
	
    }
    
    out << ")";
    //delete func;
	func = static_cast<FunctionType *>(form);
    Block::print(out,0);
	//now it's time to print the automated wrapper function.
	out << "void ";//we don't want to automatically print this for it would say "kernel void" which means Nothing
	enhanced_name.name = "_cpu_loop_"+decl->name->name;
	func->printBefore(out,&enhanced_name,0);
	out << "(const std::vector<char *>&args,unsigned int mapbegin,unsigned int mapend) {"<<std::endl;
	indent(out,1);
	out << "for (unsigned int i=mapbegin;i<mapend;++i) {"<<std::endl;
	indent(out,2);out << "_cpu_" <<decl->name->name<<" ("<<std::endl;
	{for (unsigned int i=0;i<func->nArgs;++i) {
		if (i!=0)
			out <<","<<std::endl;
		indent(out,3);
		
		if (func->args[i]->form->type==TT_Stream) {
			Type * t=static_cast<ArrayType*>(func->args[i]->form)->subType;
			out <<"*(";
			printType(out,t,true);						
			out << ")(args["<<i<<"]+i*sizeof(";
			printType(out,t,false);									
			out <<"))";
		}else {

			bool isArray = (func->args[i]->form->type==TT_Array);
			if (!isArray) out << "*";
			out << "(";
			printType(out,func->args[i]->form,!isArray);			
			out <<")(args["<<i<<"])";
		}
	}}
	out<< ");"<<std::endl;
	indent(out,1);out <<"}"<<std::endl;
	out << "}"<<std::endl;
	
}
