#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

// Forward-declaration
class BranchingStrategy;

enum ComparisonResult {
    Equal, Greater, Less
};

typedef long ID;
typedef short Sign;

enum LiteralValue {
    U, T, F
};


struct Clause;
struct Variable;

struct SignedVariable {
    Sign sign;
    Variable *variable;
};

struct Clause {
    bool active;
    Variable *deactivated_by;

    long active_literals;
    std::vector<SignedVariable> literals;
};

struct Variable {
    ID id;
    LiteralValue value;
    std::vector<Clause *> negative_occurrences;
    std::vector<Clause *> positive_occurrences;
};

std::vector<std::string> split(const std::string &str, char delim);

class Formula {
public:
    std::vector<Clause *> clauses;
    std::vector<Variable *> variables;

    size_t variables_count = 0;
    size_t clauses_count = 0;

    Formula() = default;

    ~Formula();

    void set_variables_count(size_t n);

    void set_clauses_count(size_t n);

    void add_clause(const std::string &clause_str);

    // Returns 0 if there are no unit clauses in the formula
    SignedVariable find_unit_clause();

    // Returns 0 if there are no pure literals in the formula
    SignedVariable find_pure_literal();

    // We are assuming that the literal appears in some unit clause
    static void propagate(SignedVariable literal);

    static void depropagate(const std::vector<Variable *> &eliminated_variables);

    std::pair<bool, std::vector<Variable *>> solve(const BranchingStrategy &branching_strategy);
};
