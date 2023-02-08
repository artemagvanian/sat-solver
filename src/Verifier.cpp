#include "Verifier.h"

bool Verifier::verify(const std::unordered_map<ID, std::unordered_set<Literal>> &clauses,
                      const std::unordered_map<Variable, LiteralValue> &assignments) {
    for (const auto &clause: clauses) {
        bool clause_satisfied = false;
        for (const auto &literal: clause.second) {
            if ((literal > 0 && assignments.at(static_cast<Variable>(literal)) != F) ||
                (literal < 0 && assignments.at(static_cast<Variable>(-literal)) == F)) {
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
