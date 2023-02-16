#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

#include "Formula.h"
#include "strategies/branching/BranchingStrategy.h"

#define DEBUG false

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

void Formula::add_clause(const std::vector<long> &clause_vec, bool learned) {
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
                assert(!learned);
                auto *variable = new Variable{std::abs(new_literal), U, {}, {}, 0};
                if (new_literal > 0) {
                    variable->positive_occurrences.push_back(clause);
                    clause->literals.push_back({1, variable});
                } else {
                    variable->negative_occurrences.push_back(clause);
                    clause->literals.push_back({-1, variable});
                }
                variables.push_back(variable);
                clause->active_literals++;
            } else {
                if (learned) {
                    (*it)->vsids_score++;
                }
                if (new_literal > 0) {
                    (*it)->positive_occurrences.push_back(clause);
                    clause->literals.push_back({1, (*it)});
                } else {
                    (*it)->negative_occurrences.push_back(clause);
                    clause->literals.push_back({-1, (*it)});
                }
                if ((*it)->value == U) {
                    clause->active_literals++;
                }
            }
        }
    }
    clauses.push_back(clause);
}

void Formula::add_clause(const std::string &clause_str) {
    std::vector<std::string> clause_str_vec = split(clause_str, ' ');
    std::vector<long> clause_vec;
    for (const auto &str_literal: clause_str_vec) {
        if (str_literal != "0") {
            clause_vec.push_back(std::stol(str_literal));
        } else {
            add_clause(clause_vec, false);
            break;
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

void Formula::register_conflict(SignedVariable literal, Clause *implicated_by) {
    implication_graph.insert_vertex(literal.sign * literal.variable->id);
    for (const auto &implicated_by_literal: implicated_by->literals) {
        implication_graph.insert_edge_noloop(
                -implicated_by_literal.sign * implicated_by_literal.variable->id,
                literal.sign * literal.variable->id);
    }
    if (DEBUG) std::cout << "CONFLICT +-" << literal.variable->id << std::endl;
    std::vector<long> cut;
    for (const long &conflicting_literal: implication_graph.in_neighbors(literal.variable->id)) {
        cut.push_back(-conflicting_literal);
    }
    for (const long &conflicting_literal: implication_graph.in_neighbors(-literal.variable->id)) {
        cut.push_back(-conflicting_literal);
    }

    add_clause(cut, true);
    implication_graph.remove_vertex(literal.sign * literal.variable->id);
}

void Formula::add_to_implication_graph(SignedVariable literal, Clause *implicated_by) {
    if (DEBUG) std::cout << "ADD " << literal.sign * literal.variable->id << std::endl;
    implication_graph.insert_vertex(literal.sign * literal.variable->id);
    if (implicated_by != nullptr) {
        for (const auto &implicated_by_literal: implicated_by->literals) {
            if (implicated_by_literal.variable->id != literal.variable->id) {
                implication_graph.insert_edge(-implicated_by_literal.sign * implicated_by_literal.variable->id,
                                              literal.sign * literal.variable->id);
            }
        }
    }
}

void Formula::remove_from_implication_graph(SignedVariable literal) {
    assert(implication_graph.includes_vertex(literal.sign * literal.variable->id));
    if (DEBUG) std::cout << "REMOVE " << literal.sign * literal.variable->id << std::endl;
    implication_graph.remove_vertex(literal.sign * literal.variable->id);
}

// We are assuming that the literal appears in some unit clause
std::vector<SignedVariable> Formula::propagate(SignedVariable literal, Clause *implicated_by) {
    std::vector<SignedVariable> eliminated_variables = {literal};

    assert(literal.sign != 0);
    assert(literal.variable != nullptr);

    add_to_implication_graph(literal, implicated_by);

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

    for (size_t i = 0; i < deactivate_clauses.size(); i++) {
        if (deactivate_clauses[i]->active) {
            deactivate_clauses[i]->active = false;
            deactivate_clauses[i]->deactivated_by = literal.variable;
        }
    }

    for (size_t i = 0; i < decrement_clauses.size(); i++) {
        if (decrement_clauses[i]->active) {
            decrement_clauses[i]->active_literals--;
        }
    }

    bool found_conflicts = false;
    for (size_t i = 0; i < decrement_clauses.size(); i++) {
        if (decrement_clauses[i]->active && decrement_clauses[i]->active_literals == 0) {
            register_conflict({-literal.sign, literal.variable},
                              decrement_clauses[i]);
            found_conflicts = true;
            break;
        }
    }
    if (found_conflicts) {
        return eliminated_variables;
    }

    for (size_t i = 0;
         i < decrement_clauses.size(); i++) {
        if (decrement_clauses[i]->active && decrement_clauses[i]->active_literals == 1) {
            bool propagated = false;
            for (const auto &maybe_unit: decrement_clauses[i]->literals) {
                if (maybe_unit.variable->value == U) {
                    std::vector<SignedVariable> propagated_variables = propagate(maybe_unit, decrement_clauses[i]);
                    eliminated_variables.insert(eliminated_variables.end(),
                                                std::make_move_iterator(propagated_variables.begin()),
                                                std::make_move_iterator(propagated_variables.end()));
                    propagated = true;
                    break;
                }
            }
            assert(propagated);
        }
    }
    return eliminated_variables;
}

struct Choice {
    bool tried_other_branch;
    SignedVariable literal;
    std::vector<SignedVariable> propagates;
};

bool Formula::solve_inner(const BranchingStrategy &branching_strategy) {
    std::vector<Choice> choices;
    while (true) {
        if (std::all_of(clauses.cbegin(),
                        clauses.cend(),
                        [](const auto &clause) { return !clause->active; })) {
            return true;
        } else if (std::find_if(clauses.cbegin(), clauses.cend(),
                                [](const auto &clause) {
                                    return clause->active_literals == 0;
                                }) != clauses.end()) {
            while (!choices.empty() && choices.back().tried_other_branch) {
                Choice last_choice = choices.back();
                choices.pop_back();
                depropagate(last_choice.propagates);
            }
            if (choices.empty()) {
                return false;
            } else {
                Choice last_choice = choices.back();
                choices.pop_back();
                depropagate(last_choice.propagates);

                SignedVariable literal = {-last_choice.literal.sign, last_choice.literal.variable};
                std::vector<SignedVariable> propagates = propagate(literal, nullptr);
                choices.push_back({true, literal, propagates});
                continue;
            }
        }

        const std::pair<Variable *, LiteralValue> assignment = branching_strategy.choose(*this);

        SignedVariable literal{assignment.second == T ? 1 : -1, assignment.first};
        std::vector<SignedVariable> propagates = propagate(literal, nullptr);
        choices.push_back({false, literal, propagates});
    }
}

void Formula::depropagate(const std::vector<SignedVariable> &eliminated_literals) {
    for (auto it = eliminated_literals.crbegin(); it != eliminated_literals.crend(); it++) {
        const auto &activate_clauses =
                (*it).variable->value == T ? (*it).variable->positive_occurrences :
                (*it).variable->negative_occurrences;

        const auto &increment_clauses =
                (*it).variable->value == T ? (*it).variable->negative_occurrences :
                (*it).variable->positive_occurrences;

        for (const auto &activate_clause: activate_clauses) {
            if (activate_clause->deactivated_by == (*it).variable) {
                activate_clause->active = true;
                activate_clause->deactivated_by = nullptr;
            }
        }
        for (const auto &increment_clause: increment_clauses) {
            if (increment_clause->active) {
                increment_clause->active_literals++;
            }
        }
        remove_from_implication_graph((*it));
        (*it).variable->value = U;
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

bool Formula::solve(const BranchingStrategy &branching_strategy) {
    SignedVariable unit_clause{};
    while ((unit_clause = find_unit_clause()).variable != nullptr) {
        propagate(unit_clause, nullptr);
    }
    auto result = solve_inner(branching_strategy);
    return result;
}
