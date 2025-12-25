#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parsing.h"
#include "algebra.h"

#define MODE_EVAL 1
#define MODE_SIMPLIFY 2

void promptValues(Expression* expr, bool* valueMap);

int main(int argc, char** argv) {
	unsigned char mode = MODE_EVAL;
	if(argc < 2) {
		printf("Usage: bav \"Expression\" -[e,s]\n-e: Evaluate the expression\n-s: simplify the expression\n");
		exit(EXIT_SUCCESS);
	}
	char* exprStr = argv[1];
	if(argc > 2) {
		if(strcmp("-s", argv[2]) == 0) mode = MODE_SIMPLIFY;
	}
	printf("Received string: %s\n", exprStr);
	Expression expr = createExpression(exprStr);

	if(mode == MODE_EVAL) {
		bool valueMap[26] = {0};
		promptValues(&expr, valueMap);
		bool result = eval(&expr, valueMap);
		printf("Expression resulted in %s\n", result? "true": "false");
	}

	// NEXT: Evaluation pretty much done. Do simplfication next

	return 0;
}

void promptValues(Expression* expr, bool* valueMap) {
	if(expr->exprType == TYPE_VAR) {
		char name = expr->exprValue.var.name;
		printf("Enter value for %c\n", name);
		char* input = NULL;
		size_t input_siz = 0;
		if(getline(&input, &input_siz, stdin) < 0) {
			perror("Cannot get input");
			exit(EXIT_FAILURE);
		}
		// MAYBE: check if this is a valid string
		valueMap[name - 'A'] = atoi(input);
	} else {
		for(unsigned int i = 0; i < expr->exprValue.expr.operandCount; i++) {
			promptValues(expr->exprValue.expr.operands + i, valueMap);
		}
	}
}
