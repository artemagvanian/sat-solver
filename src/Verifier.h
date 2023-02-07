#pragma once

#include "Formula.h"

class Verifier {
public:
    static bool verify(const std::vector<std::unordered_set<Literal>> &clauses,
                       const std::unordered_map<Variable, LiteralValue> &assignments);
};
