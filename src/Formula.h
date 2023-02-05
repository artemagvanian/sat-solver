#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

// Forward-declaration
class BranchingStrategy;

enum LiteralValue {
    U, T, F
};

std::vector<std::string> split(const std::string &str, char delim);

class Formula {
public:
    std::vector<std::unordered_set<int>> clauses;
    std::unordered_map<int, LiteralValue> assignments;
    int variables_count = 0;
    int clauses_count = 0;

    Formula() = default;

    void set_variables_count(int n);

    void set_clauses_count(int n);

    void add_clause(const std::string &clause_str);

    // Returns 0 if there are no unit clauses in the formula
    int find_unit_clause();

    // We are assuming that the literal appears in some unit clause
    void propagate_unit_clause(int literal);

    // Returns 0 if there are no pure literals in the formula
    int find_pure_literal();

    // We are assuming that the literal appears only in this form
    void eliminate_pure_literal(int literal);

    void print();

    bool solve(const BranchingStrategy &branching_strategy);
};
