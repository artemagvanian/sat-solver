#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

#include "Formula.h"
#include "strategies/branching/BranchingStrategy.h"

std::vector<std::string> split(const std::string &str, char delim) {
    std::vector<std::string> items;
    std::stringstream str_stream(str);
    std::string item;
    while (std::getline(str_stream, item, delim)) {
        items.push_back(item);
    }
    return items;
}

void Formula::set_variables_count(size_t n) {
    variables_count = n;
    variables.reserve(n);
}

void Formula::set_clauses_count(size_t n) {
    clauses_count = n;
    clauses.reserve(n);
}

void Formula::add_clause(const std::string &clause_str) {
    std::vector<std::string> clause_vec = split(clause_str, ' ');

    auto *clause = new Clause{true, nullptr, 0, {}};

    for (const std::string &str_literal: clause_vec) {
        if (str_literal == "0") {
            assert(clause->literals.size() == clause->active_literals);
            clauses.push_back(clause);
            break;
        } else {
            long new_literal = stol(str_literal);
            assert(new_literal != 0);

            if (std::find_if(clause->literals.cbegin(), clause->literals.cend(),
                             [&](const auto &literal) {
                                 return literal.variable->id == std::abs(new_literal);
                             }) == clause->literals.end()) {

                auto it = std::find_if(variables.begin(), variables.end(),
                                       [&](const auto &variable) {
                                           return variable->id == std::abs(new_literal);
                                       });

                if (it == variables.end()) {
                    auto *variable = new Variable{std::abs(new_literal), U, {}, {}};
                    if (new_literal > 0) {
                        variable->positive_occurrences.push_back(clause);
                        clause->literals.push_back({1, variable});
                    } else {
                        variable->negative_occurrences.push_back(clause);
                        clause->literals.push_back({-1, variable});
                    }
                    variables.push_back(variable);
                } else {
                    if (new_literal > 0) {
                        (*it)->positive_occurrences.push_back(clause);
                        clause->literals.push_back({1, (*it)});
                    } else {
                        (*it)->negative_occurrences.push_back(clause);
                        clause->literals.push_back({-1, (*it)});
                    }
                }
                clause->active_literals++;
            }
        }
    }
}

// Returns 0 if there are no unit clauses in the formula
SignedVariable Formula::find_unit_clause() {
    for (const auto &clause: clauses) {
        if (clause->active && clause->active_literals == 1) {
            for (const auto &literal: clause->literals) {
                if (literal.variable->value == U) {
                    return literal;
                }
            }
            assert(false);
        }
    }
    return {0, nullptr};
}

// We are assuming that the literal appears in some unit clause
void Formula::propagate(SignedVariable literal) {
    assert(literal.sign != 0);
    assert(literal.variable != nullptr);

    if (literal.sign > 0) {
        literal.variable->value = T;
    } else {
        literal.variable->value = F;
    }

    const auto &deactivate_clauses =
            literal.sign > 0 ? literal.variable->positive_occurrences :
            literal.variable->negative_occurrences;

    const auto &decrement_clauses =
            literal.sign > 0 ? literal.variable->negative_occurrences :
            literal.variable->positive_occurrences;

    for (const auto &deactivate_clause: deactivate_clauses) {
        if (deactivate_clause->active) {
            deactivate_clause->active = false;
            deactivate_clause->deactivated_by = literal.variable;
        }
    }
    for (const auto &decrement_clause: decrement_clauses) {
        if (decrement_clause->active) {
            decrement_clause->active_literals--;
        }
    }
}

// Returns 0 if there are no pure literals in the formula
SignedVariable Formula::find_pure_literal() {
    for (const auto &variable: variables) {
        if (variable->value == U) {
            bool no_positives = std::all_of(variable->positive_occurrences.cbegin(),
                                            variable->positive_occurrences.cend(),
                                            [](const auto &clause) { return !(clause->active); });

            bool no_negatives = std::all_of(variable->negative_occurrences.cbegin(),
                                            variable->negative_occurrences.cend(),
                                            [](const auto &clause) { return !(clause->active); });
            if (!no_positives && no_negatives) {
                return {1, variable};
            } else if (!no_negatives && no_positives) {
                return {-1, variable};
            }
        }
    }
    return {0, nullptr};
}

std::pair<bool, std::vector<Variable *>> Formula::solve(const BranchingStrategy &branching_strategy) {
    std::vector<Variable *> eliminated_variables;

    SignedVariable unit_clause{};
    while ((unit_clause = find_unit_clause()).variable != nullptr) {
        propagate(unit_clause);
        eliminated_variables.push_back(unit_clause.variable);
    }

    SignedVariable pure_literal{};
    while ((pure_literal = find_pure_literal()).variable != nullptr) {
        propagate(pure_literal);
        eliminated_variables.push_back(pure_literal.variable);
    }

    if (std::all_of(clauses.cbegin(),
                    clauses.cend(),
                    [](const auto &clause) { return !clause->active; })) {
        return {true, eliminated_variables};
    } else if (std::find_if(clauses.cbegin(), clauses.cend(),
                            [](const auto &clause) {
                                return clause->active_literals == 0;
                            }) != clauses.end()) {
        return {false, eliminated_variables};
    }

    const std::pair<Variable *, LiteralValue> assignment = branching_strategy.choose(*this);

    SignedVariable literal{0, assignment.first};
    literal.sign = assignment.second == T ? 1 : -1;
    propagate(literal);

    std::pair<bool, std::vector<Variable *>> branch_left = solve(branching_strategy);

    if (branch_left.first) {
        eliminated_variables.push_back(literal.variable);
        eliminated_variables.insert(eliminated_variables.end(),
                                    std::make_move_iterator(branch_left.second.begin()),
                                    std::make_move_iterator(branch_left.second.end()));
        return {true, eliminated_variables};
    } else {
        depropagate(branch_left.second);
        depropagate({literal.variable});

        literal.sign *= -1;
        propagate(literal);

        std::pair<bool, std::vector<Variable *>> branch_right = solve(branching_strategy);

        if (branch_right.first) {
            eliminated_variables.push_back(literal.variable);
            eliminated_variables.insert(eliminated_variables.end(),
                                        std::make_move_iterator(branch_right.second.begin()),
                                        std::make_move_iterator(branch_right.second.end()));
            return {true, eliminated_variables};
        } else {
            depropagate(branch_right.second);
            depropagate({literal.variable});

            return {false, eliminated_variables};
        }
    }
}

void Formula::depropagate(const std::vector<Variable *> &eliminated_variables) {
    for (auto it = eliminated_variables.crbegin(); it != eliminated_variables.crend(); it++) {
        const auto &activate_clauses =
                (*it)->value == T ? (*it)->positive_occurrences :
                (*it)->negative_occurrences;

        const auto &increment_clauses =
                (*it)->value == T ? (*it)->negative_occurrences :
                (*it)->positive_occurrences;

        for (const auto &activate_clause: activate_clauses) {
            if (activate_clause->deactivated_by == *it) {
                activate_clause->active = true;
                activate_clause->deactivated_by = nullptr;
            }
        }
        for (const auto &increment_clause: increment_clauses) {
            if (increment_clause->active) {
                increment_clause->active_literals++;
            }
        }

        (*it)->value = U;
    }
}

Formula::~Formula() {
    for (auto &variable: variables) {
        delete variable;
    }
    for (auto &clause: clauses) {
        delete clause;
    }
}
