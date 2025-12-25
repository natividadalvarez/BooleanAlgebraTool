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
#include <assert.h>
#include "parsing.h"

//in order from lowest to highest precedence
char opChars[5] = {'+', '^', '*', '~', 0};

static void validateExpression(const char * exprStr) {
  int parenCount = 0;
  bool parenEmpty = true;
  const char* comp = exprStr;
  while(*comp != '\0') {
    //check if string contains any invalid chars
    bool charIsVarSpaceParen = (*comp >= 'A' && *comp <= 'Z') || *comp == ' ' || *comp == '(' || *comp == ')';
    bool charIsOp = false;
    for(size_t i = 0; i < sizeof(opChars)/sizeof(opChars[0]); i++) {
      if(*comp == opChars[i] ) {
        charIsOp = true;
        break;
      }
    }
    if(!(charIsVarSpaceParen || charIsOp)) {
      printf("Expression contains invalid characters.\n");
      exit(EXIT_FAILURE);
    }

    //parenthesis parity checking
    if(*comp == '(') {
      parenEmpty = true;
      parenCount++;
    }
    else if(*comp == ')') {
      if(parenEmpty) {
        printf("Expression is invalid: contains empty ()\n");
        exit(EXIT_FAILURE);
      }
      parenCount--;
    } else if(*comp != ' ' && *comp != '\t' && *comp != '\n') { //may need to add more
      parenEmpty = false;
    }
    comp++;
  }

  if(parenCount !=0) {
		static const char whichone[2] = {'(', ')'};
		printf("Expression is invalid: has extra %c \n", whichone[parenCount < 0]);
		exit(EXIT_FAILURE);
	}
}

//Removes all whitespace from ends of string. Modifies exprStr in place
static void trim(char** exprStr) {
  while(*(*exprStr) == ' ' || *(*exprStr) == '\t') (*exprStr)++;
  size_t exprLen = strlen(*exprStr);

  bool spaceFound = false;
  size_t firstNonSpace = exprLen - 1;
  const char* s = *exprStr;

  for(; firstNonSpace > 0; firstNonSpace--) {
    if(s[firstNonSpace] != ' ' && s[firstNonSpace] != '\t'){
      break;
    }
    spaceFound = true;
  }

  if(spaceFound) {
    (*exprStr)[firstNonSpace + 1] = '\0';
  }
}

//Assumes given validated expression
static bool topLevelParen(const char* exprStr, size_t exprLen) {
  if(exprStr[0] != '(' || exprStr[exprLen - 1] != ')') return false;
  int parenCount = 1;
  for(size_t i = 1; i < exprLen; i++) {
    if(parenCount == 0) return false; //we found its pair before the end, so it's not top level
    if(exprStr[i] == ')') parenCount--;
    if(exprStr[i] == '(') parenCount++;
  }
  return parenCount == 0;
}

//Strips all top level parenthesis from expression. Modifies exprStr in place
//assumes parentheis is already validated by validateExpression
static void stripParenthesis(Expression* e, char** exprStr, size_t* exprLen) {
  if(topLevelParen(*exprStr, *exprLen)) {
    e->parenthesized = true;
    char letter = *(*exprStr);
    while(letter == '(' && *((*exprStr)+*exprLen-1) == ')'){
      (*exprStr)++;
      (*exprLen)-=2;
      (*exprStr)[*exprLen] = '\0';
      letter = (*exprStr)[0];
    }
  }
}

Actions lowestPrecedenceOp(char* exprStr, size_t exprLen) {
  Actions op = ACT_INVALID;
  int parenCount =  0;
  for(size_t i = 0; i <= exprLen; i++) {
    char c = exprStr[i];
    if(c == '(') parenCount++;
    else if(c == ')') parenCount--;
    if(parenCount > 0) continue;

    if(c == opChars[ACT_OR]) {
      op = ACT_OR;
      break;
    } else if (c == opChars[ACT_XOR]) {
      if(op <= ACT_XOR) continue;
      op = ACT_XOR;
    } else if (c == opChars[ACT_AND]) {
      if(op <= ACT_AND) continue;
      op = ACT_AND;
    } else if (c == opChars[ACT_NOT]) {
      if(op <= ACT_NOT) continue;
      op = ACT_NOT;
    }
  }
  if(op == ACT_INVALID) {
    printf("Expression is invalid: Invalid operator\n" );
    exit(EXIT_FAILURE);
  }
  return op;
}

void parseNaryOp(Expression* e, char* exprStr, size_t exprLen, Actions op) {
  //find out how many operands we have
  Expr* exprVal = &e->exprValue.expr;
  exprVal->operandCount = 0;
  size_t opIdx = 0;
  int parenCount =  0;
  for(size_t i = 0; i <= exprLen; i++) {
    if(exprStr[i] == '(') parenCount++;
    else if(exprStr[i] == ')') parenCount--;
    if(parenCount != 0) continue;

    if(exprStr[i] == opChars[op]) {
      exprVal->operandCount++;
      opIdx = i;
    }
  }
  if(opIdx >= exprLen) {
    //if this is true, the expression must have ended with an operator,
    //which is not valid (e.g. A+B+)
    printf("Expression %s is invalid\n", exprStr);
    exit(EXIT_FAILURE);
  }
  assert(exprVal->operandCount != 0);
  exprVal->operandCount++;
  exprVal->operands = malloc(sizeof(Expression) * exprVal->operandCount);

  int operandIdx = 0;
  parenCount = 0;
  size_t readOperandFrom = 0;
  for(size_t i = 0; i <= exprLen; i++) {
    char c = exprStr[i];
    //skip everything in parenthesis, they have a higher precedence
    if(c == '(') parenCount++;
    else if(c == ')') parenCount--;
    if(parenCount != 0) continue;

    if(c == opChars[op] && parenCount == 0) {
      size_t len = i - readOperandFrom;
      char* str = exprStr;
      str+=readOperandFrom;
      str[len] = '\0';
      exprVal->operands[operandIdx++] = createExpression(str);
      readOperandFrom = i+1;
    }
  }
  size_t len = exprLen - readOperandFrom;
  char* str = exprStr;
  str+=readOperandFrom;
  str[len] = '\0';
  exprVal->operands[operandIdx++] = createExpression(str);
}

//There's really only one unary operation, negation
void parseUnaryOp(Expression* e, char* exprStr, size_t exprLen, Actions op) {
  //must be true for any unary op. If there was other stuff to parse we wouldn't
  //have gotten to this point yet
  assert(exprStr[0] == opChars[op]);
  if(exprLen == 1) {
    printf("Expression %s is invalid. %c operator has no operand\n", exprStr, opChars[op]);
    exit(EXIT_FAILURE);
  }

  Expr* exprVal = &e->exprValue.expr;
  exprVal->operandCount = 1;
  exprVal->operands = malloc(sizeof(Expression));

  size_t len = exprLen - 1;
  char* str = exprStr+1;
  str[len] = '\0';
  exprVal->operands[0] = createExpression(str);
}

//Recursively parses sub-expressions until atomic unit (variable or constant) is found.
//AST will be N-ary tree of operator with its operand. Will modify exprStr in place,
//so make sure you give it a string that can be modified (i.e. stack/heap allocated)
Expression createExpression(char* exprStr) {
  static int recursion_guard = 0;
  recursion_guard++;
  if(recursion_guard > 10000) {
    printf("You have a recursion problem\n");
    exit(EXIT_FAILURE);
  }
  trim(&exprStr);
  validateExpression(exprStr);

  Expression e = {
    .parenthesized = false
  };
  size_t exprLen = strlen(exprStr);
  stripParenthesis(&e, &exprStr, &exprLen);

  if(exprLen == 1) { //this should be a var
    e.exprType = TYPE_VAR;
    e.exprValue.var = (Variable){
      .name = exprStr[0],
      .value = UNDEF
    };
  } else {
    e.exprType = TYPE_EXPR;
    //find the lowest precedence operator
    Actions op = lowestPrecedenceOp(exprStr, exprLen);
    e.exprValue.expr = (Expr){
      .action = op
    };
    if(op != ACT_NOT) {
      parseNaryOp(&e, exprStr, exprLen, op);
    } else {
      parseUnaryOp(&e, exprStr, exprLen, op);
    }
  }
  return e;
}

void printExpression(Expression* expression) {
  if(expression->parenthesized) printf("(");
  if(expression->exprType == TYPE_VAR) {
    printf("%c", expression->exprValue.var.name);
  } else {
    Expr* expr = &expression->exprValue.expr;
    if(expr->operandCount > 1) {
      for(unsigned int i = 0; i < expr->operandCount; i++) {
        Expression* e = expr->operands + i;
        printExpression(e);
        if(i < expr->operandCount - 1)
          printf(" %c ", opChars[expr->action]);
      }
    } else {
      printf("%c", opChars[expr->action]);
      printExpression(expr->operands);
    }
  }
  if(expression->parenthesized) printf(")");
}

void cleanup(Expression* e) {
  if(e->exprType == TYPE_VAR) {
    return;
  } else {
    for(unsigned int i = 0; i < e->exprValue.expr.operandCount; i++) {
      cleanup(e->exprValue.expr.operands+i);
    }
  }
  free(e->exprValue.expr.operands);
}
