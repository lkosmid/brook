/*
 * brtscatter.cpp
 *
 *      Actual code to convert reduce functions into scatter functors on CPU
 */
#include <cstring>
#include <cassert>
#include <sstream>

#include "brtstemnt.h"
#include "brtscatter.h"
#include "brtdecl.h"
#include "brtexpress.h"
#include "project.h"
#include "brtkernel.h"
using std::endl;
void convertNameToScatter (Variable * v) {
                  if (v->name->name!="STREAM_SCATTER_ASSIGN"&&
                      v->name->name!="STREAM_SCATTER_FLOAT_MUL"&&
                      v->name->name!="STREAM_SCATTER_FLOAT_ADD"&&
                      v->name->name!="STREAM_SCATTER_INTEGER_ADD"&&
                      v->name->name!="STREAM_SCATTER_INTEGER_MUL") {

                     if (v->name->name.find("__")!=0||
                         v->name->name.find("_scatter")==std::string::npos) {
                        v->name=new Symbol(*v->name);
                        v->name->name="__"+v->name->name+"_scatter";
                     }                     
                  }   
}
Expression * ConvertToBrtScatterCalls(Expression * e) {
   if (e->etype==ET_FunctionCall) {
      FunctionCall * fc = static_cast<FunctionCall*>(e);
      if (fc->function->etype==ET_Variable) {
         Variable * name = static_cast<Variable*>(fc->function);
         if (name->name->name=="streamScatterOp") {
            if (fc->nArgs()>3) {
               if (fc->args[3]->etype==ET_Variable) {
                  Variable *v=static_cast<Variable*>(fc->args[3]);
                  convertNameToScatter(v);
               }else if (fc->args[3]->etype==ET_FunctionCall) {
                  FunctionCall * cawl = 
                     static_cast<FunctionCall*>(fc->args[3]);
                  if (cawl->function->etype==ET_Variable) {
                     convertNameToScatter(static_cast<Variable*>
                                          (cawl->function));
                  }
               }
            }
         }
      }
   }
   return e;
}



// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTScatterDef::BRTScatterDef(const FunctionDef& fDef)
            : FunctionDef(fDef.location)
{
   LabelVector::const_iterator j;

   type = ST_BRTScatter;
   decl = fDef.decl->dup();

   for (Statement *stemnt=fDef.head; stemnt; stemnt=stemnt->next) {
      add(stemnt->dup());
   }

   for (j=fDef.labels.begin(); j != fDef.labels.end(); j++) {
      addLabel((*j)->dup());
   }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTScatterDef::~BRTScatterDef()
{
   /* Nothing.  We have no addition storage beyond our ancerstor classes */
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTScatterDef::print(std::ostream& out, int) const {
   std::string name = this->decl->name->name;
   std::vector<unsigned int> extraArgs;
   FunctionType * decl = static_cast<FunctionType*> (this->decl->form);   
   Symbol * in=NULL;Symbol* ret=NULL;
   unsigned int i;

   for (int j=0;j<decl->nArgs;++j) {
      if (decl->args[j]->form->type!=TT_Stream) {
         extraArgs.push_back(j);
      } else if ((decl->args[j]->form->getQualifiers()&TQ_Reduce)!=0) {
         ret=decl->args[j]->name;
      }else {
         in = decl->args[j]->name;
      }
   }
   out << "class ";
   std::string classname=name;
   if (extraArgs.empty())
      classname="__"+classname+"_scatterclass";
   else
      classname="__"+classname+"_scatter";
   out << classname << "{ public:"<<endl;
   FunctionDef * fdef = static_cast<FunctionDef *>(this->dup());
   Brook2Cpp_ConvertKernel(fdef);
   decl = static_cast<FunctionType*> (fdef->decl->form);   
   for (i=0;i<extraArgs.size();++i) {
      BRTCPUKernelCode::PrintCPUArg(decl->args[extraArgs[i]],
                                    extraArgs[i],
                                    false,
                                    true).printCPU(out,
                                                   BRTCPUKernelCode::
                                                   PrintCPUArg::HEADER);
      out << ";"<<endl;
   }
   if (extraArgs.size()) {
      indent(out,1);
      out << classname << "(";
      for (i=0;i<extraArgs.size();++i) {
         if (i!=0)
            out << ", ";
         BRTCPUKernelCode::PrintCPUArg(decl->args[extraArgs[i]],
                                       extraArgs[i],
                                       false,
                                       true).printCPU(out,BRTCPUKernelCode::
                                                      PrintCPUArg::HEADER);

      }
      out << "): ";
      for (i=0;i<extraArgs.size();++i) {
         if (i!=0)
            out << ", ";
         std::string s=decl->args[extraArgs[i]]->name->name;
         out << s<<"("<<s<<")";
      }
      out << "{}"<<endl;
   }
   if (in&&ret){
     indent(out,1);
     out << "template <class T> void operator () (T &"<< ret->name<<", "<<endl;
     indent(out,1);
     out << "                                     const T& "<<in->name<<") const";
     fdef->Block::print(out,1);
   }
   out << "}";
   if (extraArgs.empty())
      out << "__"+name+"_scatter";
   out << ";"<<endl;

   delete fdef;
}
   
   





