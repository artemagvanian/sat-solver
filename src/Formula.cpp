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

void Formula::set_variables_count(int n) {
    variables_count = n;
}

void Formula::set_clauses_count(int n) {
    clauses_count = n;
}

void Formula::add_clause(const std::string &clause_str) {
    std::vector<std::string> clause_vec = split(clause_str, ' ');

    std::unordered_set<int> clause;
    for (const std::string &literal: clause_vec) {
        if (literal == "0") {
            this->clauses.push_back(clause);
            break;
        } else {
            int int_literal = stoi(literal);
            clause.insert(int_literal);
            if (assignments.find(abs(int_literal)) == assignments.end()) {
                assignments.insert({abs(int_literal), {}});
            }
        }
    }
}

// Returns 0 if there are no unit clauses in the formula
int Formula::find_unit_clause() {
    for (const auto &clause: clauses) {
        if (clause.size() == 1) {
            return *clause.begin();
        }
    }
    return {};
}

// We are assuming that the literal appears in some unit clause
void Formula::propagate_unit_clause(int literal) {
    assert(literal != 0);

    if (literal < 0) {
        assignments[-literal] = F;
    } else {
        assignments[literal] = T;
    }

    // Remove all clauses including literal
    clauses.erase(std::remove_if(clauses.begin(), clauses.end(),
                                 [&](auto &clause) {
                                     return clause.find(literal) != clause.end();
                                 }), clauses.end());

    // Remove ~literal from other clauses
    for (auto &clause: clauses) {
        if (clause.find(-literal) != clause.end()) {
            clause.erase(-literal);
        }
    }
}

// Returns 0 if there are no pure literals in the formula
int Formula::find_pure_literal() {
    for (const auto &assignment: assignments) {
        const int &variable = assignment.first;
        const LiteralValue &value = assignment.second;

        bool pure = true;
        if (value == U) {
            int variable_instance = 0;
            for (const auto &clause: clauses) {
                if (variable_instance == 0) {
                    if (clause.find(variable) != clause.end()) {
                        variable_instance = variable;
                    } else if (clause.find(-variable) != clause.end()) {
                        variable_instance = -variable;
                    }
                } else {
                    if (clause.find(variable_instance) == clause.end()) {
                        pure = false;
                        break;
                    }
                }
            }

            if (variable_instance != 0 && pure) {
                return variable_instance;
            }
        }
    }
    return 0;
}

// We are assuming that the literal appears only in this form
void Formula::eliminate_pure_literal(int literal) {
    assert(literal != 0);

    if (literal < 0) {
        assignments[-literal] = F;
    } else {
        assignments[literal] = T;
    }

    // Remove all clauses including literal
    clauses.erase(std::remove_if(clauses.begin(), clauses.end(),
                                 [&](auto &clause) {
                                     return clause.find(literal) != clause.end();
                                 }), clauses.end());
}

void Formula::print() {
    std::cout << "FORMULA WITH " << variables_count << " VARS AND " << clauses_count << " CLAUSES" << std::endl;
    std::cout << "CLAUSES: ";
    for (const auto &clause: clauses) {
        std::cout << "(";
        for (const int &literal: clause) {
            std::cout << literal << ",";
        }
        std::cout << ") ";
    }

    std::cout << std::endl << "ASSIGNMENTS: ";
    for (const auto &assignment: assignments) {
        const int &variable = assignment.first;
        const LiteralValue &value = assignment.second;

        std::cout << variable << " = ";
        if (value != U) {
            std::cout << (value == T ? "T" : "F") << "; ";
        } else {
            std::cout << "?; ";
        }
    }
}

bool Formula::solve(const BranchingStrategy &branching_strategy) {
    int unit_clause;
    while ((unit_clause = this->find_unit_clause()) != 0) {
        this->propagate_unit_clause(unit_clause);
    }

    int pure_literal;
    while ((pure_literal = this->find_pure_literal()) != 0) {
        this->eliminate_pure_literal(pure_literal);
    }

    if (clauses.empty()) {
        return true;
    } else if (std::find_if(clauses.begin(), clauses.end(),
                            [](const auto &clause) {
                                return clause.empty();
                            }) != clauses.end()) {
        return false;
    }

    const std::pair<int, LiteralValue> assignment = branching_strategy.choose(*this);
    const int &variable = assignment.first;
    const LiteralValue &value = assignment.second;

    Formula branch_left = *this;
    branch_left.assignments[variable] = value;
    branch_left.propagate_unit_clause(value == T ? variable : -variable);

    if (branch_left.solve(branching_strategy)) {
        *this = branch_left;
        return true;
    } else {
        Formula branch_false = *this;
        branch_false.assignments[variable] = value == T ? F : T;
        branch_false.propagate_unit_clause(value == T ? -variable : variable);

        if (branch_false.solve(branching_strategy)) {
            *this = branch_false;
            return true;
        }
    }

    return false;
}

