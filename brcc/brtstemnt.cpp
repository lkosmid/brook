/*
 * brtstemnt.cpp
 *
 *      Brook extensions to cTool's stemnt.cpp.  Specifically, contains the
 *      BRTKernelDef class, which represents a kernel definition.
 */
#include "brtvout.h"
//above file must be included first so the #pragma warning:disable is included
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
void
BRTKernelDef::print(std::ostream& out, int) const
{
   if (Project::gDebug) {
      out << "/* BRTKernelDef:" ;
      location.printLocation(out) ;
      out << " */" << std::endl;
   }

#define PRINT_CODE(a,b) \
   if (globals.target & TARGET_##a) {                           \
      BRTKernelCode *var;                                       \
      var = decl->isReduce() ? new BRT##a##ReduceCode(*this) :  \
                               new BRT##a##KernelCode(*this);   \
      out << *var << std::endl;                                 \
      delete var;                                               \
   } else {                                                     \
      out << "static const char *__"                            \
          << *FunctionName() << "_" << #b << "= NULL;\n";       \
   }

   PRINT_CODE(PS20, ps20);
   PRINT_CODE(FP30, fp30);
   PRINT_CODE(CPU,  cpu);
#undef PRINT_CODE

   /*
    * XXX I have no idea why this is here instead of in
    * BRTCPUKernel::print().  It's CPU only and needs to be suppressed when
    * the CPU target is suppressed.  --Jeremy.
    */
   if (decl->isReduce()) {
      if (globals.target & TARGET_CPU) {
         BRTCPUReduceCode crc(*this);
         BRTScatterDef sd(*crc.fDef);
         sd.print(out,0);
      } else {
         /*
          * XXX The multi-threaded CPU is globbed onto CPU.  It really should be
          * its own target type and its own class.  Oh well.
          */

         out << "static const char *__"
             << *FunctionName() << "_ndcpu = NULL;\n\n";
      }
   }

   printStub(out);
}



bool incrementBoolVec(std::vector<bool> &vec) {
   if (vec.empty()) return false;
   bool carry =true;
   for (std::vector<bool>::iterator i=vec.begin();carry&&i!=vec.end();++i){
      carry = *i;
      *i = !(*i);
   }
   return !carry;
}
static std::string getDeclStream(Decl * vout,std::string append="_stream") {
   std::string temp = vout->name->name;
   unsigned int i = temp.find("_stream");
   if (i==std::string::npos) {
      return temp+"_stream";
   }
   return temp.substr(0,i)+append;
}
void BRTKernelDef::PrintVoutPrefix(std::ostream & out) const{
   FunctionType* ft = static_cast<FunctionType*>(decl->form);
   std::set<unsigned int > *vouts= &voutFunctions[FunctionName()->name];
   std::set<unsigned int >::iterator iter;
   out << "  float2 __vout_counter(0.0f, 1.0f / (float)floor (.5));";
   out << std::endl;
   out << "  int maxextents[2]={0,0};"<<std::endl;
   int i=0;   
   for (bool found=0;i<ft->nArgs;++i) {
      if ((ft->args[i]->form->getQualifiers()&TQ_Out)==0
          &&ft->args[i]->isStream()
          && vouts->find(i)==vouts->end()) {
         std::string name = ft->args[i]->name->name;
         if (!found) {
            out << "  unsigned int __dimension = "<<name<<"->getDimension();";
            out << std::endl;
         }
         out << "  assert ("<<name<<"->getDimension()==2);" << std::endl;
         out << "  maxDimension(maxextents,"<<name<<"->getExtents(),";
         out << name << "->getDimension());" << std::endl;
         found=true;
      }
   }
   for (iter = vouts->begin();iter!=vouts->end();++iter) {
      out << "  std::vector<__BRTStream *> ";
      out<<getDeclStream(ft->args[*iter],"_outputs")<<";";
      out << std::endl;
      out << "  bool "<<getDeclStream(ft->args[*iter],"_values")<<" = true;";
      out << std::endl;
   }
   out << "  while (";
   iter = vouts->begin();
   out << getDeclStream (ft->args[*iter++],"_values");
   for (;iter!=vouts->end();++iter) {
      out << " || " << getDeclStream (ft->args[*iter],"_values");
   }
   out << ") {"<<std::endl;
   for (iter = vouts->begin();iter!=vouts->end();++iter) {
      out << "    if ("<<getDeclStream(ft->args[*iter],"_values")<<")";
      out << std::endl;
      out << "      "<<getDeclStream(ft->args[*iter],"_outputs");
      out << ".push_back (new __BRTStream (maxextents, ";
      out << "__dimension, ";
      out << ft->args[*iter]->name->name<<"->getStreamType()));"<<std::endl;
      
   }
}
void BRTKernelDef::PrintVoutPostfix(std::ostream & out) const{
   out << "    __vout_counter.x+=1.0f;"<<std::endl;
   FunctionType* ft = static_cast<FunctionType*>(decl->form);
   std::set<unsigned int >::iterator beginvout
      = voutFunctions[FunctionName()->name].begin();
   std::set<unsigned int >::iterator endvout
      = voutFunctions[FunctionName()->name].end();
   std::set<unsigned int >::iterator iter;
   for (iter = beginvout;iter!=endvout;++iter) {
      Decl * decl = ft->args[*iter];
      out << "    "<<getDeclStream(decl,"_values")<< " = ";
      out << " finiteValueProduced ("<<getDeclStream(decl,"_outputs");
      out << ".back())?1:0;"<<std::endl;
   }
   out << "  }"<<std::endl;
   for (iter = beginvout;iter!=endvout;++iter) {
      Decl * decl = ft->args[*iter];
      out<< "  __BRTStream "<<getDeclStream(decl,"_temp")<<"(";
      out<< decl->name->name<< "->getStreamType(),1,1,-1);"<<std::endl;
      out<< "  combineStreams(&"<<getDeclStream(decl,"_outputs")<<"[0],";
      out<< std::endl;
      out<< "                 "<<getDeclStream(decl,"_outputs")<<".size()-1,";
      out<< std::endl;
      out<< "                 maxextents[0],";
      out<< std::endl;
      out<< "                 maxextents[1],";
      out<< std::endl;
      out<< "                 &"<<getDeclStream(decl,"_temp")<<");";
      out<< std::endl;
      out<< "  shiftValues(&"<<getDeclStream(decl,"_temp")<<",";
      out<< std::endl;
      out<< "              &"<< decl->name->name<<",";
      out<< std::endl;
      out<< "              "<<getDeclStream (decl,"_temp");
      out<< "->getExtents()[0],";
      out<<std::endl;
      out<< "              "<<getDeclStream (decl,"_temp");
      out<< "->getExtents()[1],";
      out <<std::endl;
      out<< "              -1);"<<std::endl; 
      
   }
}
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTKernelDef::printStub(std::ostream& out) const
{
   FunctionType *fType;
   int i,NumArgs;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;
   std::vector <bool> streamOrVal;
   NumArgs=fType->nArgs;
   bool vout=voutFunctions.find(FunctionName()->name)!=voutFunctions.end();
   if (vout) {
      out << "extern int finiteValueProduced (struct __BRTStream * input);\n"
             "extern float shiftValues(struct __BRTStream *list_stream,\n"
             "                         struct __BRTStream *output_stream,\n"
             "                         int WIDTH, \n"
             "                         int LENGTH, \n"
             "                         int sign);\n"
             "void combineStreams (struct __BRTStream **streams,\n"
             "                     unsigned int num,\n"
             "                     unsigned int width, \n"
             "                     unsigned int length,\n"
             "                     struct __BRTStream * output) ;\n";
   }
   if (vout) {
      NumArgs--;
   }
   for (i = 0; i < NumArgs; i++) {
      if ((fType->args[i]->form->getQualifiers()&TQ_Reduce)!=0) {
         streamOrVal.push_back(false);
      }
   }
   do {
      unsigned int reducecount=0;
      fType->subType->printType(out, NULL, true, 0);
      out << " " << *FunctionName() << " (";
      
      for (i = 0; i < NumArgs; i++) {
         if (i) out << ",\n\t\t";
         
         if ((fType->args[i]->form->getQualifiers()&TQ_Reduce)!=0){
            if (streamOrVal[reducecount++]) {
               Symbol name;name.name = "& "+fType->args[i]->name->name;
               Type * t = fType->args[i]->form;
               if (fType->args[i]->isStream())
                  t = static_cast<ArrayType*>(fType->args[i]->form)->subType;                  
               t->printType(out,&name,true,0);
            }else{
               out << "__BRTStream& "<< *fType->args[i]->name;
            }
         } else if ((fType->args[i]->form->getQualifiers() & TQ_Iter)!=0) {
            out << "const __BRTIter& " << *fType->args[i]->name;
         } else if (recursiveIsStream(fType->args[i]->form) ||
                    recursiveIsGather(fType->args[i]->form)) {
            if ((fType->args[i]->form->getQualifiers()&TQ_Out)==0) {
               out << "const ";
            }
            out << "__BRTStream& " << *fType->args[i]->name;
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
         if (globals.multiThread) {//only make combiner if needed
            out << "     \"combine\", (void *) __";
            out << *FunctionName() << "__combine_cpu,";
            out << std::endl;
         }
      }
      if (!globals.multiThread) {
         out << "     \"combine\", 0,";//this signals to runtime
         //not to use multithreading
         out << std::endl;
      }
      out << "     NULL, NULL };"<<std::endl;
      
      out << "  static __BRTKernel k("
          << "__" << *FunctionName() << "_fp);\n\n";
      if (vout) {
         PrintVoutPrefix(out);
      }
      for (i=0; i < fType->nArgs; i++) {
         if (vout)
            out <<"  ";//nice spacing
         if (recursiveIsStream(fType->args[i]->form) &&
             (fType->args[i]->form->getQualifiers()&TQ_Out)!=0) {
            
            if (voutFunctions.find(FunctionName()->name)==voutFunctions.end()
                ||  voutFunctions[FunctionName()->name].find(i)
                    == voutFunctions[FunctionName()->name].end()) {
               out << "  k->PushOutput(" << *fType->args[i]->name << ");\n";
            }else {
               out << "  k->PushOutput(*" << getDeclStream(fType->args[i],
                                                           "_outputs");
               out << ".back());\n";
            }
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
      if (vout)
         out <<"  ";//nice spacing
      if (decl->isReduce()) {
         out << "  k->Reduce();\n";
      }else {
         out << "  k->Map();\n";
      }
      if (vout)
         PrintVoutPostfix(out);
      out << "\n}\n\n";
   }while (incrementBoolVec(streamOrVal));
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool
BRTKernelDef::CheckSemantics() const
{
   FunctionType *fType;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   for (int i = 0; i < fType->nArgs; i++) {
      BaseTypeSpec baseType;

      baseType = fType->args[i]->form->getBase()->typemask;
      if (baseType < BT_Float || baseType > BT_Float4) {
         std::cerr << location << "Illegal type in ";
         fType->args[i]->print(std::cerr, true);
         std::cerr << ". (Must be floatN).\n";
         return false;
      }
   }

   if (!fType->subType->isBaseType() ||
      ((BaseType *) fType->subType)->typemask != BT_Void) {
      std::cerr << location << "Illegal return type for kernel "
                << *FunctionName() << ": " << fType->subType
                << ". Must be void.\n";
      return false;
   }

   return true;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool
BRTMapKernelDef::CheckSemantics() const
{
   FunctionType *fType;
   Decl *outArg = NULL;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   for (int i = 0; i < fType->nArgs; i++) {
      if (fType->args[i]->isReduce()) {
         std::cerr << location << "Reduce arguments are not allowed in "
                   << *FunctionName() << ": ";
         fType->args[i]->print(std::cerr, true);
         std::cerr << ".\n";
         return false;
      }

      if ((fType->args[i]->form->getQualifiers() & TQ_Out) != 0) {
         if (outArg) {
            std::cerr << location << "Multiple outputs not supported: ";
            outArg->print(std::cerr, true);
            std::cerr << ", ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }
         outArg = fType->args[i];

         if (!recursiveIsStream(outArg->form)) {
            std::cerr << location << "Output is not a stream: ";
            outArg->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }

         if ((outArg->form->getQualifiers() & TQ_Iter) != 0) {
            std::cerr << location << "Output cannot be an iterator: ";
            outArg->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }
      }
   }

   if (outArg == NULL) {
      std::cerr << location << "Warning: " << *FunctionName()
                << " has no output.\n";
   }

   return true;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool
BRTReduceKernelDef::CheckSemantics() const
{
   FunctionType *fType;
   Decl *streamArg = NULL, *reduceArg = NULL;

   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;

   for (int i = 0; i < fType->nArgs; i++) {
      if (fType->args[i]->isReduce()) {
         if (reduceArg != NULL) {
            std::cerr << location << "Multiple reduce arguments in "
                      << *FunctionName() << ": ";
            reduceArg->print(std::cerr, true);
            std::cerr << ", ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }

         reduceArg = fType->args[i];
      } else if (fType->args[i]->isStream()) {
         if (streamArg != NULL) {
            std::cerr << location << "Multiple non-reduce streams in "
                      << *FunctionName() << ": ";
            streamArg->print(std::cerr, true);
            std::cerr << ", ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }

         streamArg = fType->args[i];
      }

      if ((fType->args[i]->form->getQualifiers() & TQ_Out) != 0) {
         std::cerr << location << "Non-reduce output in reduction kernel "
                   << *FunctionName() << ".\n";
         return false;
      }
   }

   if (reduceArg == NULL) {
      std::cerr << location << "Reduction kernel " << *FunctionName()
                << " has no reduce argument.\n";
      return false;
   }

   if (streamArg == NULL) {
      std::cerr << location << "Reduction kernel " << *FunctionName()
                << " has no stream argument.\n";
      return false;
   }

   return true;
}
