#include "MomsStrategy.h"

// Implementing Maximum Occurrences in clauses of Minimum Size heuristic
std::pair<Variable, LiteralValue> MomsStrategy::choose(const Formula &formula) const {
    size_t minimum_size = SIZE_MAX;
    std::unordered_map<Literal, size_t> oms;
    std::unordered_set<Variable> variables;

    for (const auto &clause: formula.clauses) {
        if (clause.second.size() < minimum_size) {
            oms.clear();
            variables.clear();
            minimum_size = clause.second.size();
            for (const auto &literal: clause.second) {
                oms[literal]++;
                variables.insert(std::abs(literal));
            }
        } else if (clause.second.size() == minimum_size) {
            for (const auto &literal: clause.second) {
                oms[literal]++;
                variables.insert(std::abs(literal));
            }
        }
    }

    size_t moms = 0;
    Variable moms_variable = 0;

    for (const auto &variable: variables) {
        size_t f = (oms[variable] + oms[-variable]) * static_cast<long>(std::pow(2, k))
                   + oms[variable] * oms[-variable];
        if (f > moms) {
            moms = f;
            moms_variable = variable;
        }
    }

    return {moms_variable, oms[moms_variable] > oms[-moms_variable] ? T : F};

}
