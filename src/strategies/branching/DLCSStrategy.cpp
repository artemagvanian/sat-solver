#include "strategies/branching/DLCSStrategy.h"

// Implementing Dynamic Largest Combined Sum heuristic
std::pair<Variable *, LiteralValue> DLCSStrategy::choose(const Formula &formula) const {
    size_t positive_occurrences = 0;
    size_t negative_occurrences = 0;
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

            if (current_positive_occurrences + current_negative_occurrences >
                positive_occurrences + negative_occurrences) {
                positive_occurrences = current_positive_occurrences;
                negative_occurrences = current_negative_occurrences;
                target_variable = variable;
            }
        }
    }

    assert(target_variable != nullptr);

    if (positive_occurrences > negative_occurrences) {
        return {target_variable, T};
    } else {
        return {target_variable, F};
    }
}

