#pragma once

#include "Formula.h"

class Verifier {
public:
    static bool verify(const std::vector<Clause *> &clauses,
                       const std::unordered_map<Variable, VariableData> &variables);
};
