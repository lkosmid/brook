#include "brtvout.h"

#include "ctool.h"

VoutFunctionType voutFunctions;

FunctionDef *IdentifyVoutFunc(FunctionDef * fd){
   FunctionType * ft = static_cast<FunctionType* >(fd->decl->form);
   bool foundvout=false;
   std::set<unsigned int> vouts;
   for (int i=0;i<ft->nArgs;++i) {
      if ((ft->args[i]->form->getQualifiers()&TQ_Vout)!=0) {
         foundvout=true;
         vouts.insert(i);
      }
   }
   if (foundvout)
      voutFunctions.insert(VoutFunctionType::
                           value_type(fd->FunctionName()->name,vouts));
   
   return NULL;
}
FunctionDef * TransformHeader (FunctionDef * fd) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fd->FunctionName()->name);
   if (func==voutFunctions.end())
      return NULL;
   FunctionType * ft = static_cast<FunctionType*>(fd->decl->form);
   std::set<unsigned int>::iterator iter =func->second.begin();
   
   for (;iter!=func->second.end();++iter) {
      DeclStemnt * ds = new DeclStemnt (fd->location);
      Decl * tmpVout =ft->args[*iter];
      ft->args[*iter]=ft->args[*iter]->dup();
      ft->args[*iter]->name->name=
         "__"+ft->args[*iter]->name->name+"_stream";
      if (tmpVout->form->type==TT_Stream) {
         tmpVout->form = static_cast<ArrayType*>(tmpVout->form)->subType;
      }
      tmpVout->next=NULL;
      ds->addDecl(tmpVout);
      ds->next = fd->head;
      fd->head = ds;
   }
   Symbol * voutCounter = new Symbol;
   voutCounter->name = "__vout_counter";
   Decl * VoutCounter =  new Decl (voutCounter);
   VoutCounter->form = new BaseType (BT_Float);
   ft->addArg(VoutCounter);
   return NULL;
}
FunctionDef * TransformVoutToOut (FunctionDef * fd) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fd->FunctionName()->name);
   if (func==voutFunctions.end())
      return NULL;
   FunctionType * ft = static_cast<FunctionType*>(fd->decl->form);
   std::set<unsigned int>::iterator iter =func->second.begin();
   
   for (;iter!=func->second.end();++iter) {
      BaseType * bat = ft->args[*iter]->form->getBase();

      
      bat->qualifier&=(~TQ_Vout);
      bat->qualifier|=TQ_Out;
   }   
   return NULL;
}
FunctionDef* TransformVoutPush(FunctionDef*fd) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fd->FunctionName()->name);
   if (func==voutFunctions.end())
      return NULL;
   FunctionType * ft = static_cast<FunctionType*>(fd->decl->form);
   Decl * vout_counter = ft->args[ft->nArgs-1];
   
   Statement * ste=fd->head;
   Statement** lastSte=&fd->head;
   for (;ste;ste=ste->next) {
      if (ste->type==ST_ExpressionStemnt) {
         ExpressionStemnt * es = static_cast<ExpressionStemnt*>(ste);
         if (es->expression->etype==ET_FunctionCall) {
            FunctionCall* fc=static_cast<FunctionCall*>(es->expression);
            if (fc->args.size()==1
                &&fc->function->etype==ET_Variable
                &&static_cast<Variable*>(fc->function)->name->name=="push") {
               if (fc->args[0]->etype!=ET_Variable) {
                  std::cerr<<"Error: ";
                  fc->args[0]->location.printLocation(std::cerr);
                  std::cerr<< " Push called without specific vout stream.\n";
                  return NULL;
               }
               std::string voutname=static_cast<Variable*>
                  (fc->args[0])->name->name;
               Decl * streamDecl=NULL;
               for (unsigned int i=0;i<ft->nArgs;++i) {
                  if (ft->args[i]->name->name
                      ==std::string("__")+voutname+"_stream") {
                     streamDecl=ft->args[i];
                     break;
                  }
               }
               if (streamDecl==NULL) {
                  std::cerr<<"Error: ";
                  fc->args[0]->location.printLocation(std::cerr);
                  std::cerr<<" Push called on var that is not a vout arg.\n";
                  return NULL;
               }
               Symbol * counter=new Symbol;
               counter->name=vout_counter->name->name;
               counter->entry=mk_paramdecl(counter->name,vout_counter);
               Symbol * stream=new Symbol;
               stream->name="__"+voutname+"_stream";
               stream->entry=mk_paramdecl(stream->name,streamDecl);
               Block * AssignStream = new Block(fc->location);
               AssignStream->add
                  (new ExpressionStemnt
                   (new AssignExpr(AO_Equal,
                                   new Variable(stream,fc->location),
                                   fc->args[0]->dup(),
                                   fc->location),
                    fc->location));
               
               IfStemnt* ifs=new IfStemnt
                  (new RelExpr(RO_Equal,
                               new FloatConstant(-1,fc->location),
                               new UnaryExpr(UO_PreDec,
                                             new Variable(counter,
                                                          fc->location),
                                             fc->location),
                               fc->location),
                   AssignStream,
                   fc->location);
               
               if (ste==fd->tail) {
                  fd->tail=ifs;
               }
               *lastSte=ifs;
               ifs->next= ste->next;
               //delete ste;
            }
         }
      }
      lastSte=&ste->next;
   }
   return NULL;
}
void transform_vout (TransUnit * tu) {
   tu->findFunctionDef (IdentifyVoutFunc);
   tu->findFunctionDef (TransformHeader);
   tu->findFunctionDef(TransformVoutToOut);
   tu->findFunctionDef(TransformVoutPush);
   //transform push calls
   //transform function calls
   

}




