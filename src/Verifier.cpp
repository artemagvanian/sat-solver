#include "Verifier.h"

bool Verifier::verify(const std::vector<Clause*> &clauses,
                      const std::unordered_map<Variable, VariableData> &variables) {
    for (const auto &clause: clauses) {
        bool clause_satisfied = false;
        for (const auto &literal: clause->literals) {
            if ((literal > 0 && variables.at(literal).value != F) ||
                (literal < 0 && variables.at(-literal).value == F)) {
                clause_satisfied = true;
                break;
            }
        }
        if (!clause_satisfied) {
            return false;
        }
    }
    return true;
}
