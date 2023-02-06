#include "strategies/branching/DLCSStrategy.h"

// Implementing Dynamic Largest Combined Sum heuristic
std::pair<Variable, LiteralValue> DLCSStrategy::choose(const Formula &initial) const {
    size_t positive_occurrences = 0;
    size_t negative_occurrences = 0;
    Variable target_variable = 0;

    for (auto &assignment: initial.assignments) {
        const Variable &variable = assignment.first;
        const LiteralValue &value = assignment.second;

        if (value == U) {
            size_t current_positive_occurrences = 0;
            size_t current_negative_occurrences = 0;

            for (auto &clause: initial.clauses) {
                if (clause.find(static_cast<Literal>(variable)) != clause.end()) {
                    current_positive_occurrences++;
                } else if (clause.find(-static_cast<Literal>(variable)) != clause.end()) {
                    current_negative_occurrences++;
                }
            }

            if (current_positive_occurrences + current_negative_occurrences >
                positive_occurrences + negative_occurrences) {
                positive_occurrences = current_positive_occurrences;
                negative_occurrences = current_negative_occurrences;
                target_variable = variable;
            }
        }
    }

    if (positive_occurrences > negative_occurrences) {
        return {target_variable, T};
    } else {
        return {target_variable, F};
    }
}

