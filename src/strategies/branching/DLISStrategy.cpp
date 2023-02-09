#include "DLISStrategy.h"

// Implementing Dynamic Largest Individual Sum heuristic
std::pair<Variable *, LiteralValue> DLISStrategy::choose(const Formula &formula) const {
    size_t occurrences = 0;
    LiteralValue literal = U;
    Variable *target_variable = nullptr;

    for (const auto &variable: formula.variables) {
        if (variable->value == U) {
            size_t current_positive_occurrences =
                    std::count_if(
                            variable->positive_occurrences.cbegin(),
                            variable->positive_occurrences.cend(),
                            [](const auto &clause) { return clause->active; });

            size_t current_negative_occurrences = std::count_if(
                    variable->negative_occurrences.cbegin(),
                    variable->negative_occurrences.cend(),
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

    assert(target_variable != nullptr);
    return {target_variable, literal};
}
