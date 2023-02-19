#include <random>

#include "VSIDSStrategy.h"

#define SEED 2951

std::pair<Variable *, LiteralValue> VSIDSStrategy::choose(const Formula &formula) const {
    std::mt19937 mt(SEED);

    double conflicts_max = 0;
    std::vector<std::pair<Variable *, LiteralValue>> target_variables = {};

    for (const auto &variable: formula.variables) {
        if (variable->value == U) {
            if (variable->positive_conflicts + variable->negative_conflicts > conflicts_max) {
                conflicts_max = variable->positive_conflicts + variable->negative_conflicts;
                target_variables = {{variable,
                                     variable->positive_conflicts > variable->negative_conflicts ? F : T}};
            } else if (variable->positive_conflicts + variable->negative_conflicts == conflicts_max &&
                       conflicts_max != 0) {
                target_variables.emplace_back(variable,
                                              variable->positive_conflicts > variable->negative_conflicts ? F : T);
            }
        }
    }

    if (conflicts_max == 0) {
        long max_positive_occurrences = 0;
        long max_negative_occurrences = 0;
        Variable *max_variable = nullptr;
        for (const auto &variable: formula.variables) {
            if (variable->value == U) {
                if (variable->positive_occurrences.size() + variable->negative_occurrences.size() >=
                    max_positive_occurrences + max_negative_occurrences) {
                    max_positive_occurrences = static_cast<long>(variable->positive_occurrences.size());
                    max_negative_occurrences = static_cast<long>(variable->negative_occurrences.size());
                    max_variable = variable;
                }
            }
        }
        assert(max_variable != nullptr);
        assert(max_variable->value == U);
        return {max_variable, max_positive_occurrences > max_negative_occurrences ? T : F};
    }

    std::uniform_int_distribution<std::mt19937::result_type> dist(0, target_variables.size() - 1);
    std::pair<Variable *, LiteralValue> target_assignment = target_variables.at(dist(mt));

    assert(target_assignment.first != nullptr);
    assert(target_assignment.first->value == U);
    return target_assignment;
}
