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
    * The scatter functions need to be there whether or not the CPU target is repressed
    * For the fallback requirement --Daniel
    */
   if (decl->isReduce()) {
     BRTCPUReduceCode crc(*this);
     BRTScatterDef sd(*crc.fDef);
     sd.print(out,0); 
     // this is needed for CPU fallback for scatter whether or not CPU is enabled
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
   out << "  float __vout_counter=0.0f;"<<std::endl;
   out << "  brook::Stream * __inf = *sentinelStream(1);";
   out << std::endl;
   out << "  int maxextents[2]={0,0};"<<std::endl;
   unsigned int i=0;   
   for (bool found=0;i<ft->nArgs;++i) {
      if ((ft->args[i]->form->getQualifiers()&TQ_Out)==0
          &&ft->args[i]->isStream()
          && vouts->find(i)==vouts->end()) {
         std::string name = ft->args[i]->name->name;
         if (!found) {
            out << "  unsigned int __dimension = "<<name<<"->getDimension();";
            out << std::endl;
         }
         out << "  assert ("<<name<<"->getDimension()<=2);" << std::endl;
         out << "  maxDimension(maxextents,"<<name<<"->getExtents(),";
         out << name << "->getDimension());" << std::endl;
         found=true;
      }
   }
   for (iter = vouts->begin();iter!=vouts->end();++iter) {
      out << "  std:: vector <__BRTStreamType> ";
      std::string typevector = getDeclStream(ft->args[*iter],"_types");
      out << typevector <<";"<<std::endl;
      std::string streamiter = getDeclStream(ft->args[*iter],"_iter");
      out << "  for (int "<<streamiter << " = 0; ";
      out <<streamiter<<" < "<<ft->args[*iter]->name->name;
      out <<"->getFieldCount(); ++";
      out << streamiter << ") \n";
      out << "    "<<typevector<<".push_back("<<ft->args[*iter]->name->name;
      out << "->getIndexedFieldType("<<streamiter<<"));\n";
      out << "  "<<typevector<<".push_back(__BRTNONE);\n";
     out << "  std::vector<brook::stream*> ";
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
      std::string typevector = getDeclStream(ft->args[*iter],"_types");
      out << "    if ("<<getDeclStream(ft->args[*iter],"_values")<<")";
      out << std::endl;
      out << "      "<<getDeclStream(ft->args[*iter],"_outputs");
      out << ".push_back (new ::brook::stream (maxextents, ";
      out << "__dimension, &"<<typevector<<"[0]));";
      out <<std::endl;      
   }
}
std::string undecoratedBase(Decl * decl) {
   BaseType * base = decl->form->getBase();
   BaseTypeSpec typemask = base->typemask;
   if (typemask&BT_Float2)
      return "float2";
   else if (typemask&BT_Float3)
      return "float3";
   else if (typemask&BT_Float4)
      return "float4";
   return "float";
         
}
std::string getDimensionString (int dim) {
      std::string dimensionstring;
      if (dim!=2) {
         dimensionstring +=dim+'0';
         dimensionstring+='d';
      }

      return dimensionstring;
}
void BRTKernelDef::PrintVoutDimensionalShift(std::ostream &out, 
                                             Decl* decl,
                                             unsigned int dim) const {
   
      std::string type = undecoratedBase(decl);
      unsigned int i;
      std::string dimensionstring = getDimensionString(dim);
      out<< "    ::brook::stream "<<getDeclStream(decl,"_temp")<<"(&";
      std::string typevector = getDeclStream(decl,"_types");
      out<< typevector<<"[0],";
      for (i=0;i<dim;++i) {
         out << "1, ";
      }
      out << "-1);"<<std::endl;
      out<< "    combineStreams"<<dimensionstring;
      out << type <<" (&"<<getDeclStream(decl,"_outputs")<<"[0],";
      out<< std::endl;
      out<< "                   "<<getDeclStream(decl,"_outputs")<<".size()-1,";
      out<< std::endl;
      out<< "                   maxextents[0],";
      out<< std::endl;
      out<< "                   maxextents[1],";
      out<< std::endl;
      out<< "                   "<<getDeclStream(decl,"_temp")<<");";
      out<< std::endl;
      out<< "    shiftValues"<<dimensionstring;
      out << type << "("<<getDeclStream(decl,"_temp")<<",";
      out<< std::endl;
      out<< "                "<< decl->name->name<<",";
      out<< std::endl;
      for (i=0;i<dim;++i) {
         out<< "                "<<getDeclStream (decl,"_temp");
         out<< "->getExtents()["<<i<<"],";
         out<<std::endl;
      }
      for (;i<2;++i) out << "                 1,";
      out<< "                -1);"<<std::endl; 
}
void BRTKernelDef::PrintVoutPostfix(std::ostream & out) const{
   out << "    __vout_counter+=1.0f;"<<std::endl;
   FunctionType* ft = static_cast<FunctionType*>(decl->form);
   std::set<unsigned int >::iterator beginvout
      = voutFunctions[FunctionName()->name].begin();
   std::set<unsigned int >::iterator endvout
      = voutFunctions[FunctionName()->name].end();
   std::set<unsigned int >::iterator iter;
   
   for (iter = beginvout;iter!=endvout;++iter) {
      Decl * decl = ft->args[*iter];
      Expression * vout_limit = decl->form->getQualifiers().vout;
      
      if (vout_limit) {
        bool useparen=(vout_limit->precedence() < 
                       RelExpr(RO_Less,
                               NULL,
                               NULL,
                               vout_limit->location).precedence());
        // the above is a simple check for the common expressions.
        // no need to get fancy here for parens are ok in this case.
        out <<"    if (__vout_counter >= ";
        if (useparen) out << "(";
        vout_limit->print(out);
        if (useparen) out << ")";
        out <<") {"<<std::endl;
        out <<"      ";
        out <<getDeclStream(decl,"_outputs")<<".push_back(0);"<<std::endl;
        out <<"      ";
        out <<"break;"<<std::endl;
        out <<"    }"<<std::endl;
      }
      out << "    "<<getDeclStream(decl,"_values")<< " = ";

      out << "(";
      out << decl->name->name<<"->getDimension()==2?";
      out << "finiteValueProduced"<<getDimensionString(2)<<undecoratedBase(decl);
      out << ":finiteValueProduced"<<getDimensionString(1);      
      out << undecoratedBase(decl)<<")(*"<<getDeclStream(decl,"_outputs");
      out << ".back())?1:0;"<<std::endl;
   }
   out << "  }"<<std::endl;
   for (iter = beginvout;iter!=endvout;++iter) {
      
      Decl * decl = ft->args[*iter];
      out << "  if ("<<decl->name->name<<"->getDimension()==2) {"<<std::endl;
      PrintVoutDimensionalShift(out,decl,2);
      out << "  }else {"<<std::endl;
      PrintVoutDimensionalShift(out,decl,1);
      out << "  }"<<std::endl;
      out << "  while ("<<getDeclStream(decl,"_outputs")<<".size()) {";
      out << std::endl;
      out << "    if ("<<getDeclStream(decl,"_outputs")<<".back())"<<std::endl;
      out << "      delete "<<getDeclStream(decl,"_outputs")<<".back();";
      out << std::endl;
      out << "    "<<getDeclStream(decl,"_outputs")<<".pop_back();"<<std::endl;
      out << "  }"<<std::endl;
   }
}
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
static void printPrototypes(std::ostream & out, std::string type) {
   int i;
   for (i=1;i<=2;++i) {
      std::string dimensionstring = getDimensionString (i);
      out << "extern int finiteValueProduced"<<dimensionstring ;
      out << type << " (brook::stream &input);\n"      ;
      out <<"extern float shiftValues"<<dimensionstring;
      out << type << "(brook::stream &list_stream,\n"
         "                         brook::stream& output_stream,\n"
         "                         int WIDTH, \n"
         "                         int LENGTH, \n"
         "                         int sign);\n"
         "void combineStreams"<<dimensionstring;
      out << type << "(brook::stream **streams,\n"
         "                     unsigned int num,\n"
         "                     unsigned int width, \n"
         "                     unsigned int length,\n"
         "                     brook::stream &output) ;\n";
      
   }
}
void
BRTKernelDef::printStub(std::ostream& out) const
{
  if (!returnsVoid())return;
   FunctionType *fType;
   unsigned int i,NumArgs;
   bool vout=voutFunctions.find(FunctionName()->name)!=voutFunctions.end();
   if (vout) {
      printPrototypes (out,"float");
      printPrototypes (out,"float2");
      printPrototypes (out,"float3");
      printPrototypes (out,"float4");
   }
   assert (decl->form->type == TT_Function);
   fType = (FunctionType *) decl->form;
   std::vector <bool> streamOrVal;
   NumArgs=fType->nArgs;

   if (vout) {

   }
   if (vout) {
     NumArgs-=2;//one for counter one for innf
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
               out << "::brook::stream& "<< *fType->args[i]->name;
            }
         } else if ((fType->args[i]->form->getQualifiers() & TQ_Iter)!=0) {
            out << "const __BRTIter& " << *fType->args[i]->name;
         } else if (recursiveIsStream(fType->args[i]->form) ||
                    recursiveIsGather(fType->args[i]->form)) {
            if ((fType->args[i]->form->getQualifiers()&TQ_Out)==0) {
               out << "const ";
            }
            out << "::brook::stream& " << *fType->args[i]->name;
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
         if (globals.target&TARGET_MULTITHREADED_CPU) {//only make combiner if needed
            out << "     \"combine\", (void *) __";
            out << *FunctionName() << "__combine_cpu,";
            out << std::endl;
         }
      }
      if ((globals.target&TARGET_MULTITHREADED_CPU)==0) {
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
   for (unsigned int i = 0; i < fType->nArgs; i++) {
      if (!fType->args[i]->isStream() &&
            (fType->args[i]->form->getQualifiers() & TQ_Iter) != 0) {
         std::cerr << location << "'";
         fType->args[i]->print(std::cerr, true);
         std::cerr << "' is tagged an iter, but is not a stream!\n";
         return false;
      }

/*TIM: remove type checking for arglist (because of structs)
      BaseTypeSpec baseType;

      baseType = fType->args[i]->form->getBase()->typemask;
      if (baseType < BT_Float || baseType > BT_Float4) {
         std::cerr << location << "Illegal type in ";
         fType->args[i]->print(std::cerr, true);
         std::cerr << ". (Must be floatN).\n";
         return false;
      }
*/
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

   for (unsigned int i = 0; i < fType->nArgs; i++) {
      if (fType->args[i]->isReduce()) {
         std::cerr << location << "Reduce arguments are not allowed in "
                   << *FunctionName() << ": ";
         fType->args[i]->print(std::cerr, true);
         std::cerr << ".\n";
         return false;
      }

      if ((fType->args[i]->form->getQualifiers() & TQ_Out) != 0) {
 /*        if (outArg) {
            std::cerr << location << "Multiple outputs not supported: ";
            outArg->print(std::cerr, true);
            std::cerr << ", ";
            fType->args[i]->print(std::cerr, true);
            std::cerr << ".\n";
            return false;
         }*/
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

   if (outArg == NULL&&returnsVoid()) {
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

   for (unsigned int i = 0; i < fType->nArgs; i++) {
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
