#include "DLISStrategy.h"

// Implementing Dynamic Largest Individual Sum heuristic
std::pair<Variable, LiteralValue> DLISStrategy::choose(const Formula &formula) const {
    size_t occurrences = 0;
    LiteralValue literal = U;
    Variable target_variable = 0;

    for (const auto &variable_data_pair: formula.variables) {
        const Variable &variable = variable_data_pair.first;
        const LiteralValue &value = variable_data_pair.second.value;

        if (value == U) {
            size_t current_positive_occurrences =
                    std::count_if(
                            variable_data_pair.second.positive_occurrences.cbegin(),
                            variable_data_pair.second.positive_occurrences.cend(),
                            [](const auto &clause) { return clause->active; });

            size_t current_negative_occurrences = std::count_if(
                    variable_data_pair.second.negative_occurrences.cbegin(),
                    variable_data_pair.second.negative_occurrences.cend(),
                    [](const auto &clause) { return clause->active; });

            if (std::max(current_positive_occurrences, current_negative_occurrences) > occurrences) {
                occurrences = std::max(current_positive_occurrences, current_negative_occurrences);
                if (current_positive_occurrences >= current_negative_occurrences) {
                    literal = T;
                } else {
                    literal = F;
                }
                target_variable = variable;
            }
        }
    }

    assert(target_variable != 0);
    return {target_variable, literal};
}
