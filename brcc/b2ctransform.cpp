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


Expression * MaskConverter (Expression * e) {
	AssignExpr * ae;
	BinaryExpr * ret =NULL;
	if (e->etype == ET_BinaryExpr&&(ae= dynamic_cast<AssignExpr *> (e))) {
		//now lets identify what expression is to the left... if it's a dot then we go!
		BinaryExpr * lval;
		if (e->etype==ET_BinaryExpr&& (lval = dynamic_cast<BinaryExpr*>(ae->lValue()))) {
			if (lval->op()==BO_Member) {
				printf ("mask detected");
				BinaryOp bo =TranslatePlusGets (ae->op());
				if (bo!=BO_Assign) {
					//now we need to move the lval to the right
					ae->_rightExpr = new BinaryExpr(bo,ae->lValue()->dup0(),ae->rValue(),ae->location);
					ae->aOp=AO_Equal;
				}
				Symbol mask;mask.name="mask";
				FunctionCall * func = new FunctionCall (new Variable (new Symbol(mask),ae->location),ae->location);
				func->addArg ( ae->rValue()->dup0());
				ret = new BinaryExpr (BO_Member,lval->leftExpr ()->dup0(),func,lval->location);
				
			}
		}
	}
	return ret;
	
}
void ConvertToMask (Statement * s) {
	ExpressionStemnt * es = dynamic_cast <ExpressionStemnt*> (s);
	if (es) {
		Expression * e = MaskConverter (es->expression);
		if (e) {
			delete es->expression;
			es->expression=e;
		}
	}
}

bool compileCpp() {
	Project proj;
	TransUnit * tu = proj.parse(globals.sourcename,false,NULL,false,NULL,NULL,NULL);
	if (tu) {
		std::ofstream out;
//		tu->findFunctionDef (ConvertToMask);
		tu->findStemnt(ConvertToMask);		
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
