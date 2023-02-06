#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "utils/types.h"

// Forward-declaration
class BranchingStrategy;

std::vector<std::string> split(const std::string &str, char delim);

class Formula {
public:
    std::vector<std::unordered_set<Literal>> clauses;
    std::unordered_map<Variable, LiteralValue> assignments;
    size_t variables_count = 0;
    size_t clauses_count = 0;

    Formula() = default;

    void set_variables_count(size_t n);

    void set_clauses_count(size_t n);

    void add_clause(const std::string &clause_str);

    // Returns 0 if there are no unit clauses in the formula
    Literal find_unit_clause();

    // We are assuming that the literal appears in some unit clause
    void propagate_unit_clause(Literal literal);

    // Returns 0 if there are no pure literals in the formula
    Literal find_pure_literal();

    // We are assuming that the literal appears only in this form
    void eliminate_pure_literal(Literal literal);

    void print();

    bool solve(const BranchingStrategy &branching_strategy);
};
