// test_createExpression.c
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <stdbool.h>
#include "parsing.h"

// Helper to check if an operand is a variable
static void assert_expr_var(Expression* e, char expectedName) {
  cr_assert_eq(e->exprType, TYPE_VAR, "Expected Expression type to be TYPE_VAR");
  cr_assert_eq(e->exprValue.var.name, expectedName, "Variable name %c does not match expected name %c", e->exprValue.var.name, expectedName);
}

// Helper to check if an operand is an expression
static void assert_expr_expr(Expression* e, Actions expectedAction, int expectedOpCount) {
  cr_assert_lt(expectedAction, sizeof(opChars)/sizeof(opChars[0]), "Action %i is invalid", expectedAction);
  cr_assert_eq(e->exprType, TYPE_EXPR, "Expected Expression type to be TYPE_EXPR");
  cr_assert_eq(e->exprValue.expr.operandCount, expectedOpCount, "Operand count %i does not match expected operand count %i",
               e->exprValue.expr.operandCount, expectedOpCount);
  cr_assert_eq(e->exprValue.expr.action, expectedAction, "Operation %c does not match expected operation %c",
               opChars[e->exprValue.expr.action],  opChars[expectedAction]);
}

Test(createExpression, single_variable) {
  char exprStr[] = "A";
  Expression expr = createExpression(exprStr);
  cr_assert_eq(expr.parenthesized, false, "Expression should not be parenthesized");
  assert_expr_var(&expr, 'A');
}

Test(createExpression, not_expression) {
  char exprStr[] = "~A";
  Expression expr = createExpression(exprStr);
  cr_assert_eq(expr.parenthesized, false, "Expression should not be parenthesized");
  assert_expr_expr(&expr, ACT_NOT, 1);
  assert_expr_var(expr.exprValue.expr.operands, 'A');
}

Test(createExpression, and_expression) {
  char exprStr[] = "A*B";
  Expression expr = createExpression(exprStr);
  cr_assert_eq(expr.parenthesized, false, "Expression should not be parenthesized");
  assert_expr_expr(&expr, ACT_AND, 2);
  assert_expr_var(expr.exprValue.expr.operands, 'A');
  assert_expr_var(expr.exprValue.expr.operands+1, 'B');
}

Test(createExpression, or_with_parentheses) {
  char exprStr[] = "(A+B)";
  Expression expr = createExpression(exprStr);
  cr_assert_eq(expr.parenthesized, true, "Expression should be parenthesized");
}

Test(createExpression, no_top_level_parens) {
  char exprStr[] = "(A*B) + (A^A)";
  Expression expr = createExpression(exprStr);
  cr_assert_eq(expr.exprType, TYPE_EXPR, "Expected Expression type to be ExprType");
}

Test(createExpression, nested_expression) {
  char exprStr[] = "~(A*B)";
  Expression* subExpr;
  Expression expr = createExpression(exprStr);
  assert_expr_expr(&expr, ACT_NOT, 1);

  subExpr = expr.exprValue.expr.operands;
  assert_expr_expr(subExpr, ACT_AND, 2);
  cr_assert_eq(subExpr->parenthesized, true, "Expression should be parenthesized");

  //checking (A*B) operands
  Expression* op1 = subExpr->exprValue.expr.operands;
  assert_expr_var(op1, 'A');
  Expression* op2 = subExpr->exprValue.expr.operands+1;
  assert_expr_var(op2, 'B');
}

Test(createExpression, nary_or)	{
  char exprStr[] = "A+B+C+D";
  Expression expr = createExpression(exprStr);
  assert_expr_expr(&expr, ACT_OR, 4);

  Expression* op1 = expr.exprValue.expr.operands;
  Expression* op2 = expr.exprValue.expr.operands + 1;
  Expression* op3 = expr.exprValue.expr.operands + 2;
  Expression* op4 = expr.exprValue.expr.operands + 3;

  assert_expr_var(op1, 'A');
  assert_expr_var(op2, 'B');
  assert_expr_var(op3, 'C');
  assert_expr_var(op4, 'D');
}

Test(createExpression, nary_and)	{
  char exprStr[] = "A*B*C";
  Expression expr = createExpression(exprStr);
  assert_expr_expr(&expr, ACT_AND, 3);

  Expression* op1 = expr.exprValue.expr.operands;
  Expression* op2 = expr.exprValue.expr.operands + 1;
  Expression* op3 = expr.exprValue.expr.operands + 2;

  assert_expr_var(op1, 'A');
  assert_expr_var(op2, 'B');
  assert_expr_var(op3, 'C');
}

//This _might_ cause issues, i'm not sure if
//XOR can be N-ary. Though I think it can, it just becomes
//a even,odd counter essentially
Test(createExpression, xor_chain)	{
  char exprStr[] = "A^B^C";
  Expression expr = createExpression(exprStr);
  assert_expr_expr(&expr, ACT_XOR, 3);

  Expression* op1 = expr.exprValue.expr.operands;
  Expression* op2 = expr.exprValue.expr.operands + 1;
  Expression* op3 = expr.exprValue.expr.operands + 2;

  assert_expr_var(op1, 'A');
  assert_expr_var(op2, 'B');
  assert_expr_var(op3, 'C');
}

Test(createExpression, precedence_and_over_or)	{
  char exprStr[] = "A+B*C";
  Expression expr = createExpression(exprStr);
  assert_expr_expr(&expr, ACT_OR, 2);

  Expression* op1 = expr.exprValue.expr.operands;
  assert_expr_var(op1, 'A');

  Expression* op2 = expr.exprValue.expr.operands + 1;
  cr_assert_eq(expr.parenthesized, false, "Expression should not be parenthesized");
  assert_expr_expr(op2, ACT_AND, 2);
}

Test(createExpression, precedence_or_vs_and_left)	{
  char exprStr[] = "A*B+C";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, precedence_xor_lowest_left_group)	{
  char exprStr[] = "A+B^C";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, precedence_xor_lowest_right_group)	{
  char exprStr[] = "A^B+C";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, mixed_or_and_sequence)	{
  char exprStr[] = "A+B*C+D";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, spaces_handling)	{
  char exprStr[] = "  A   +   B*C  ";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, parentheses_without_not)	{
  char exprStr[] = "(A+B)*C";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, xor_over_or_both_sides)	{
  char exprStr[] = "A^B+C^D";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, xor_chain_on_right_of_or)	{
  char exprStr[] = "A+B^C^D";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, xor_left_then_two_ors)	{
  char exprStr[] = "A^B+C+D";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, parentheses_force_or_then_and)	{
  char exprStr[] = "(A+B)*C";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, parentheses_with_xor_and_or)	{
  char exprStr[] = "(A^B)+C";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, and_groups_around_xor)	{
  char exprStr[] = "A*B^C*D";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, spaces_and_parentheses_mixed)	{
  char exprStr[] = " ( A + B ) * ( C ^ D ) ";
  Expression expr = createExpression(exprStr);
}

Test(createExpression, single_variable_with_spaces)	{
  char* exprStr = "   Z   ";
  Expression expr = createExpression(exprStr);
  assert_expr_var(&expr, 'Z');
}

Test(createExpression, long_nary_or)	{
  char exprStr[] = "A+B+C+D+E+F+G";
  Expression expr = createExpression(exprStr);

  assert_expr_expr(&expr, ACT_OR, 7);

  Expression* op1 = expr.exprValue.expr.operands;
  Expression* op2 = expr.exprValue.expr.operands + 1;
  Expression* op3 = expr.exprValue.expr.operands + 2;
  Expression* op4 = expr.exprValue.expr.operands + 3;
  Expression* op5 = expr.exprValue.expr.operands + 4;
  Expression* op6 = expr.exprValue.expr.operands + 5;
  Expression* op7 = expr.exprValue.expr.operands + 6;

  assert_expr_var(op1, 'A');
  assert_expr_var(op2, 'B');
  assert_expr_var(op3, 'C');
  assert_expr_var(op4, 'D');
  assert_expr_var(op5, 'E');
  assert_expr_var(op6, 'F');
  assert_expr_var(op7, 'G');
}
