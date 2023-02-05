#include "strategies/branching/DLCSStrategy.h"

std::pair<int, LiteralValue> DLCSStrategy::choose(const Formula &initial) const {
    int positive_occurrences = 0;
    int negative_occurrences = 0;
    int target_variable = 0;

    for (auto &assignment: initial.assignments) {
        const int &variable = assignment.first;
        const LiteralValue &value = assignment.second;

        if (value == U) {
            int current_positive_occurrences = 0;
            int current_negative_occurrences = 0;

            for (auto &clause: initial.clauses) {
                if (clause.find(variable) != clause.end()) {
                    current_positive_occurrences++;
                } else if (clause.find(-variable) != clause.end()) {
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

