#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

#include "Formula.h"
#include "strategies/branching/BranchingStrategy.h"

#define DEBUG true

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
    variables.reserve(n);
}

void Formula::set_clauses_count(size_t n) {
    clauses.reserve(n);
}

void Formula::add_clause(const std::vector<long> &clause_vec) {
    auto *clause = new Clause{true, nullptr, 0, {}};
    for (const long &new_literal: clause_vec) {
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
                auto *variable = new Variable{std::abs(new_literal), U, {}, {}, 0};
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
    active_clauses++;
    clauses.push_back(clause);
}

void Formula::add_clause(const std::string &clause_str) {
    std::vector<std::string> clause_str_vec = split(clause_str, ' ');
    std::vector<long> clause_vec;
    for (const auto &str_literal: clause_str_vec) {
        if (str_literal != "0") {
            clause_vec.push_back(std::stol(str_literal));
        } else {
            add_clause(clause_vec);
            break;
        }
    }

}

// Returns a stub pair if no unit clauses have been found.
std::pair<SignedVariable, Clause *> Formula::find_unit_clause() {
    for (const auto &clause: clauses) {
        // If an active clause has only one active literal.
        if (clause->active && clause->active_literals == 1) {
            for (const auto &literal: clause->literals) {
                // Find an unassigned variable in the clause.
                if (literal.variable->value == U) {
                    return {literal, clause};
                }
            }
            // We should have found at least one unassigned variable.
            assert(false);
        }
    }
    return {{0, nullptr}, nullptr};
}

void Formula::add_learned_clause(const std::vector<SignedVariable> &clause_vec) {
    auto *clause = new Clause{true, nullptr, 0, {}};
    for (const SignedVariable &new_literal: clause_vec) {
        if (new_literal.sign > 0) {
            new_literal.variable->positive_occurrences.push_back(clause);
        } else {
            new_literal.variable->negative_occurrences.push_back(clause);
        }
        // Variables in a learned clause can't be unassigned at the time of adding.
        assert(new_literal.variable->value != U);
        new_literal.variable->vsids_score++;
        clause->literals.push_back(new_literal);
    }
    active_clauses++;
    clauses.push_back(clause);
}

void Formula::register_conflict(SignedVariable literal, Clause *implicated_by) {
    std::vector<SignedVariable> cut;

    if (implicated_by != nullptr) {
        for (const auto &implicated_by_literal: implicated_by->literals) {
            if (implicated_by_literal.variable->id != literal.variable->id) {
                if (std::find_if(cut.begin(), cut.end(),
                                 [&](const auto &cut_literal) {
                                     return cut_literal.variable->id == implicated_by_literal.variable->id;
                                 }) == cut.end()) {
                    cut.push_back(implicated_by_literal);
                }
            }
        }
    }

    auto opposite_choice = std::find_if(choices.rbegin(), choices.rend(),
                                        [&](const auto &choice) {
                                            return choice.literal.variable->id == literal.variable->id;
                                        });

    // Should have made an opposite choice sometime along the line.
    assert(opposite_choice != choices.rend());

    if ((*opposite_choice).implicated_by != nullptr) {
        for (const auto &implicated_by_literal: (*opposite_choice).implicated_by->literals) {
            if (implicated_by_literal.variable->id != literal.variable->id) {
                if (std::find_if(cut.begin(), cut.end(),
                                 [&](const auto &cut_literal) {
                                     return cut_literal.variable->id == implicated_by_literal.variable->id;
                                 }) == cut.end()) {
                    cut.push_back(implicated_by_literal);
                }
            }
        }
    }

    add_learned_clause(cut);
    new_conflicts++;
}

// We are assuming that the literal appears in some unit clause
bool Formula::propagate(SignedVariable literal, Clause *implicated_by, bool retry) {
    // Can't propagate an empty variable.
    assert(literal.sign != 0);
    assert(literal.variable != nullptr);

    choices.push_back({retry, literal, implicated_by});

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
            active_clauses--;
        }
    }

    for (const auto &decrement_clause: decrement_clauses) {
        if (decrement_clause->active) {
            decrement_clause->active_literals--;
        }
    }

    for (const auto &decrement_clause: decrement_clauses) {
        if (decrement_clause->active && decrement_clause->active_literals == 0) {
            register_conflict(literal, decrement_clause);
            return false;
        }
    }

    // Propagation chaining.
    for (const auto &decrement_clause: decrement_clauses) {
        if (decrement_clause->active && decrement_clause->active_literals == 1) {
            for (const auto &maybe_unit: decrement_clause->literals) {
                if (maybe_unit.variable->value == U) {
                    if (!propagate(maybe_unit, decrement_clause, false)) {
                        return false;
                    }
                    break;
                }
            }
        }
    }

    return true;
}

bool Formula::backtrack() {
    // Depropagate all non-retry choices
    while (!choices.empty() && !choices.back().retry) {
        Choice last_choice = choices.back();
        depropagate(last_choice.literal);
        choices.pop_back();
    }
    // Check whether we are done with choices
    if (choices.empty()) {
        return false;
    } else {
        Choice last_choice = choices.back();
        // We should not be able to retry an implicated clause
        assert(last_choice.implicated_by == nullptr);
        depropagate(last_choice.literal);
        SignedVariable literal = {-last_choice.literal.sign, last_choice.literal.variable};
        choices.pop_back();
        // Try to propagate the opposite of the clause
        if (!propagate(literal, nullptr, false)) {
            return backtrack();
        }
        return true;
    }
}

bool Formula::solve(const BranchingStrategy &branching_strategy) {
    while (true) {
        // Randomized restarts with a geometric sequence.
        if (new_conflicts > conflict_ceiling) {
            while (!choices.empty()) {
                Choice last_choice = choices.back();
                depropagate(last_choice.literal);
                choices.pop_back();
            }
            new_conflicts = 0;
            conflict_ceiling *= 2;
        }
        // Perform Unit Propagation
        while (true) {
            std::pair<SignedVariable, Clause *> unit = find_unit_clause();
            // If we have found something
            if (unit.first.variable != nullptr) {
                if (!propagate(unit.first, unit.second, false)) {
                    if (!backtrack()) {
                        return false;
                    }
                }
            } else {
                break;
            }
        }
        // Check for whether no clauses are active
        if (active_clauses == 0) {
            return true;
        }

        // Choose a variable to branch on
        const std::pair<Variable *, LiteralValue> assignment = branching_strategy.choose(*this);
        SignedVariable literal{assignment.second == T ? 1 : -1, assignment.first};
        if (!propagate(literal, nullptr, true)) {
            if (!backtrack()) {
                return false;
            }
        }
    }
}

void Formula::depropagate(const SignedVariable &eliminated_literal) {
    const auto &activate_clauses =
            eliminated_literal.variable->value == T ? eliminated_literal.variable->positive_occurrences :
            eliminated_literal.variable->negative_occurrences;

    const auto &increment_clauses =
            eliminated_literal.variable->value == T ? eliminated_literal.variable->negative_occurrences :
            eliminated_literal.variable->positive_occurrences;

    for (const auto &activate_clause: activate_clauses) {
        if (activate_clause->deactivated_by == eliminated_literal.variable) {
            activate_clause->active = true;
            activate_clause->deactivated_by = nullptr;
            active_clauses++;
        }
    }

    for (const auto &increment_clause: increment_clauses) {
        if (increment_clause->active) {
            increment_clause->active_literals++;
        }
    }

    eliminated_literal.variable->value = U;
}

Formula::~Formula() {
    for (auto &variable: variables) {
        delete variable;
    }
    for (auto &clause: clauses) {
        delete clause;
    }
}
