// test_parseExpression.c
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "parsing.h"

// Helper to check if an operand is a variable
static void assert_operand_var(Operand *op, char expected) {
    cr_assert_eq(op->type, OP_VAR, "Expected OP_VAR, got %d", op->type);
    cr_assert_eq(op->data.var, expected, "Expected var %c, got %c", expected, op->data.var);
}

// Helper to check if an operand is an expression
static void assert_operand_expr(Operand *op, enum Actions expected_action, int expected_opCount) {
    cr_assert_eq(op->type, OP_EXPR, "Expected OP_EXPR, got %d", op->type);
    cr_assert_eq(op->data.expr.action, expected_action, "Expected action %d, got %d",
                 expected_action, op->data.expr.action);
    cr_assert_eq(op->data.expr.opCount, expected_opCount, "Expected %d operands, got %d",
                 expected_opCount, op->data.expr.opCount);
}


Test(parseExpression, trim_function)	{
  char* exprStr = trim("  Z ");
  cr_assert_str_eq(exprStr, "Z", "String '  Z ' should become 'Z'");
  free(exprStr);
  exprStr = trim("Z   ");
  cr_assert_str_eq(exprStr, "Z", "String 'Z   ' should become 'Z'");
  free(exprStr);
  exprStr = trim("   Z");
  cr_assert_str_eq(exprStr, "Z", "String '   Z' should become 'Z'");
  free(exprStr);
}

Test(parseExpression, single_variable) {
    char exprStr[] = "A";
    Expr expr = parseExpression(exprStr);
    cr_assert_eq(expr.action, ACT_BUFF, "Single variable should have ACT_BUFF action");
    cr_assert_eq(expr.opCount, 1, "Single variable should have one operand");
    assert_operand_var(&expr.operands[0], 'A');
}

Test(parseExpression, not_expression) {
    char exprStr[] = "!A";
    Expr expr = parseExpression(exprStr);
    cr_assert_eq(expr.action, ACT_NOT, "Expected NOT action");
    cr_assert_eq(expr.opCount, 1, "NOT should have 1 operand");
    assert_operand_var(&expr.operands[0], 'A');
}

Test(parseExpression, and_expression) {
    char exprStr[] = "A*B";
    Expr expr = parseExpression(exprStr);
    cr_assert_eq(expr.action, ACT_AND, "Expected AND action");
    cr_assert_eq(expr.opCount, 2, "Expected 2 operands");

    assert_operand_var(&expr.operands[0], 'A');
    assert_operand_var(&expr.operands[1], 'B');
}

Test(parseExpression, or_with_parentheses) {
	char exprStr[] = "(A+B)";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Expected OR action");
	cr_assert(expr.parenthesized, "Expected expression to be parenthesized");

	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_var(&expr.operands[1], 'B');
}

Test(parseExpression, no_top_level_parens) {
    char exprStr[] = "(A*B) + (A^A)";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Expected OR action");

	assert_operand_expr(&expr.operands[0], ACT_AND, 2);
	assert_operand_expr(&expr.operands[1], ACT_XOR, 2);
}

Test(parseExpression, nested_expression) {
		char exprStr[] = "!(A*B)";
    Expr expr = parseExpression(exprStr);
    cr_assert_eq(expr.action, ACT_NOT, "Expected NOT action");
    cr_assert_eq(expr.opCount, 1, "Expected 1 operand");

    Operand *inner = expr.operands;
    assert_operand_expr(inner, ACT_AND, 2);
    assert_operand_var(&inner->data.expr.operands[0], 'A');
    assert_operand_var(&inner->data.expr.operands[1], 'B');
}

Test(parseExpression, variadic_or)	{
	char exprStr[] = "A+B+C+D";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Expected OR action");
	cr_assert_eq(expr.opCount, 4, "Expected 4 operands");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_var(&expr.operands[1], 'B');
	assert_operand_var(&expr.operands[2], 'C');
	assert_operand_var(&expr.operands[3], 'D');
}

Test(parseExpression, variadic_and)	{
	char exprStr[] = "A*B*C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_AND, "Expected AND action");
	cr_assert_eq(expr.opCount, 3, "Expected 3 operands");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_var(&expr.operands[1], 'B');
	assert_operand_var(&expr.operands[2], 'C');
}

Test(parseExpression, xor_chain)	{
	char exprStr[] = "A^B^C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_XOR, "Expected XOR action");
	cr_assert_eq(expr.opCount, 3, "Expected 3 operands for XOR chain");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_var(&expr.operands[1], 'B');
	assert_operand_var(&expr.operands[2], 'C');
}

Test(parseExpression, precedence_and_over_or)	{
	char exprStr[] = "A+B*C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_expr(&expr.operands[1], ACT_AND, 2);
}

Test(parseExpression, precedence_or_vs_and_left)	{
	char exprStr[] = "A*B+C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_AND, 2);
	assert_operand_var(&expr.operands[1], 'C');
}

Test(parseExpression, precedence_xor_lowest_left_group)	{
	char exprStr[] = "A+B^C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR when XOR has higher precedence");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_expr(&expr.operands[1], ACT_XOR, 2);
}

Test(parseExpression, precedence_xor_lowest_right_group)	{
	char exprStr[] = "A^B+C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR when XOR has higher precedence");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_XOR, 2);
	assert_operand_var(&expr.operands[1], 'C');
}

Test(parseExpression, mixed_or_and_sequence)	{
	char exprStr[] = "A+B*C+D";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR");
	cr_assert_eq(expr.opCount, 3, "Expected 3 operands for OR");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_expr(&expr.operands[1], ACT_AND, 2);
	assert_operand_var(&expr.operands[2], 'D');
}

Test(parseExpression, spaces_handling)	{
	char exprStr[] = "  A   +   B*C  ";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_expr(&expr.operands[1], ACT_AND, 2);
}

Test(parseExpression, parentheses_without_not)	{
	char exprStr[] = "(A+B)*C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_AND, "Top should be AND");
	cr_assert_eq(expr.opCount, 2, "AND should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_OR, 2);
	assert_operand_var(&expr.operands[1], 'C');
}

Test(parseExpression, xor_over_or_both_sides)	{
	char exprStr[] = "A^B+C^D";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR when XOR has higher precedence");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_XOR, 2);
	assert_operand_expr(&expr.operands[1], ACT_XOR, 2);
}

Test(parseExpression, xor_chain_on_right_of_or)	{
	char exprStr[] = "A+B^C^D";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_expr(&expr.operands[1], ACT_XOR, 3);
}

Test(parseExpression, xor_left_then_two_ors)	{
	char exprStr[] = "A^B+C+D";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR");
	cr_assert_eq(expr.opCount, 3, "OR should have 3 operands when variadic");
	assert_operand_expr(&expr.operands[0], ACT_XOR, 2);
	assert_operand_var(&expr.operands[1], 'C');
	assert_operand_var(&expr.operands[2], 'D');
}

Test(parseExpression, parentheses_force_or_then_and)	{
	char exprStr[] = "(A+B)*C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_AND, "Top should be AND");
	cr_assert_eq(expr.opCount, 2, "AND should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_OR, 2);
	assert_operand_var(&expr.operands[1], 'C');
}

Test(parseExpression, parentheses_with_xor_and_or)	{
	char exprStr[] = "(A^B)+C";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Top should be OR");
	cr_assert_eq(expr.opCount, 2, "OR should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_XOR, 2);
	assert_operand_var(&expr.operands[1], 'C');
}

Test(parseExpression, and_groups_around_xor)	{
	char exprStr[] = "A*B^C*D";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_XOR, "Top should be XOR (AND higher than XOR)");
	cr_assert_eq(expr.opCount, 2, "XOR should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_AND, 2);
	assert_operand_expr(&expr.operands[1], ACT_AND, 2);
}

Test(parseExpression, spaces_and_parentheses_mixed)	{
	char exprStr[] = " ( A + B ) * ( C ^ D ) ";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_AND, "Top should be AND");
	cr_assert_eq(expr.opCount, 2, "AND should have 2 operands");
	assert_operand_expr(&expr.operands[0], ACT_OR, 2);
	assert_operand_expr(&expr.operands[1], ACT_XOR, 2);
}

Test(parseExpression, single_variable_with_spaces)	{
	char* exprStr = "   Z   ";
  exprStr = trim(exprStr);
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_BUFF, "Single variable should have ACT_BUFF action");
	cr_assert_eq(expr.opCount, 1, "Single variable should have one operand");
	assert_operand_var(&expr.operands[0], 'Z');
  free(exprStr);
}

Test(parseExpression, long_variadic_or)	{
	char exprStr[] = "A+B+C+D+E+F+G";
	Expr expr = parseExpression(exprStr);
	cr_assert_eq(expr.action, ACT_OR, "Expected OR action");
	cr_assert_eq(expr.opCount, 7, "Expected 7 operands");
	assert_operand_var(&expr.operands[0], 'A');
	assert_operand_var(&expr.operands[1], 'B');
	assert_operand_var(&expr.operands[2], 'C');
	assert_operand_var(&expr.operands[3], 'D');
	assert_operand_var(&expr.operands[4], 'E');
	assert_operand_var(&expr.operands[5], 'F');
	assert_operand_var(&expr.operands[6], 'G');
}
