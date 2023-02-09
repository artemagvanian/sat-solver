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
    assignments.reserve(n);
    occurrences.reserve(n);
}

void Formula::set_clauses_count(size_t n) {
    clauses_count = n;
    clauses.reserve(n);
}

void Formula::add_clause(const std::string &clause_str) {
    std::vector<std::string> clause_vec = split(clause_str, ' ');

    std::unordered_set<Literal> literals;
    for (const std::string &str_literal: clause_vec) {
        if (str_literal == "0") {
            clauses[next_assigned_id] = std::move(literals);
            next_assigned_id++;
            break;
        } else {
            Literal literal = stol(str_literal);
            assert(literal != 0);
            if (literal > 0) {
                occurrences[std::abs(literal)].first.insert(next_assigned_id);
            } else {
                occurrences[std::abs(literal)].second.insert(next_assigned_id);
            }
            literals.insert(literal);
            assignments[std::abs(literal)] = U;
        }
    }
}

// Returns 0 if there are no unit clauses in the formula
Literal Formula::find_unit_clause() {
    for (const auto &clause: clauses) {
        if (clause.second.size() == 1) {
            return *(clause.second.begin());
        }
    }
    return 0;
}

// We are assuming that the literal appears in some unit clause
OperationResult Formula::propagate_unit_clause(Literal literal) {
    assert(literal != 0);

    if (literal > 0) {
        assignments[literal] = T;
    } else {
        assignments[-literal] = F;
    }

    std::vector<std::pair<ID, std::unordered_set<Literal>>> removed_clauses;
    std::vector<ID> reduced_clauses;

    const auto &remove_occurrences =
            literal > 0 ? occurrences[std::abs(literal)].first : occurrences[std::abs(literal)].second;
    const auto &reduce_occurrences =
            literal > 0 ? occurrences[std::abs(literal)].second : occurrences[std::abs(literal)].first;

    removed_clauses.reserve(remove_occurrences.size());
    reduced_clauses.reserve(reduce_occurrences.size());

    for (const auto &remove_occurrence: remove_occurrences) {
        removed_clauses.emplace_back(remove_occurrence, std::move(clauses[remove_occurrence]));
        clauses.erase(remove_occurrence);
    }

    for (const auto &removed_clause: removed_clauses) {
        for (const auto &removed_literal: removed_clause.second) {
            if (removed_literal > 0) {
                occurrences[std::abs(removed_literal)].first.erase(removed_clause.first);
            } else {
                occurrences[std::abs(removed_literal)].second.erase(removed_clause.first);
            }
        }
    }

    for (const auto &reduce_occurrence: reduce_occurrences) {
        clauses[reduce_occurrence].erase(-literal);
        reduced_clauses.push_back(reduce_occurrence);
    }

    occurrences[std::abs(literal)].first.clear();
    occurrences[std::abs(literal)].second.clear();

    return {literal, removed_clauses, reduced_clauses};
}

// Returns 0 if there are no pure literals in the formula
Literal Formula::find_pure_literal() {
    for (const auto &occurrence: occurrences) {
        if (occurrence.second.second.empty() && !occurrence.second.first.empty()) {
            return occurrence.first;
        } else if (occurrence.second.first.empty() && !occurrence.second.second.empty()) {
            return -occurrence.first;
        }
    }
    return 0;
}

// We are assuming that the literal appears only in this form
OperationResult Formula::eliminate_pure_literal(Literal literal) {
    assert(literal != 0);

    if (literal > 0) {
        assignments[literal] = T;
    } else {
        assignments[-literal] = F;
    }

    std::vector<std::pair<ID, std::unordered_set<Literal>>> removed_clauses;

    if (literal > 0) {
        for (const auto &positive_occurrence: occurrences[std::abs(literal)].first) {
            removed_clauses.emplace_back(positive_occurrence, std::move(clauses[positive_occurrence]));
            clauses.erase(positive_occurrence);
        }
    } else {
        for (const auto &negative_occurrence: occurrences[std::abs(literal)].second) {
            removed_clauses.emplace_back(negative_occurrence, std::move(clauses[negative_occurrence]));
            clauses.erase(negative_occurrence);
        }
    }

    for (const auto &removed_clause: removed_clauses) {
        for (const auto &removed_literal: removed_clause.second) {
            if (removed_literal > 0) {
                occurrences[std::abs(removed_literal)].first.erase(removed_clause.first);
            } else {
                occurrences[std::abs(removed_literal)].second.erase(removed_clause.first);
            }
        }
    }

    occurrences[std::abs(literal)].first.clear();
    occurrences[std::abs(literal)].second.clear();

    return {literal, removed_clauses, {}};
}

void Formula::print() {
    std::cout << "FORMULA WITH " << variables_count << " VARS AND " << clauses_count << " CLAUSES" << std::endl;
    std::cout << "CLAUSES: ";
    for (const auto &clause: clauses) {
        std::cout << "(";
        for (const Literal &literal: clause.second) {
            std::cout << literal << ",";
        }
        std::cout << ") ";
    }

    std::cout << std::endl << "ASSIGNMENTS: ";
    for (const auto &assignment: assignments) {
        const Variable &variable = assignment.first;
        const LiteralValue &value = assignment.second;

        std::cout << variable << " = ";
        if (value != U) {
            std::cout << (value == T ? "T" : "F") << "; ";
        } else {
            std::cout << "?; ";
        }
    }
}

SolutionResult Formula::solve(const BranchingStrategy &branching_strategy) {
    std::vector<OperationResult> ops;

    Literal unit_clause;
    while ((unit_clause = find_unit_clause()) != 0) {
        OperationResult op = propagate_unit_clause(unit_clause);
        ops.push_back(std::move(op));
    }

    Literal pure_literal;
    while ((pure_literal = find_pure_literal()) != 0) {
        OperationResult op = eliminate_pure_literal(pure_literal);
        ops.push_back(std::move(op));
    }

    if (clauses.empty()) {
        return {true, ops};
    } else if (std::find_if(clauses.cbegin(), clauses.cend(),
                            [](const auto &clause) {
                                return clause.second.empty();
                            }) != clauses.end()) {
        return {false, ops};
    }

    const std::pair<Variable, LiteralValue> assignment = branching_strategy.choose(*this);
    const Variable &variable = assignment.first;
    const LiteralValue &value = assignment.second;

    Literal literal = value == T ? variable : -variable;
    OperationResult op = propagate_unit_clause(literal);
    SolutionResult sol = solve(branching_strategy);

    if (sol.result) {
        ops.push_back(std::move(op));
        ops.insert(ops.end(), std::make_move_iterator(sol.ops.begin()),
                   std::make_move_iterator(sol.ops.end()));
        return {true, ops};
    } else {
        undo_solution(sol);
        undo_operation(op);

        OperationResult anti_op = propagate_unit_clause(-literal);
        SolutionResult anti_sol = solve(branching_strategy);

        if (anti_sol.result) {
            ops.push_back(std::move(anti_op));
            ops.insert(ops.end(), std::make_move_iterator(anti_sol.ops.begin()),
                       std::make_move_iterator(anti_sol.ops.end()));
            return {true, ops};
        } else {
            undo_solution(anti_sol);
            undo_operation(anti_op);
            return {false, ops};
        }
    }
}

void Formula::restore_clauses(std::vector<std::pair<ID, std::unordered_set<Literal>>> &removed_clauses) {
    for (const auto &removed_clause: removed_clauses) {
        for (const auto &literal: removed_clause.second) {
            assert(literal != 0);
            if (literal > 0) {
                occurrences[std::abs(literal)].first.insert(removed_clause.first);
            } else {
                occurrences[std::abs(literal)].second.insert(removed_clause.first);
            }
        }
    }

    clauses.insert(std::make_move_iterator(removed_clauses.begin()),
                   std::make_move_iterator(removed_clauses.end()));
}

void Formula::restore_literal(const std::vector<ID> &reduced_clauses, Literal literal) {
    for (const auto &reduced_clause: reduced_clauses) {
        clauses[reduced_clause].insert(literal);
        assert(literal != 0);
        if (literal > 0) {
            occurrences[std::abs(literal)].first.insert(reduced_clause);
        } else {
            occurrences[std::abs(literal)].second.insert(reduced_clause);
        }
    }
}

void Formula::restore_literal_value(const Literal &literal) {
    assignments[std::abs(literal)] = U;
}

void Formula::undo_operation(OperationResult &op) {
    restore_clauses(op.removed_clauses);
    restore_literal(op.reduced_clauses, -op.set_literal);
    restore_literal_value(op.set_literal);
}

void Formula::undo_solution(SolutionResult &sol) {
    for (auto it = sol.ops.rbegin(); it != sol.ops.rend(); it++) {
        undo_operation(*it);
    }
}







