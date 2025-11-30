typedef struct Expr Expr;
typedef struct Operand Operand;

enum Actions {
  ACT_OR = 0,
  ACT_XOR = 1,
  ACT_AND = 2,
  ACT_NOT = 3,
  ACT_PAREN = 4,
  ACT_BUFF = 5,
  ACT_INVALID = 6
};

typedef enum {
  OP_EXPR,
  OP_VAR
} OperandType;

struct Expr {
  enum Actions action;
  Operand* operands;
  int opCount;
  bool parenthesized;
};

typedef union {
  Expr expr;
  char var;
} OperandValue;


struct Operand {
  OperandValue data;
  OperandType type;
};

extern char opChars[];

char* trim(const char* exprStr);
void validateExpression(const char * exprStr);
void printExpr(Expr* expression);
//parseExpression potentially modifies exprStr,
//parseOperand may call parseExpression, so it may also modify
//exprStr, therefore, neither can use a const char*
Expr parseExpression(char* exprStr);
Operand parseOperand(char* opStr);
