#include "JeroslowWangStrategy.h"

// Implementing Jeroslow-Wang heuristic
std::pair<Variable, LiteralValue> JeroslowWangStrategy::choose(const Formula &formula) const {
    double max_j = 0;
    Variable variable;
    LiteralValue value;

    for (const auto &assignment: formula.assignments) {
        if (assignment.second == U) {
            const auto &occurrences = formula.occurrences.at(assignment.first);
            double positive_j = 0;
            for (const auto &positive_occurrence: occurrences.first) {
                positive_j += std::pow(2, -formula.clauses.at(positive_occurrence).size());
            }
            if (positive_j > max_j) {
                max_j = positive_j;
                variable = assignment.first;
                value = T;
            }
            double negative_j = 0;
            for (const auto &negative_occurrence: occurrences.second) {
                negative_j += std::pow(2, -formula.clauses.at(negative_occurrence).size());
            }
            if (negative_j > max_j) {
                max_j = negative_j;
                variable = assignment.first;
                value = F;
            }
        }
    }

    return {variable, value};
}
