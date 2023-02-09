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

    auto *clause = new Clause{true, 0, {}, 0};

    for (const std::string &str_literal: clause_vec) {
        if (str_literal == "0") {
            assert(clause->literals.size() == clause->active_literals);
            clauses.push_back(clause);
            break;
        } else {
            Literal literal = stol(str_literal);
            assert(literal != 0);

            if (clause->literals.find(literal) == clause->literals.end()) {
                if (literal > 0) {
                    variables[std::abs(literal)].positive_occurrences.push_back(clause);
                } else {
                    variables[std::abs(literal)].negative_occurrences.push_back(clause);
                }
                variables[std::abs(literal)].value = U;

                clause->literals.insert(literal);
                clause->active_literals++;
            }
        }
    }
}

// Returns 0 if there are no unit clauses in the formula
Literal Formula::find_unit_clause() {
    for (const auto &clause: clauses) {
        if (clause->active && clause->active_literals == 1) {
            for (const auto &literal: clause->literals) {
                if (variables[std::abs(literal)].value == U) {
                    return literal;
                }
            }
            assert(false);
        }
    }
    return 0;
}

// We are assuming that the literal appears in some unit clause
void Formula::propagate(Literal literal) {
    assert(literal != 0);

    if (literal > 0) {
        variables[literal].value = T;
    } else {
        variables[-literal].value = F;
    }

    const auto &deactivate_clauses =
            literal > 0 ? variables[std::abs(literal)].positive_occurrences :
            variables[std::abs(literal)].negative_occurrences;

    const auto &decrement_clauses =
            literal > 0 ? variables[std::abs(literal)].negative_occurrences :
            variables[std::abs(literal)].positive_occurrences;

    for (const auto &deactivate_clause: deactivate_clauses) {
        if (deactivate_clause->active) {
            deactivate_clause->active = false;
            deactivate_clause->deactivated_by = std::abs(literal);
        }
    }
    for (const auto &decrement_clause: decrement_clauses) {
        if (decrement_clause->active) {
            decrement_clause->active_literals--;
        }
    }
}

// Returns 0 if there are no pure literals in the formula
Literal Formula::find_pure_literal() {
    for (const auto &variable_data_pair: variables) {
        if (variable_data_pair.second.value == U) {
            bool no_positives = std::all_of(variable_data_pair.second.positive_occurrences.cbegin(),
                                            variable_data_pair.second.positive_occurrences.cend(),
                                            [](const auto &clause) { return !(clause->active); });

            bool no_negatives = std::all_of(variable_data_pair.second.negative_occurrences.cbegin(),
                                            variable_data_pair.second.negative_occurrences.cend(),
                                            [](const auto &clause) { return !(clause->active); });
            if (!no_positives && no_negatives) {
                return variable_data_pair.first;
            } else if (!no_negatives && no_positives) {
                return -variable_data_pair.first;
            }
        }
    }
    return 0;
}

void Formula::print() {
    std::cout << "FORMULA WITH " << variables_count << " VARS AND " << clauses_count << " CLAUSES" << std::endl;
    std::cout << "CLAUSES: ";
    for (const auto &clause: clauses) {
        std::cout << "(";
        for (const Literal &literal: clause->literals) {
            std::cout << literal << ",";
        }
        std::cout << ") ";
    }

    std::cout << std::endl << "ASSIGNMENTS: ";
    for (const auto &variable_data_pair: variables) {
        const Variable &variable = variable_data_pair.first;
        const LiteralValue &value = variable_data_pair.second.value;

        std::cout << variable << " = ";
        if (value != U) {
            std::cout << (value == T ? "T" : "F") << "; ";
        } else {
            std::cout << "?; ";
        }
    }
}

std::pair<bool, std::vector<Variable>> Formula::solve(const BranchingStrategy &branching_strategy) {
    std::vector<Variable> eliminated_variables;

    Literal unit_clause;
    while ((unit_clause = find_unit_clause()) != 0) {
        propagate(unit_clause);
        eliminated_variables.push_back(std::abs(unit_clause));
    }

    Literal pure_literal;
    while ((pure_literal = find_pure_literal()) != 0) {
        propagate(pure_literal);
        eliminated_variables.push_back(std::abs(pure_literal));
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

    const std::pair<Variable, LiteralValue> assignment = branching_strategy.choose(*this);
    const Variable &variable = assignment.first;
    const LiteralValue &value = assignment.second;

    Literal literal = value == T ? variable : -variable;
    propagate(literal);

    std::pair<bool, std::vector<Variable>> branch_left = solve(branching_strategy);

    if (branch_left.first) {
        eliminated_variables.push_back(std::abs(literal));
        eliminated_variables.insert(eliminated_variables.end(),
                                    std::make_move_iterator(branch_left.second.begin()),
                                    std::make_move_iterator(branch_left.second.end()));
        return {true, eliminated_variables};
    } else {
        depropagate(branch_left.second);
        depropagate({std::abs(literal)});

        propagate(-literal);

        std::pair<bool, std::vector<Variable>> branch_right = solve(branching_strategy);

        if (branch_right.first) {
            eliminated_variables.push_back(std::abs(literal));
            eliminated_variables.insert(eliminated_variables.end(),
                                        std::make_move_iterator(branch_right.second.begin()),
                                        std::make_move_iterator(branch_right.second.end()));
            return {true, eliminated_variables};
        } else {
            depropagate(branch_right.second);
            depropagate({std::abs(literal)});

            return {false, eliminated_variables};
        }
    }
}

void Formula::depropagate(const std::vector<Variable> &eliminated_variables) {
    for (auto it = eliminated_variables.crbegin(); it != eliminated_variables.crend(); it++) {
        LiteralValue value = variables[*it].value;

        const auto &activate_clauses =
                value == T ? variables[*it].positive_occurrences :
                variables[*it].negative_occurrences;

        const auto &increment_clauses =
                value == T ? variables[*it].negative_occurrences :
                variables[*it].positive_occurrences;

        for (const auto &activate_clause: activate_clauses) {
            if (activate_clause->deactivated_by == *it) {
                activate_clause->active = true;
                activate_clause->deactivated_by = 0;
            }
        }
        for (const auto &increment_clause: increment_clauses) {
            if (increment_clause->active) {
                increment_clause->active_literals++;
            }
        }

        variables[*it].value = U;
    }
}
