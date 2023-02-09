#include "JeroslowWangStrategy.h"

// Implementing Jeroslow-Wang heuristic
std::pair<Variable, LiteralValue> JeroslowWangStrategy::choose(const Formula &formula) const {
    double max_j = 0;
    Variable variable;
    LiteralValue value;

    for (const auto &variable_data_pair: formula.variables) {
        if (variable_data_pair.second.value == U) {
            double positive_j = 0;
            for (const auto &positive_occurrence: variable_data_pair.second.positive_occurrences) {
                if (positive_occurrence->active) {
                    positive_j += std::pow(2, -positive_occurrence->active_literals);
                }
            }
            if (positive_j > max_j) {
                max_j = positive_j;
                variable = variable_data_pair.first;
                value = T;
            }
            double negative_j = 0;
            for (const auto &negative_occurrence: variable_data_pair.second.negative_occurrences) {
                if (negative_occurrence->active) {
                    negative_j += std::pow(2, -negative_occurrence->active_literals);
                }
            }
            if (negative_j > max_j) {
                max_j = negative_j;
                variable = variable_data_pair.first;
                value = F;
            }
        }
    }

    return {variable, value};
}
