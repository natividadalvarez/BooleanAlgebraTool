#pragma once
#include <stdbool.h>
#include "parsing.h"

bool checkIdentity();
bool eval(Expression* expr, bool* valueVarMap);
