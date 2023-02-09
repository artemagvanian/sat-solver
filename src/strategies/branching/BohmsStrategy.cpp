#include "BohmsStrategy.h"

#include <functional>
#include <map>

// Stores positive and negative occurrences of some variable
typedef size_t ClauseSize;
typedef std::pair<size_t, size_t> Occurrences;

long big_h(long alpha, long beta, size_t h_x, size_t h_neg_x) {
    return alpha * static_cast<long>(std::max(h_x, h_neg_x)) + beta * static_cast<long>(std::min(h_x, h_neg_x));
}

bool lexicographical_comp(const std::map<ClauseSize, Occurrences> &a,
                          const std::map<ClauseSize, Occurrences> &b,
                          const std::function<ComparisonResult(Occurrences, Occurrences)> &comp) {
    auto it_a = a.begin(), it_b = b.begin();

    for (; it_a != a.end() && it_b != b.end(); it_a++, it_b++) {
        long comp_result = comp(it_a->second, it_b->second);
        if (comp_result == Less) {
            return true;
        } else if (comp_result == Greater) {
            return false;
        }
    }

    if (it_a == a.end()) {
        if (it_b == b.end()) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool h_values_comp(long alpha, long beta,
                   const std::map<ClauseSize, Occurrences> &h_values_a,
                   const std::map<ClauseSize, Occurrences> &h_values_b) {
    return lexicographical_comp(h_values_a, h_values_b,
                                [&](const auto &a, const auto &b) {
                                    long big_h_a = big_h(alpha, beta, a.first, a.second);
                                    long big_h_b = big_h(alpha, beta, b.first, b.second);
                                    if (big_h_a < big_h_b) {
                                        return Less;
                                    } else if (big_h_a > big_h_b) {
                                        return Greater;
                                    } else {
                                        return Equal;
                                    }
                                });
}

// Implementing Bohm's heuristic
std::pair<Variable *, LiteralValue> BohmsStrategy::choose(const Formula &initial) const {
    std::unordered_map<ID, std::map<ClauseSize, Occurrences>> h_values;

    for (const auto &clause: initial.clauses) {
        for (const auto &literal: clause->literals) {
            assert(literal.variable != nullptr);
            if (literal.sign > 0) {
                h_values[literal.variable->id][clause->literals.size()].first++;
            } else if (literal.sign < 0) {
                h_values[literal.variable->id][clause->literals.size()].second++;
            }
        }
    }
    auto max = *std::max_element(h_values.begin(), h_values.end(),
                                 [this](const auto &h_values_a,
                                        const auto &h_values_b) {
                                     return h_values_comp(alpha, beta, h_values_a.second, h_values_b.second);
                                 });


    size_t sum_positive = 0, sum_negative = 0;
    for (const auto &pair: max.second) {
        sum_positive += pair.second.first;
        sum_positive += pair.second.second;
    }

    auto variable = std::find_if(initial.variables.begin(), initial.variables.end(),
                                 [&](const auto &variable) {
                                     return variable->id == max.first;
                                 });

    return {*variable, sum_positive > sum_negative ? T : F};
}