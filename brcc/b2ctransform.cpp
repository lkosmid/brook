#ifdef _WIN32
#pragma warning(disable:4786)
//the above warning disables visual studio's annoying habit of warning when using the standard set lib
#endif
#include <set>
#include <fstream>
#include <string>
#include <iostream>

#include "ctool.h"
#include "brtdecl.h"
#include "brtstemnt.h"
#include "main.h"
#include "brook2cpp.h"

template <class ConverterFunctor> void ConvertToT (Expression * expression);

BinaryOp TranslatePlusGets (AssignOp ae) {
                BinaryOp bo=BO_Assign;
                switch (ae) {
                case AO_Equal:
                    break;
                case AO_PlusEql:
                    bo=BO_Plus;
                    break;
                case AO_MinusEql:
                    bo= BO_Minus;
                    break;
                case AO_DivEql:
                    bo=BO_Div;
                    break;
                case AO_ModEql:
                    bo=BO_Mod;
                    break;
                case AO_ShlEql:
                    bo = BO_Shl;
                    break;
                case AO_ShrEql:
                    bo = BO_Shr;
                    break;
                case AO_BitAndEql:
                    bo = BO_BitAnd;
                    break;
                case AO_BitXorEql:
                    bo = BO_BitXor;
                    break;
                case AO_BitOrEql:
                    bo = BO_BitOr;
                    break;
                }
                return bo;
}
bool looksLikeMask (std::string s) {
    if (s.length()<=4) {
        for (unsigned int i=0;i<s.length();++i) {
            if (s[i]!='x'&&s[i]!='y'&&s[i]!='z'&&s[i]!='w'&&
                s[i]!='r'&&s[i]!='g'&&s[i]!='b'&&s[i]!='a'){
                return false;
            }
        }
        return true;
    }
    return false;
}
int translateSwizzle (char mask) {
    switch (mask) {
    case 'y':
    case 'g':
        return 1;
    case 'z':
    case 'b':
        return 2;
    case 'w':
    case 'a':
        return 3;
    default:
        return 0;
    }
}
std::string translateMask (int swizzle) {
    switch (swizzle) {
    case 3:
        return "maskW";
    case 2:
        return "maskZ";
    case 1:
        return "maskY";
    default:
        return "maskX";
    }
}
class MaskExpr : public BinaryExpr {
  public:
    std::string mask;
    MaskExpr( Expression *lExpr, Expression *rExpr, std::string mask, const Location& l ):
        BinaryExpr(BO_Member,lExpr,rExpr,l) {
        this->mask=mask;
    }
    ~MaskExpr(){}
    
    Expression *lValue() const { return leftExpr(); }
    Expression *rValue() const { return rightExpr(); }

    int precedence() const { return 16;/*maybe left expr's prec*/ }

    Expression *dup0()  const {
        MaskExpr *ret = new MaskExpr(_leftExpr->dup(),_rightExpr->dup(),mask, location);
        ret->type = type;  
        return ret;
    }
    
    void print(std::ostream& out) const{
        if (lValue()->precedence()<precedence())
            out << "(" << *lValue() << ")";
        else
            out << *lValue();
        out << ".mask"<< mask.length()<<"(";
        int commaprecedence = BinaryExpr(BO_Comma,NULL,NULL,location).precedence();        
        if (rValue()->precedence()<commaprecedence/*comma*/)
            out << "(" <<*rValue() <<")";
        else
            out << *rValue();
        for (unsigned int i=0;i<4&&i<mask.length();++i) {
            out << ",";
            out << translateMask(translateSwizzle(mask[i]));
        }
        out << ")";
    }

};
class NewQuestionColon :public TrinaryExpr {public:
    NewQuestionColon (Expression * c, Expression * t, Expression * f, const Location &l):TrinaryExpr(c,t,f,l) {}
    Expression * dup0() const {return new NewQuestionColon(condExpr()->dup(),trueExpr()->dup(),falseExpr()->dup(),location);}
    int precedence() const {return 3;}
    void print(std::ostream &out) const {
        int memberprecedence = BinaryExpr(BO_Member,NULL,NULL,location).precedence();
        if (condExpr()->precedence()<memberprecedence)/*member*/ 
            out << "(";
        out<<*condExpr();
        if (condExpr()->precedence()<memberprecedence) /*member*/
            out << ")";        
        out<<".questioncolon(";
        out<<*trueExpr();
        out << ",";
        out << *falseExpr();
        out << ")";
    }
        
};
class QuestionColonConverter{public:
    Expression * operator()(Expression * e) {
        TrinaryExpr * te;
        NewQuestionColon * ret=NULL;
        if (e->etype==ET_TrinaryExpr&& (te = static_cast<TrinaryExpr*>(e))) {
            ret = new NewQuestionColon(te->condExpr()->dup(),
                                       te->trueExpr()->dup(),
                                       te->falseExpr()->dup(),
                                       te->location);
        }
        return ret;
    }
};
std::set<Expression *> ArrayBlacklist;//set of items we do not want to change to int1 objects.

class BaseType1:public BaseType {public:
    BaseType1(const BaseType &t):BaseType(t){
	ArrayBlacklist.insert((Expression *)this);
    }
    BaseType1::~BaseType1() {
	std::set<Expression *>::iterator i = ArrayBlacklist.find((Expression *)this);
	if (i!=ArrayBlacklist.end())
	    ArrayBlacklist.erase(i);
    }
	virtual void printBase(std::ostream& out, int level) const {
		//this->printQual(out,qualifier);
	int special = BT_Char|BT_Int|BT_Float|BT_Float2|BT_Float3|BT_Float4|BT_Long;
	if ((typemask&special)!=0){
		if (qualifier &TQ_Const)
			out << "const ";
		if (qualifier &TQ_Volatile)
			out << "volatile ";
		
		if (typemask & BT_Char)
			out << "bool1 ";
		else if (typemask & BT_Float)
			out << "float1 ";
		else if (typemask & BT_Float2)
			out << "float2 ";
		else if (typemask & BT_Float3)
			out << "float3 ";
		else if (typemask & BT_Float4)
			out << "float4 ";
		else
			out << "int1 ";
		if (qualifier &TQ_Out)
			out << "&";
		
	}else {
		BaseType::printBase(out,level);
	}
	}
};



void ArrayBlackmailer(Expression * e) {
    ArrayBlacklist.insert(e);
}
void BlackmailType(Type **t);
template <class T> void BlackmailT(Type **&t) {
    T * k;
    k = static_cast<T *>(*t);
    if (k) {
        BlackmailType(&k->subType);
    }
}

void BlackmailBaseType (BaseType **t) {
	*t = new BaseType1(**t);
}
void BlackmailType (Type **t) {
	if (!t)
		return;
    if (!(*t))
        return;
    BaseType * basei ;
    
    if ((*t)->type==TT_Base&&(basei= static_cast<BaseType *>(*t))) {
		*t = new BaseType1(*basei);
		//delete base;
    }
    ArrayType *at ;
    if ((*t)->type==TT_Array&&(at = static_cast<ArrayType *>(*t))) {
        at->size->findExpr(ArrayBlackmailer);
        ArrayBlackmailer(at->size);
    }
	if ((*t)->type==TT_Array||(*t)->type==TT_Stream)
	    BlackmailT<ArrayType>(t);//this takes care of Streams as well as constant arrays    
	if ((*t)->type==TT_Pointer)
	    BlackmailT<PtrType>(t);
	if ((*t)->type==TT_BitField)
	    BlackmailT<BitFieldType>(t);
    BrtStreamType * st;

    if ((*t)->type==TT_BrtStream && (st = static_cast<BrtStreamType *>(*t))) {
//        st->dims->findExpr(ArrayBlackmailer); //these must not be constant any more
//        ArrayBlackmailer(st->dims);        //these must not be constant any more
        BlackmailBaseType(&st->base);
        
    }
    FunctionType * ft;
    if ((*t)->type==TT_Function&&(ft = static_cast<FunctionType *>(*t))) {
		for (int i=0;i<ft->nArgs;++i) {
			BlackmailType(&ft->args[i]->form);
		}
		BlackmailType(&ft->subType);
    }
    
    
}

void FindTypesDecl (Statement * s) {
    DeclStemnt * ds;

    if (s->isDeclaration()&&(ds=static_cast<DeclStemnt*>(s))) {
        for (unsigned int i=0;i<ds->decls.size();++i) {
            BlackmailType(&ds->decls[i]->form);
        }
//        ds->print(std::cout,0);        
    }
    FunctionDef * fd;
    if (s->isFuncDef() && (fd = static_cast<FunctionDef *>(s))) {
        BlackmailType(&fd->decl->form);
      }
    
}

class NewIndexExpr :public IndexExpr {public:
    NewIndexExpr (Expression * a, Expression * s,const Location &l):IndexExpr(a,s,l) {} //Not *the* tr1nity with a/s/l check onjoin.
    Expression * dup0() const {return new NewIndexExpr(array->dup(),_subscript->dup(),location);}
    void print(std::ostream &out) const {
    if (array->precedence() < precedence())
    {
        out << "(";
        array->print(out);
        out << ")";
    }
    else
        array->print(out);

    out << "[(int)";
    int castprecedence= CastExpr(NULL,NULL,location).precedence();
    if (_subscript->precedence()<=castprecedence)
        out <<"(";
    _subscript->print(out);
    if (_subscript->precedence()<=castprecedence)
        out <<")";    
    out << "]";

    }
        
};
void ConvertToTIndexExprConverter(Expression * e);
class IndexExprConverter{public:
    Expression * operator()(Expression * e) {
        IndexExpr *ie;
        IndexExpr * ret=NULL;
        if (e->etype==ET_IndexExpr&&(ie=static_cast<IndexExpr*>(e))) {
            ret = new NewIndexExpr (ie->array->dup(),ie->_subscript->dup(),e->location);
            ret->array->findExpr(&ConvertToTIndexExprConverter);
            ret->_subscript->findExpr(&ConvertToTIndexExprConverter);            
        }
        CastExpr * ce;
        if (e->etype==ET_CastExpr&&(ce=static_cast<CastExpr*>(e))) {
            BlackmailType(&ce->castTo);
        }
        return ret;
    }
};
class NewIntConstant:public IntConstant {public:
    NewIntConstant(long val, const Location &l):IntConstant(val,l){}
    virtual void print (std::ostream&out) const{
        out << "int1(";
        (this)->IntConstant::print(out);
        out << ")";
    }
};
class NewUIntConstant:public UIntConstant {public:
    NewUIntConstant(unsigned int val, const Location &l):UIntConstant(val,l){}
    virtual void print (std::ostream&out) const{
        out << "int1(";
        (this)->UIntConstant::print(out);
        out << ")";
    }
};
class NewCharConstant:public CharConstant {public:
    NewCharConstant(char val, const Location &l):CharConstant(val,l){}
    virtual void print (std::ostream&out) const{
        out << "bool1(";
        (this)->CharConstant::print(out);
        out << ")";
    }
};
class NewFloatConstant:public FloatConstant {public:
    NewFloatConstant(double val, const Location &l):FloatConstant(val,l){}
    virtual void print (std::ostream&out) const{
        out << "float1(";
        (this)->FloatConstant::print(out);
        out << ")";
    }
};
class NewArrayConstant:public ArrayConstant {public:
    NewArrayConstant(const Location &l):ArrayConstant(l){}
    virtual void print (std::ostream&out) const{
        out << "float"<<items.size()<<"(";
        for (unsigned int i=0;i<items.size();++i) {
            items[i]->print(out);
            if (i!=items.size()-1) {
                out <<", ";
            }
        }
        out << ")";
    }
};
void ConvertToTConstantExprConverter (Expression *);
class ConstantExprConverter{public:
    Expression * operator()(Expression * e) {
        Constant *con;
        Constant * ret=NULL;
        if (e->etype==ET_Constant&&(con=static_cast<Constant*>(e))) {
            switch (con->ctype) {
            case CT_Char:
            {
                CharConstant * cc = static_cast<CharConstant *>(con);
                ret = new NewCharConstant(cc->ch,cc->location);
                break;
            }
            case CT_Int:
            {
                IntConstant * cc = static_cast<IntConstant *>(con);
                ret = new NewIntConstant(cc->lng,cc->location);                
                break;
            }
            case CT_UInt:
            {
                UIntConstant * cc = static_cast<UIntConstant *>(con);
                ret = new NewUIntConstant(cc->ulng,cc->location);                                
                break;
            }
            case CT_Float:
            {
                FloatConstant * cc = static_cast<FloatConstant *>(con);
                ret = new NewFloatConstant(cc->doub,cc->location);                                                
                break;
            }
            case CT_Array:
            {
                ArrayConstant * ac = static_cast<ArrayConstant *>(con);
                ArrayConstant *aret=  new NewArrayConstant (ac->location);
                ret=aret;
                for (unsigned int i=0;i<ac->items.size();++i){
                    Expression * expr=ac->items[i]->dup();
                    expr->findExpr(ConvertToTConstantExprConverter);
                    aret->addElement(expr);                    
                }
                break;
            }
            }
        }
        return ret;
    }
};


class SwizzleConverter{public:
    Expression * operator()(Expression * e) {
        BinaryExpr * be;
        if (e->etype==ET_BinaryExpr&& (be = static_cast<BinaryExpr*>(e))) {
            if (be->op()==BO_Member) {
                Variable * vswiz;
                if (be->rightExpr()->etype==ET_Variable&&(vswiz = static_cast<Variable*>(be->rightExpr()))) {
                    if (looksLikeMask(vswiz->name->name)) {
//                        printf ("swizzlefound %s",vswiz->name->name.c_str());
                        unsigned int len =vswiz->name->name.length();
                        char swizlength [2]={len+'0',0};
                        std::string rez=std::string("swizzle")+swizlength+"<";
                        for (unsigned int i=0;i<len;++i) {
                            char swizchar [3] =  {'0'+translateSwizzle(vswiz->name->name[i]),i==len-1?'\0':',','\0'};
                            rez+=swizchar;
                        }
                        rez+=">()";
                        vswiz->name->name=rez;
                    }
                }
            }
        }
        return NULL;
    }
};
class MaskConverter{public:
Expression * operator () (Expression * e) {
    AssignExpr * ae;
    BinaryExpr * ret =NULL;
    Variable * vmask=NULL;
    if (e->etype == ET_BinaryExpr&&(ae= static_cast<AssignExpr *> (e))) {
        //now lets identify what expression is to the left... if it's a dot then we go!
        BinaryExpr * lval;
        if (ae->lValue()->etype==ET_BinaryExpr&& (lval = static_cast<BinaryExpr*>(ae->lValue()))) {
            if (lval->op()==BO_Member) {
                if (lval->rightExpr()->etype==ET_Variable&&(vmask = static_cast<Variable*>(lval->rightExpr()))) {
                    if (looksLikeMask(vmask->name->name)) {
                        //printf ("mask detected %s\n",vmask->name->name.c_str());
                        BinaryOp bo =TranslatePlusGets (ae->op());
                        if (bo!=BO_Assign) {
                            //now we             need to move the lval to the right
                            ae->_rightExpr = new BinaryExpr(bo,ae->lValue()->dup(),ae->_rightExpr,ae->location);
                            ae->aOp=AO_Equal;
                        }
                        ret = new MaskExpr (lval->leftExpr()->dup(),ae->rValue()->dup(),vmask->name->name,lval->location);
                    }
                }
            }
        }
    }
    return ret;
    
}};
class ChainExpression:public Expression {
public:
    ChainExpression (Expression * e):Expression(e->etype,e->location) {
        next = e;
    }
    virtual void print (std::ostream & out)const{
        next->print(out);
    }
    virtual Expression * dup0() const {
        return new ChainExpression(next);
    }
    virtual void findExpr( fnExprCallback cb ) { next->findExpr(cb); }    
};
void COnvertToTMaskConverter (Expression * e);
template <class ConverterFunctor> void ConvertToT (Expression * expression) {
    if (ArrayBlacklist.find(expression)==ArrayBlacklist.end()) {
        Expression * e = ConverterFunctor()(expression);
        if (e) {
//        (*expression)=ChainExpression(e);
            Expression * k = new ChainExpression(e);//memory leak--but how else are we to guarantee the integrity of our expression..it's a lost cause unless we have some way of assinging to the passed in expression without rewriting the whole loop.
            char location[sizeof(Location)];
            memcpy (&location[0],&expression->location,sizeof(Location));
            memcpy (expression,k,sizeof(Expression));//DANGEROUS but we don't have access to the code
            memcpy (&expression->location,&location[0],sizeof(Location));
            e->findExpr(ConvertToTMaskConverter);
            
        }
    }
}

void ConvertToTMaskConverter (Expression * e) {
	ConvertToT<MaskConverter>(e);
}
void FindMask (Statement * s) {
    s->findExpr(&ConvertToTMaskConverter);
}
void ConvertToTSwizzleConverter (Expression * e) {
	ConvertToT<SwizzleConverter>(e);//this function is created because VC++ can't take the address of a template function and "know" its type...blame ole billyG
}
void FindSwizzle (Statement * s) {
    s->findExpr((fnExprCallback)&ConvertToTSwizzleConverter);
}
void ConvertToTQuestionColonConverter (Expression * e) {
	ConvertToT<QuestionColonConverter>(e);//this function is created because VC++ can't take the address of a template function and "know" its type...blame ole billyG
}
void FindQuestionColon (Statement * s) {
    s->findExpr((fnExprCallback)&ConvertToTQuestionColonConverter);
}
void ConvertToTIndexExprConverter (Expression * e) {
	ConvertToT<IndexExprConverter>(e);//this function is created because VC++ can't take the address of a template function and "know" its type...blame ole billyG
}
void FindIndexExpr (Statement * s) {
    s->findExpr((fnExprCallback)&ConvertToTIndexExprConverter);
}
void ConvertToTConstantExprConverter (Expression * e) {
	ConvertToT<ConstantExprConverter>(e);//this function is created because VC++ can't take the address of a template function and "know" its type...blame ole billyG
}
void FindConstantExpr (Statement * s) {
	void (*tmp)(class Expression *) = &ConvertToTConstantExprConverter;
    s->findExpr(tmp);
}
void RestoreTypes(BRTKernelDef *kDef){
	ArrayBlacklist.clear();
	kDef->findStemnt (FindTypesDecl);
}

void Brook2Cpp_ConvertKernel(BRTKernelDef *kDef) {
    kDef->findStemnt(&FindMask);
    RestoreTypes(kDef);
    kDef->findStemnt (&FindSwizzle);
    RestoreTypes(kDef);
    kDef->findStemnt (&FindQuestionColon);
    RestoreTypes(kDef);
    kDef->findStemnt (&FindIndexExpr);
    RestoreTypes(kDef);
    kDef->findStemnt (&FindConstantExpr);
    RestoreTypes(kDef);
}
