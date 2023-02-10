#include "Verifier.h"

bool Verifier::verify(const std::vector<Clause *> &clauses) {
    for (const auto &clause: clauses) {
        bool clause_satisfied = false;
        for (const auto &literal: clause->literals) {
            if ((literal.sign > 0 && literal.variable->value != F) ||
                (literal.sign < 0 && literal.variable->value == F)) {
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
