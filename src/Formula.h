#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// Forward-declaration
class BranchingStrategy;

typedef long ID;
typedef long Sign;

enum LiteralValue {
    U, T, F
};

struct Clause;
struct Variable;

struct SignedVariable {
    Sign sign;
    Variable *variable;

    SignedVariable() {
        this->sign = 0;
        this->variable = nullptr;
    }

    SignedVariable(Sign sign, Variable *variable) {
        this->sign = sign;
        this->variable = variable;
    }

    bool operator==(const SignedVariable &literal) const {
        return sign == literal.sign && variable == literal.variable;
    }
};

struct hash_sv {
    std::size_t operator()(const SignedVariable &literal) const {
        std::size_t h1 = std::hash<Sign>()(literal.sign);
        std::size_t h2 = std::hash<Variable *>()(literal.variable);

        return h1 ^ h2;
    }
};

struct Clause {
    std::list<SignedVariable> literals;
    std::pair<SignedVariable, SignedVariable> watched;
};

struct Variable {
    ID id;
    LiteralValue value;

    Clause *implicated_by;

    std::list<Clause *> positive_occurrences;
    std::list<Clause *> negative_occurrences;

    double positive_conflicts;
    double negative_conflicts;
};

struct Choice {
    Choice() {
        this->retry = false;
        this->literal = SignedVariable();
    }

    Choice(bool retry, SignedVariable literal) {
        this->retry = retry;
        this->literal = literal;
    }


    bool retry{};
    SignedVariable literal;
};

std::vector<std::string> split(const std::string &, char);

class Formula {
public:
    std::list<Clause *> clauses;
    std::list<Variable *> variables;
    std::list<Choice> choices;

    long new_conflicts = 0;

    long current_ceiling = 1;
    long absolute_ceiling = 2;

    long new_iterations = 0;

    Formula() = default;

    ~Formula();

    void set_variables_count(size_t);

    void set_clauses_count(size_t);

    void add_clause(const std::string &);

    void add_clause(const std::vector<long> &);

    void add_learned_clause(const std::unordered_set<SignedVariable, hash_sv> &);

    bool backtrack();

    bool propagate(SignedVariable, Clause *, bool);

    void depropagate(const SignedVariable &);

    bool solve(const BranchingStrategy &);

    std::unordered_set<SignedVariable, hash_sv>
    register_conflict(Variable *, Clause *, Clause *);

    SignedVariable find_new_watched(Clause *);

    Choice find_conflicting_choice(Clause *);

    bool non_chronological_backtrack(const std::unordered_set<SignedVariable, hash_sv> &);

    void print() const;

    void random_restart();

    void decay(double decay_factor);

    bool check_clauses(const std::list<Clause *> &);

    bool handle_conflict(Clause *);

    bool check_all_satisfied();
};
