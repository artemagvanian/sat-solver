#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

// Forward-declaration
class BranchingStrategy;

enum ComparisonResult {
    Equal, Greater, Less
};

typedef long ID;
typedef int Sign;

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

    long vsids_score;
};

struct Choice {
    bool retry;
    SignedVariable literal;
    Clause *implicated_by;
};

std::vector<std::string> split(const std::string &str, char delim);

class Formula {
public:
    std::vector<Clause *> clauses;
    std::vector<Variable *> variables;
    std::vector<Choice> choices;

    long new_conflicts = 0;
    long active_clauses = 0;
    long conflict_ceiling = 1;

    Formula() = default;

    ~Formula();

    void set_variables_count(size_t n);

    void set_clauses_count(size_t n);

    void add_clause(const std::string &clause_str);

    void add_clause(const std::vector<long> &clause_vec);

    void add_learned_clause(const std::vector<SignedVariable> &clause_vec);

    bool backtrack();

    // Returns 0 if there are no unit clauses in the formula
    std::pair<SignedVariable, Clause *> find_unit_clause();

    // We are assuming that the literal appears in some unit clause
    bool propagate(SignedVariable literal, Clause *implicated_by, bool retry);

    void depropagate(const SignedVariable &eliminated_literal);

    bool solve(const BranchingStrategy &branching_strategy);

    void register_conflict(SignedVariable literal, Clause *implicated_by);
};
