#pragma once

#include "Formula.h"

class Verifier {
public:
    static bool verify(const std::unordered_map<ID, std::unordered_set<Literal>> &clauses,
                       const std::unordered_map<Variable, LiteralValue> &assignments);
};
