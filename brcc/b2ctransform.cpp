#include <fstream>
#include <string>
#include "ctool.h"
#include "main.h"
#include <iostream>
#if 0
static FunctionDef * ConvertToMask(FunctionDef *s) {
	if (!s->decl->isKernel()) return NULL;
	printf ("mask kernel found");	
	return NULL;
}
static FunctionDef * ConvertToSwizzle(FunctionDef *s) {
	if (!s->decl->isKernel()) return NULL;
	printf ("swiz kernel found");
	return NULL;	
}
#endif
void SwizzleConverter (Expression * e) {
	AssignExpr * ae;
	if (e->etype == ET_BinaryExpr&&(ae=dynamic_cast<AssignExpr *> (e))) {
		
	}
}
void ConvertToSwizzle(Statement* s) {
	s->findExpr(SwizzleConverter);
}

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
int translateMask (char mask) {
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
		out << ".mask(";
		if (rValue()->precedence()<2/*comma*/)
			out << "(" <<*rValue() <<")";
		else
			out << *rValue();
		for (unsigned int i=0;i<4&&i<mask.length();++i) {
			out << ",";
			out << translateMask(mask[i]);
		}
		out << ")";
	}

};


Expression * MaskConverter (Expression * e) {
	AssignExpr * ae;
	BinaryExpr * ret =NULL;
	Variable * vmask=NULL;
	if (e->etype == ET_BinaryExpr&&(ae= dynamic_cast<AssignExpr *> (e))) {
		//now lets identify what expression is to the left... if it's a dot then we go!
		BinaryExpr * lval;
		if (e->etype==ET_BinaryExpr&& (lval = dynamic_cast<BinaryExpr*>(ae->lValue()))) {
			if (lval->op()==BO_Member) {
				if (lval->rightExpr()->etype==ET_Variable&&(vmask = dynamic_cast<Variable*>(lval->rightExpr()))) {
					if (looksLikeMask(vmask->name->name)) {
						printf ("mask detected %s\n",vmask->name->name.c_str());
						BinaryOp bo =TranslatePlusGets (ae->op());
						if (bo!=BO_Assign) {
							//now we	 		need to move the lval to the right
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
	
}
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
};
void ConvertToMask (Expression * expression) {
	Expression * e = MaskConverter (expression);
	if (e) {
//		(*expression)=ChainExpression(e);
		Expression * k = new ChainExpression(e);
		memcpy (expression,k,sizeof(Expression));
		e->findExpr(ConvertToMask);
		
	}
}
void FindMask (Statement * s) {
	s->findExpr (ConvertToMask);
}
bool compileCpp() {
	Project proj;
	TransUnit * tu = proj.parse(globals.sourcename,false,NULL,false,NULL,NULL,NULL);
	if (tu) {
		std::ofstream out;
//		tu->findFunctionDef (ConvertToMask);
		tu->findStemnt(FindMask);
		tu->findStemnt(FindMask);				
		tu->findStemnt (ConvertToSwizzle);
		std::string s (globals.coutputname);
		s+="pp";
		out.open(s.c_str());
		if (out.fail()) {
			std::cerr << "***Unable to open " << s << "\n";
		}
		out << *tu << std::endl;
		out.close();
		
	}else {
      std::cerr << "***Unable to parse " << globals.sourcename << std::endl;
	  return false;
	}
	return true;
}
