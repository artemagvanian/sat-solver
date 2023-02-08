#include "strategies/branching/DLCSStrategy.h"

// Implementing Dynamic Largest Combined Sum heuristic
std::pair<Variable, LiteralValue> DLCSStrategy::choose(const Formula &formula) const {
    size_t positive_occurrences = 0;
    size_t negative_occurrences = 0;
    Variable target_variable = 0;

    for (auto &assignment: formula.assignments) {
        const Variable &variable = assignment.first;
        const LiteralValue &value = assignment.second;

        if (value == U) {
            size_t current_positive_occurrences = formula.occurrences.at(variable).first.size();
            size_t current_negative_occurrences = formula.occurrences.at(variable).second.size();

            if (current_positive_occurrences + current_negative_occurrences >
                positive_occurrences + negative_occurrences) {
                positive_occurrences = current_positive_occurrences;
                negative_occurrences = current_negative_occurrences;
                target_variable = variable;
            }
        }
    }

    assert(target_variable != 0);

    if (positive_occurrences > negative_occurrences) {
        return {target_variable, T};
    } else {
        return {target_variable, F};
    }
}

