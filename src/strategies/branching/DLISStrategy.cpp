#include "DLISStrategy.h"

// Implementing Dynamic Largest Individual Sum heuristic
std::pair<Variable, LiteralValue> DLISStrategy::choose(const Formula &formula) const {
    size_t occurrences = 0;
    LiteralValue literal = U;
    Variable target_variable = 0;

    for (auto &assignment: formula.assignments) {
        const Variable &variable = assignment.first;
        const LiteralValue &value = assignment.second;

        if (value == U) {
            size_t current_positive_occurrences = formula.occurrences.at(variable).first.size();
            size_t current_negative_occurrences = formula.occurrences.at(variable).second.size();

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
