#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "algebra.h"

bool checkIdentity() {
  return false;
}

//valueVarMap is an array that will has enough space to store
//a boolean for every letter in the alphabet, which is what we
//restrict the variable names to. Will definitely need to change
//If We want to have more variables or more complex names
bool eval(Expression* expr, bool* valueVarMap) {
	if(expr->exprType == TYPE_VAR) {
		//last thing we evaluate
		//most primitive type, can't go any further
		char name = expr->exprValue.var.name;
		return valueVarMap[name - 'A'];
	}
	else if(expr->exprType == TYPE_EXPR) {
		Actions act = expr->exprValue.expr.action;
		switch(act) {
		case ACT_NOT:
			return !eval(expr->exprValue.expr.operands, valueVarMap);
		case ACT_XOR:
		case ACT_OR:
		case ACT_AND: {
			bool value = false;
			for(unsigned int i = 0; i < expr->exprValue.expr.operandCount; i++) {
				if(i == 0) {
					value = eval(expr->exprValue.expr.operands+i, valueVarMap);
				} else if(act == ACT_AND){
					value = value &&  eval(expr->exprValue.expr.operands+i, valueVarMap);
				} else if(act == ACT_XOR){
					bool nv =  eval(expr->exprValue.expr.operands+i, valueVarMap);
					value = (value && !nv) || (!value && nv);
				} else {
					value = value || eval(expr->exprValue.expr.operands+i, valueVarMap);
				}
			}
			return value;
		}
		case ACT_INVALID:
		default:
			printf("Invalid Operation\n");
			exit(EXIT_FAILURE);
		}
	}

	printf("Expression type is invalid\n");
	exit(EXIT_FAILURE);
}
