#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parsing.h"

void createExprFromInput(char* exprStr);

#define MODE_EVAL 0
#define MODE_SIMPLIFY 1

int main(int argc, char** argv) {
  int mode = MODE_EVAL;
  char* exprStr;
  switch(argc) {
  case 2:
  case 3: {
    exprStr = argv[1];
    if(argc == 3) {
      char* option = argv[2];
      size_t len = strlen(option);
      len = len < 2? len: 2; //options are - followed by single letter, so 2 at most
      if(strncmp(option, "-e", len) == 0) {
        mode = MODE_EVAL;
      } else if (strncmp(option, "-s", len) == 0) {
        mode = MODE_SIMPLIFY;
      }
    }
    break;
  }
  default:
    printf("Args passed: %i\n", argc);
    printf("Usage: bav \"expression\" -[e,s] \n -e: Evaluate the expression \n -s: simplify the expression\n");
    return 0;
  }
  //parse into AST
  printf("Given expression is: %s\n", exprStr);
  Expression expr =  createExpression(exprStr);
  printf("The expression is: ");
  printExpression(&expr);
  cleanup(&expr);
  printf("\n");
  return 0;
}
