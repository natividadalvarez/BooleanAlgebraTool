//IDEA:
//a program in which you can input a series of boolean algebra expressions
//and it tells the user whether or not you can get from each expression to the next
//adjacent one validly in one step, and which theorem you can do so with.

//IDEA:
//multi-thread the application of the boolean laws when verifying

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parsing.h"

char opChars[] = {'+', '^', '*', '!', '(', '\0'};

void validateExpression(const char * exprStr) {
	int parenCount = 0;
	char lastChar = '\0';
	const char* comp = exprStr;
	while(*comp != '\0') {
		if(*comp == ')' && lastChar == '(') {
			printf("%s is invalid: contains empty ()\n", exprStr);
			exit(EXIT_FAILURE);
		}
		if(*comp == '(') parenCount++;
		if(*comp == ')') parenCount--;
		lastChar = *comp;
		comp++;
	}
	if(parenCount > 0)
		printf("%s is invalid: has extra (\n", exprStr);
	if(parenCount < 0)
		printf("%s is invalid: has extra )\n", exprStr);

	if(parenCount !=0)
		exit(EXIT_FAILURE);
}

//Assumes given validated expression
static bool topLevelParen(const char* exprStr, int exprLen) {
  if(exprStr[exprLen-1] != ')') return false;
  int parenCount = 1;
  for(int i = exprLen - 2; i > 0; i--) {
    if(parenCount == 0) return false; //we found its pair before the end, so it's not top level
    if(exprStr[i] == ')') parenCount++;
    if(exprStr[i] == '(') parenCount--;
  }
  return parenCount == 1 && exprStr[0] == '(';
}

//Removes all whitespace from ends of string
//Creates new string
char* trim(const char* exprStr) {
  while(*exprStr == ' ') exprStr++;
  int exprLen = strlen(exprStr);
  int firstSpace = -1;
  for(int i = exprLen - 1; i >= 0; i--) {
    if(exprStr[i] != ' ') break;
    firstSpace = i;
  }
  exprLen = firstSpace >= 0? firstSpace + 1: exprLen + 1;
  //return new string
  char* trimmed = malloc(sizeof(char) * exprLen+1);
  for(int i = 0; i < exprLen - 1; i++) {
    trimmed[i] = exprStr[i];
  }
  trimmed[exprLen - 1] = '\0';
  return trimmed;
}

//Make sure you have validated any expression string
//That you pass into here
Expr parseExpression(char* exprStr) {
  Expr expression;
  int exprLen = strlen(exprStr);
  //Parentheses are just for grouping, so we can just get
  //rid of it if it's the outermost thing
  int topLevel = topLevelParen((const char*) exprStr, exprLen);
  if(topLevel) {
    while(exprStr[0] == '(' && exprStr[0] != '\0') {
        exprStr++;
        //write the null where the current closing paren is at
        exprLen--;
        exprStr[exprLen - 1] = '\0';
        //reduce len 1 more time since we just took away the last
        //character (the closing paren)
        exprLen--;
    }
    if(exprLen == 0) {
        printf("Invalid expresson: all parenthesis\n");
        exit(EXIT_FAILURE);
    }
    expression.parenthesized = true;
  } else {
    expression.parenthesized = false;
  }
  enum Actions action = ACT_INVALID;
  int action_count = 0;
  for(int i = 0; i < exprLen; i++) {
    enum Actions current_action = ACT_INVALID;
    switch(exprStr[i]) {
    case '+':
      current_action = ACT_OR;
      break;
    case '^':
      current_action = ACT_XOR;
      break;
    case '*':
      current_action = ACT_AND;
      break;
    case '!':
      current_action = ACT_NOT;
      break;
    case '(':
        current_action = ACT_PAREN;
        break;
    }
    if(current_action == ACT_PAREN) {
      //Everything in the parenthesis now has a higher precedence
      //This should basically only be the expression we parse
      //If there is nothing else to parse

      //Skip everything in the parenthesis
      int parenCount = 1;
      while(parenCount > 0 && i < exprLen) {
        i++;
        if(exprStr[i] == '(') parenCount++;
        else if(exprStr[i] == ')') parenCount--;
      }
    }
    else if(current_action != ACT_INVALID && current_action < action) {
      action = current_action;
      action_count = 1;
    } else if(current_action != ACT_INVALID && current_action == action) {
      action_count++;
    }
  }
  if(action == ACT_AND || action == ACT_OR || action == ACT_XOR) {
    expression.action = action;
    expression.opCount = action_count + 1;
    expression.operands = malloc(sizeof(Operand) * expression.opCount);
    char operandStr[exprLen];
    int operandStrIdx = 0;
    int operandArrIdx = 0;
    for(int i = 0; i < exprLen; i++) {
      if(exprStr[i] == opChars[expression.action]) {
        operandStr[operandStrIdx] = '\0';
        operandStrIdx = 0;
        //Send it off to be evaluated
        Operand op = parseOperand(operandStr);
        expression.operands[operandArrIdx++] = op;
      } else if (exprStr[i] != ' ') {
        operandStr[operandStrIdx++] = exprStr[i];
      }
    }
    if(operandStrIdx > 0) { //catch the last operand
      operandStr[operandStrIdx] = '\0';
      operandStrIdx = 0;
      Operand op = parseOperand(operandStr);
      expression.operands[operandArrIdx++] = op;
    }
  } else {
    //This is either a NOT or a single var
    //assuming this string was already validated
    if(exprLen == 1) {
			expression.action = ACT_BUFF;
			expression.opCount = 1;
			expression.operands = malloc(sizeof(Operand)*1);
			expression.operands[0] = parseOperand(exprStr);
    } else if(exprStr[0] == '!') {
			expression.action = ACT_NOT;
			expression.opCount = 1;
			expression.operands = malloc(sizeof(Operand)*1);
			expression.operands[0] = parseOperand(++exprStr);
		}
  }
  return expression;
}


Operand parseOperand(char* opStr) {
  int opStrLen = strlen(opStr);
  Operand parentOp;
  if(opStrLen == 1) {
    //it should be a single var (base case)
    parentOp.type = OP_VAR;
    parentOp.data.var = opStr[0];
  } else {
    parentOp.type = OP_EXPR;
    //parse recursively
    parentOp.data.expr = parseExpression(opStr);
  }
  return parentOp;
}

void printExpr(Expr* expression) {
  static int calls = 0;
  calls++;
  if(calls > 1000) {
    printf("You probably have a recusion issue\n");
    exit(-1);
  }
  if(expression->opCount == 1) {
    char opC = opChars[expression->action];
      printf("%c", opC);
      Operand* op = expression->operands;
    if(op->type == OP_EXPR) {
      printExpr(&(op->data.expr));
    } else {
      printf("%c", op->data.var);
    }
  }
  else {
    for(int i = 0; i < expression->opCount; i++) {
      if(i == 0 && expression->parenthesized)
        printf("(");
      char opC = opChars[expression->action];
      Operand* op = expression->operands + i;
      if(op->type == OP_EXPR) {
        printExpr(&(op->data.expr));
      } else {
        printf("%c", op->data.var);
      }
      if(i != expression->opCount - 1)
        printf(" %c ", opC);
      else if(expression->parenthesized)
        printf(")");
    }
  }
  calls--;
}
