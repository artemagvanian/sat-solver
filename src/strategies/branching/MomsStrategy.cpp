#include "MomsStrategy.h"

// Implementing Maximum Occurrences in clauses of Minimum Size heuristic
std::pair<Variable *, LiteralValue> MomsStrategy::choose(const Formula &formula) const {
    size_t minimum_size = SIZE_MAX;
    std::unordered_map<ID, size_t> oms;
    std::unordered_set<Variable *> variables;

    for (const auto &clause: formula.clauses) {
        if (clause->active) {
            if (clause->active_literals < minimum_size) {
                oms.clear();
                variables.clear();
                minimum_size = clause->active_literals;
                for (const auto &literal: clause->literals) {
                    oms[literal.sign * literal.variable->id]++;
                    variables.insert(literal.variable);
                }
            } else if (clause->active_literals == minimum_size) {
                for (const auto &literal: clause->literals) {
                    oms[literal.sign * literal.variable->id]++;
                    variables.insert(literal.variable);
                }
            }
        }
    }

    size_t moms = 0;
    Variable *moms_variable = nullptr;

    for (const auto &variable: variables) {
        if (variable->value == U) {
            size_t f = (oms[variable->id] + oms[-variable->id]) * static_cast<long>(std::pow(2, k))
                       + oms[variable->id] * oms[-variable->id];
            if (f > moms) {
                moms = f;
                moms_variable = variable;
            }
        }
    }

    return {moms_variable, oms[moms_variable->id] > oms[-moms_variable->id] ? T : F};

}
