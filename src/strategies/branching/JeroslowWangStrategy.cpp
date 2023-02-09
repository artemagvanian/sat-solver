#include "JeroslowWangStrategy.h"

// Implementing Jeroslow-Wang heuristic
std::pair<Variable *, LiteralValue> JeroslowWangStrategy::choose(const Formula &formula) const {
    double max_j = 0;
    Variable *target_variable;
    LiteralValue value;

    for (const auto &variable: formula.variables) {
        if (variable->value == U) {
            double positive_j = 0;
            for (const auto &positive_occurrence: variable->positive_occurrences) {
                if (positive_occurrence->active) {
                    positive_j += std::pow(2, -positive_occurrence->active_literals);
                }
            }
            if (positive_j > max_j) {
                max_j = positive_j;
                target_variable = variable;
                value = T;
            }
            double negative_j = 0;
            for (const auto &negative_occurrence: variable->negative_occurrences) {
                if (negative_occurrence->active) {
                    negative_j += std::pow(2, -negative_occurrence->active_literals);
                }
            }
            if (negative_j > max_j) {
                max_j = negative_j;
                target_variable = variable;
                value = F;
            }
        }
    }

    return {target_variable, value};
}
