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
#include "brtscatter.h"
#include "brtdecl.h"
#include "brtexpress.h"
#include "project.h"
using std::endl;
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
      } else if ((decl->args[j]->form->getQualifiers()&TQ_Out)!=0) {
         ret=decl->args[j]->name;
      }else {
         in = decl->args[j]->name;
      }
   }
   out << "class ";
   if (extraArgs.empty())
      out << "__"; 
   out <<name << "{ public:"<<endl;
   FunctionDef * fdef = static_cast<FunctionDef *>(this->dup());
   Brook2Cpp_ConvertKernel(fdef);
   decl = static_cast<FunctionType*> (fdef->decl->form);   
   for (i=0;i<extraArgs.size();++i) {
      decl->args[extraArgs[i]]->print(out,1,1);
      out << ";"<<endl;
   }
   if (extraArgs.size()) {
      indent(out,1);
      out << name << "(";
      for (i=0;i<extraArgs.size();++i) {
         decl->args[extraArgs[i]]->print(out,1);
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
      out << name;
   out << ";"<<endl;

   delete fdef;
}
   
   





