typedef struct Expression Expression;

typedef enum {
  ACT_OR = 0,
  ACT_XOR = 1,
  ACT_AND = 2,
  ACT_NOT = 3,
  ACT_INVALID = 4
} Actions;

typedef enum {
  FALSE = 0,
  TRUE = 1,
  UNDEF = 2
} Value;

typedef struct {
  Actions action;
  Expression* operands;
  int operandCount;
} Expr;

typedef struct {
  char name;
  Value value;
} Variable;

typedef union {
  Expr expr;
  Variable var;
} ExprValue;

typedef enum {
  TYPE_EXPR,
  TYPE_VAR
} ExprType;

struct Expression {
  ExprType exprType;
  ExprValue exprValue;
  bool parenthesized;
};

extern char opChars[5];

Expression createExpression(const char* exprStr);
void printExpression(Expression* expression);
void cleanup(Expression* e);
