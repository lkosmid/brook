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
               Variable * strm=NULL;
               if (fc->args[0]->etype==ET_Variable) {
                  strm = static_cast<Variable*>(fc->args[0]);
               }else if (fc->args[2]->etype==ET_Variable) {
                  strm = static_cast<Variable*>(fc->args[2]);
               }else {
                  std::cerr << "Error:";
                  fc->location.printLocation(std::cerr);
                  std::cerr << "Function args must be stream variables";
                  assert (0);
               }
               char symbolAppend[2]={'1',0};
               if (strm->name->entry) {
                  Decl * uVarDecl;
                  if ((uVarDecl=strm->name->entry->uVarDecl)!=0) {
                     BaseTypeSpec typ=uVarDecl->form->getBase()->typemask;
                     if (typ&BT_Float4) {
                        symbolAppend[0]='4';
                     }else if (typ&BT_Float3) {
                        symbolAppend[0]='3';
                     }else if (typ&BT_Float2) {
                        symbolAppend[0]='2';
                     }
                  }
               }
               name->name= new Symbol(*name->name);
               name->name->name+=symbolAppend;
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
   Decl * in=NULL;Decl* ret=NULL;
   unsigned int inindex=0,retindex=0;
   unsigned int i;

   for (int j=0;j<decl->nArgs;++j) {
      if (decl->args[j]->form->type!=TT_Stream) {
         extraArgs.push_back(j);
      } else if ((decl->args[j]->form->getQualifiers()&TQ_Reduce)!=0) {
        ret=decl->args[j];
        retindex=j;
      }else {
        in = decl->args[j];
        inindex=j;
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
      out << "{}"<<std::endl;
   }
   if (in&&ret){
     indent(out,1);
     out << "void operator () (";
     BRTCPUKernelCode::PrintCPUArg(decl->args[retindex],
                                   retindex,
                                   false,
                                   true).printCPU(out,BRTCPUKernelCode::
                                                  PrintCPUArg::HEADER);
     out <<", "<<endl;
     indent(out,1);
     out << "                  ";
     //     if ((in->form->getQualifiers()&TQ_Const)==0)
     //       out << "const ";     
     BRTCPUKernelCode::PrintCPUArg(decl->args[inindex],
                                   inindex,
                                   false,
                                   true).printCPU(out,BRTCPUKernelCode::
                                                  PrintCPUArg::HEADER);
     out <<") const {";
     //fdef->Block::print(out,1);
     out << std::endl;
     indent(out,2);
     out << "__"<<name<<"_cpu_inner (";
     for (int i=0;i<decl->nArgs;++i) {
       if (i!=0) 
         out << ", ";
       out << decl->args[i]->name->name;
     }
     out << ");"<<std::endl;
     indent(out,1);
     out << "}"<<endl;
   }
   out << "}";
   if (extraArgs.empty())
      out << "__"+name+"_scatter";
   out << ";"<<endl;

   delete fdef;
}
   
   





