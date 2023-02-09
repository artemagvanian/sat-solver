#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "utils/types.h"

// Forward-declaration
class BranchingStrategy;

std::vector<std::string> split(const std::string &str, char delim);

typedef long ID;

struct OperationResult {
    Literal set_literal;
    std::vector<std::pair<ID, std::unordered_set<Literal>>> removed_clauses;
    std::vector<ID> reduced_clauses;
};

struct SolutionResult {
    bool result;
    std::vector<OperationResult> ops;
};

class Formula {
public:
    std::unordered_map<ID, std::unordered_set<Literal>> clauses;
    std::unordered_map<Variable, LiteralValue> assignments;
    // Variable -> { positive_occurrences, negative_occurrences }
    std::unordered_map<Variable, std::pair<std::unordered_set<ID>, std::unordered_set<ID>>> occurrences;

    size_t variables_count = 0;
    size_t clauses_count = 0;
    ID next_assigned_id = 1;

    Formula() = default;

    void set_variables_count(size_t n);

    void set_clauses_count(size_t n);

    void add_clause(const std::string &clause_str);

    // Returns 0 if there are no unit clauses in the formula
    Literal find_unit_clause();

    // We are assuming that the literal appears in some unit clause
    OperationResult propagate_unit_clause(Literal literal);

    // Returns 0 if there are no pure literals in the formula
    Literal find_pure_literal();

    // We are assuming that the literal appears only in this form
    OperationResult eliminate_pure_literal(Literal literal);

    void undo_operation(OperationResult &op);

    void undo_solution(SolutionResult &sol);

    void restore_clauses(std::vector<std::pair<ID, std::unordered_set<Literal>>> &removed_clauses);

    void restore_literal(const std::vector<ID> &reduced_clauses, Literal literal);

    void restore_literal_value(const Literal &literal);

    void print();

    SolutionResult solve(const BranchingStrategy &branching_strategy);
};
