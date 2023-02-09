#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

// Forward-declaration
class BranchingStrategy;

enum ComparisonResult {
    Equal, Greater, Less
};

typedef long Literal;
typedef long Variable;

enum LiteralValue {
    U, T, F
};

struct Clause {
    bool active;
    long active_literals;
    std::unordered_set<Literal> literals;
    Variable deactivated_by;
};

struct VariableData {
    LiteralValue value;
    std::vector<Clause *> negative_occurrences;
    std::vector<Clause *> positive_occurrences;
};

std::vector<std::string> split(const std::string &str, char delim);

class Formula {
public:
    std::vector<Clause *> clauses;
    std::unordered_map<Variable, VariableData> variables;

    size_t variables_count = 0;
    size_t clauses_count = 0;

    Formula() = default;

    void set_variables_count(size_t n);

    void set_clauses_count(size_t n);

    void add_clause(const std::string &clause_str);

    // Returns 0 if there are no unit clauses in the formula
    Literal find_unit_clause();

    // Returns 0 if there are no pure literals in the formula
    Literal find_pure_literal();

    // We are assuming that the literal appears in some unit clause
    void propagate(Literal literal);

    void depropagate(const std::vector<Variable> &eliminated_variables);

    void print();

    std::pair<bool, std::vector<Variable>> solve(const BranchingStrategy &branching_strategy);
};
