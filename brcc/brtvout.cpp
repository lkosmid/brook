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
   VoutCounter->form = new BaseType (BT_Float2);
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

Statement * InitialInfSet (std::string fname,                                 
                           FunctionType * ft,
                           Decl * vout_counter,
                           const Location & location) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fname);
   if (func==voutFunctions.end())
      return NULL;
   std::set<unsigned int>::iterator iter =func->second.begin();
   ExpressionStemnt * expr;
   if (iter!=func->second.end()) {
      Symbol * Why = new Symbol;Why->name = "y";
      Symbol * vout_sym=  ft->args[*iter]->name->dup();     
      expr = new ExpressionStemnt
         (new AssignExpr (AO_Equal,
                          new Variable(vout_sym,location),
                          new BinaryExpr(BO_Member,
                                         new Variable(vout_counter->name
                                                      ->dup(),
                                                      location),
                                         new Variable (Why,location),
                                         location),
                          location),
          location);
      ++iter;
   }else return NULL;
   for (;iter!=func->second.end();++iter) {
   Symbol * vout_sym=  ft->args[*iter]->name->dup();
   Symbol * Why = new Symbol;Why->name = "y";
   expr->expression 
      = new BinaryExpr(BO_Comma,
                       expr->expression,
                       new AssignExpr (AO_Equal,
                                       new Variable(vout_sym,location),
                                       new BinaryExpr(BO_Member,
                                                      new Variable
                                                      (vout_counter->name
                                                       ->dup(),
                                                       location),
                                                      new Variable(Why,
                                                                   location),
                                                      location),
                                       location),  
                       location);

   }
   return expr;
}
Decl * findVoutCounter (FunctionType * ft) {
   for (int i=0;i<ft->nArgs;++i) {
      if (ft->args[i]->name->name=="__vout_counter")
         return ft->args[i];
   }
   return ft->args[ft->nArgs-1];
}

static FunctionType * pushFunctionType=NULL;
static Statement * PushToIfStatement(Statement * ste) {
   Statement * newstemnt=NULL;   
   FunctionType * ft = pushFunctionType;
   Decl * vout_counter = findVoutCounter(ft);
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
               for (unsigned int i=0;i<(unsigned int)ft->nArgs;++i) {
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
               Symbol * Eks = new Symbol; Eks->name="x";
               Symbol * counter=vout_counter->name->dup();
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
               
               newstemnt=new IfStemnt
                  (new RelExpr(RO_Equal,
                               new FloatConstant(-1,fc->location),
                               new AssignExpr(AO_MinusEql,
                                              new BinaryExpr
                                              (BO_Member,
                                               new Variable (counter,
                                                             fc->location),
                                               new Variable (Eks,
                                                             fc->location),
                                               fc->location),
                                              new IntConstant(1,fc->location),
                                              fc->location),
                               fc->location),
                   AssignStream,
                   fc->location);
               
            }
         }
      }
      return newstemnt;
}
FunctionDef* TransformVoutPush(FunctionDef*fd) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fd->FunctionName()->name);
   if (func==voutFunctions.end())
      return NULL;
   FunctionType * ft = static_cast<FunctionType*>(fd->decl->form);
   Decl * vout_counter = findVoutCounter(ft);
   pushFunctionType = ft;
   TransformStemnt(fd,&PushToIfStatement);
   Block * MainFunction= new Block(fd->location);
   Statement * tmp=MainFunction->head;
   MainFunction->head=fd->head;
   fd->head=tmp;
   tmp =MainFunction->tail;
   MainFunction->tail=fd->tail;
   fd->tail=tmp;
   fd->add(InitialInfSet(fd->FunctionName()->name,
                         ft,
                         vout_counter,
                         fd->location));
   fd->add(MainFunction);
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




