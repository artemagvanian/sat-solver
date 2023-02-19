#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <string>
#include <sstream>

#include "Formula.h"
#include "strategies/branching/BranchingStrategy.h"

#define ASSERT false

std::vector<std::string> split(const std::string &str, char delim) {
    std::vector<std::string> items;
    std::stringstream str_stream(str);
    std::string item;
    while (std::getline(str_stream, item, delim)) {
        if (!item.empty()) items.push_back(item);
    }
    return items;
}

void Formula::set_variables_count(size_t n) {
//    variables.reserve(n);
}

void Formula::set_clauses_count(size_t n) {
//    clauses.reserve(n);
}

void Formula::add_clause(const std::string &clause_str) {
    std::vector<std::string> clause_str_vec = split(clause_str, ' ');
    std::vector<long> clause_vec;
    for (const auto &str_literal: clause_str_vec) {
        if (str_literal != "0") {
            clause_vec.push_back(std::stol(str_literal));
        } else {
            add_clause(clause_vec);
            break;
        }
    }

}

void Formula::add_clause(const std::vector<long> &clause_vec) {
    auto *clause = new Clause{{},
                              {}};
    for (const long &new_literal: clause_vec) {
        if (ASSERT) assert(new_literal != 0);
        if (std::find_if(clause->literals.cbegin(), clause->literals.cend(),
                         [&](const auto &literal) {
                             return literal.variable->id == std::abs(new_literal);
                         }) == clause->literals.end()) {

            auto it = std::find_if(variables.begin(), variables.end(),
                                   [&](const auto &variable) {
                                       return variable->id == std::abs(new_literal);
                                   });

            if (it == variables.end()) {
                auto *variable = new Variable{std::abs(new_literal), U, nullptr, {}, {}, 0, 0};
                if (new_literal > 0) {
                    variable->positive_occurrences.push_back(clause);
                    clause->literals.emplace_back(1, variable);
                } else {
                    variable->negative_occurrences.push_back(clause);
                    clause->literals.emplace_back(-1, variable);
                }
                variables.push_back(variable);
            } else {
                if (new_literal > 0) {
                    (*it)->positive_occurrences.push_back(clause);
                    clause->literals.emplace_back(1, (*it));
                } else {
                    (*it)->negative_occurrences.push_back(clause);
                    clause->literals.emplace_back(-1, (*it));
                }
            }
        }
    }

    if (clause->literals.size() >= 1) {
        clause->watched.first = *clause->literals.begin();
    }
    if (clause->literals.size() >= 2) {
        clause->watched.second = *std::next(clause->literals.begin());
    }

    clauses.push_back(clause);
}

void Formula::add_learned_clause(const std::unordered_set<SignedVariable, hash_sv> &clause_vec) {
    auto *clause = new Clause{{},
                              {}};
    for (const SignedVariable &new_literal: clause_vec) {
        // Variables in a learned clause can't be unassigned at the time of adding.
        if (ASSERT) assert(new_literal.variable->value != U);
        // Variables in a learned clause cannot satisfy this clause
        if (ASSERT)
            assert(new_literal.variable->value == T && new_literal.sign < 0 ||
                   new_literal.variable->value == F && new_literal.sign > 0);
        if (new_literal.sign > 0) {
            new_literal.variable->positive_occurrences.push_back(clause);
            new_literal.variable->negative_conflicts++;
        } else if (new_literal.sign < 0) {
            new_literal.variable->negative_occurrences.push_back(clause);
            new_literal.variable->positive_conflicts++;
        }
        clause->literals.push_back(new_literal);
    }

    if (clause->literals.size() >= 1) {
        clause->watched.first = *clause->literals.begin();
    }
    if (clause->literals.size() >= 2) {
        clause->watched.second = *std::next(clause->literals.begin());
    }

    clauses.push_back(clause);
}

std::unordered_set<SignedVariable, hash_sv>
Formula::register_conflict(Variable *variable, Clause *implicated_by_left, Clause *implicated_by_right) {
    if (implicated_by_left != nullptr && implicated_by_right != nullptr) {
        std::unordered_set<SignedVariable, hash_sv> conflict;
        std::unordered_set<Variable *> visited;
        std::deque<SignedVariable> traversal_queue;

        for (const auto &implicated_by_literal: implicated_by_left->literals) {
            if (implicated_by_literal.variable != variable) {
                assert(implicated_by_literal.variable->value != U &&
                       (implicated_by_literal.variable->value == T && implicated_by_literal.sign < 0) ||
                       (implicated_by_literal.variable->value == F && implicated_by_literal.sign > 0));
                if (visited.find(implicated_by_literal.variable) == visited.end()) {
                    traversal_queue.push_back(implicated_by_literal);
                    visited.insert(implicated_by_literal.variable);
                }
            }
        }

        for (const auto &implicated_by_literal: implicated_by_right->literals) {
            if (implicated_by_literal.variable != variable) {
                assert(implicated_by_literal.variable->value != U &&
                       (implicated_by_literal.variable->value == T && implicated_by_literal.sign < 0) ||
                       (implicated_by_literal.variable->value == F && implicated_by_literal.sign > 0));
                if (visited.find(implicated_by_literal.variable) == visited.end()) {
                    traversal_queue.push_back(implicated_by_literal);
                    visited.insert(implicated_by_literal.variable);
                }
            }
        }

        while (!traversal_queue.empty()) {
            SignedVariable literal = traversal_queue.front();

            // Found a UIP, return it immediately
            if (traversal_queue.size() == 1 && conflict.empty()) {
                conflict = {literal};
                break;
            }

            if (literal.variable->implicated_by == nullptr) {
                if (conflict.find(literal) == conflict.end()) {
                    conflict.insert(literal);
                }
            } else {
                for (const auto &implicated_by_literal: literal.variable->implicated_by->literals) {
                    if (visited.find(implicated_by_literal.variable) == visited.end()) {
                        traversal_queue.push_back(implicated_by_literal);
                        visited.insert(implicated_by_literal.variable);
                    }
                }
            }
            traversal_queue.pop_front();
        }

        for (const auto &clause: clauses) {
            if (clause->literals.size() == conflict.size()) {
                bool seen_clause = true;
                auto clause_it = clause->literals.begin();
                auto conflict_it = conflict.begin();
                while (conflict_it != conflict.end()) {
                    if ((*clause_it).variable != (*conflict_it).variable ||
                        (*clause_it).sign != (*conflict_it).sign) {
                        seen_clause = false;
                        break;
                    }
                    clause_it++;
                    conflict_it++;
                }
                if (seen_clause) return {};
            }
        }

        add_learned_clause(conflict);
        new_conflicts++;
        return conflict;
    }
    return {};
}

// We are assuming that the literal appears in some unit clause
bool Formula::propagate(SignedVariable literal, Clause *implicated_by, bool retry) {
    // Can't propagate an empty variable.
    if (ASSERT) assert(literal.sign != 0);
    if (ASSERT) assert(literal.variable != nullptr);

    new_iterations++;
    choices.emplace_back(retry, literal);
    literal.variable->implicated_by = implicated_by;

    if (literal.sign > 0) {
        literal.variable->value = T;
        for (const auto &positive_occurrence: literal.variable->positive_occurrences) {
            if (positive_occurrence->watched.first.variable != literal.variable) {
                positive_occurrence->watched.first = literal;
            } else if (positive_occurrence->watched.second.variable != literal.variable) {
                positive_occurrence->watched.second = literal;
            }
        }
        return check_clauses(literal.variable->negative_occurrences);
    } else if (literal.sign < 0) {
        literal.variable->value = F;
        for (const auto &negative_occurrence: literal.variable->negative_occurrences) {
            if (negative_occurrence->watched.first.variable != literal.variable) {
                negative_occurrence->watched.first = literal;
            } else if (negative_occurrence->watched.second.variable != literal.variable) {
                negative_occurrence->watched.second = literal;
            }
        }
        return check_clauses(literal.variable->positive_occurrences);
    }
    assert(false);
}

SignedVariable Formula::find_new_watched(Clause *clause) {
    for (const auto &literal: clause->literals) {
        bool unassigned_variable = literal.variable->value == U;
        bool satisfying_variable = (literal.variable->value == T && literal.sign > 0) ||
                                   (literal.variable->value == F && literal.sign < 0);
        if ((unassigned_variable || satisfying_variable) &&
            literal.variable != clause->watched.first.variable &&
            literal.variable != clause->watched.second.variable) {
            return literal;
        }
    }
    return {0, nullptr};
}

Choice Formula::find_conflicting_choice(Clause *dead_clause) {
    for (const auto &choice: choices) {
        for (const auto &literal: dead_clause->literals) {
            if (choice.literal.variable == literal.variable &&
                choice.literal.sign != literal.sign) {
                return choice;
            }
        }
    }
    return {false, {0, nullptr}};
}

bool Formula::non_chronological_backtrack(const std::unordered_set<SignedVariable, hash_sv> &cut) {
    bool backtracked = false;
    while (!choices.empty()) {
        // Non-chronological backtrack.
        for (const auto &literal: cut) {
            if (choices.back().literal == literal) {
                backtracked = true;
                break;
            }
        }
        Choice last_choice = choices.back();
        depropagate(last_choice.literal);
        choices.pop_back();
        if (backtracked) break;
    }

    return backtracked;
}

bool Formula::backtrack() {
    while (!choices.empty() && !choices.back().retry) {
        Choice last_choice = choices.back();
        depropagate(last_choice.literal);
        choices.pop_back();
    }
    // Exhausted our space
    if (choices.empty()) {
        return false;
    } else {
        Choice last_choice = choices.back();
        depropagate(last_choice.literal);
        if (ASSERT) assert(last_choice.retry);
        if (ASSERT) assert(last_choice.literal.variable->implicated_by == nullptr);
        SignedVariable new_literal = {-last_choice.literal.sign, last_choice.literal.variable};
        choices.pop_back();
        propagate(new_literal, nullptr, false);
        return true;
    }
}

void Formula::random_restart() {
    if (new_conflicts >= current_ceiling) {
        while (!choices.empty()) {
            Choice last_choice = choices.back();
            depropagate(last_choice.literal);
            choices.pop_back();
        }
        new_conflicts = 0;
        if (current_ceiling == absolute_ceiling) {
            current_ceiling = 1;
            absolute_ceiling *= 2;
        } else {
            current_ceiling *= 2;
        }
    }
}

void Formula::decay(double decay_factor) {
    if (new_iterations > 65536) {
        print();
        for (const auto &variable: variables) {
            variable->positive_conflicts *= decay_factor;
            variable->negative_conflicts *= decay_factor;
        }
        new_iterations = 0;
    };
}

// Returns true if we were able to recover from the conflict.
bool Formula::handle_conflict(Clause *clause) {
    // Conflict encountered, time to register it.
    Choice conflicting_choice = find_conflicting_choice(clause);
    std::unordered_set<SignedVariable, hash_sv> cut =
            register_conflict(conflicting_choice.literal.variable,
                              conflicting_choice.literal.variable->implicated_by,
                              clause);
    if (cut.empty()) {
        if (!backtrack()) {
            return false;
        }
    } else {
        if (!non_chronological_backtrack(cut)) {
            return false;
        }
    }
    return true;
}

bool Formula::check_all_satisfied() {
    for (const auto &clause: clauses) {
        bool first_watched_satisfies =
                clause->watched.first.variable->value == T && clause->watched.first.sign > 0 ||
                clause->watched.first.variable->value == F && clause->watched.first.sign < 0;
        bool second_watched_satisfies =
                clause->watched.second.variable->value == T && clause->watched.second.sign > 0 ||
                clause->watched.second.variable->value == F && clause->watched.second.sign < 0;
        if (first_watched_satisfies || second_watched_satisfies) {
            continue;
        }
        return false;
    }
    return true;
}

bool Formula::check_clauses(const std::list<Clause *> &clauses_to_check) {
    // Perform boolean constraint propagation.
    for (auto &clause: clauses_to_check) {
        // If we can no longer hold on to the literals.
        if (clause->literals.size() >= 2 && (clause->watched.first.variable->value != U ||
                                             clause->watched.second.variable->value != U)) {
            // Check whether one of the watched variables satisfies the clause.
            bool first_watched_satisfies =
                    clause->watched.first.variable->value == T && clause->watched.first.sign > 0 ||
                    clause->watched.first.variable->value == F && clause->watched.first.sign < 0;
            bool second_watched_satisfies =
                    clause->watched.second.variable->value == T && clause->watched.second.sign > 0 ||
                    clause->watched.second.variable->value == F && clause->watched.second.sign < 0;
            if (first_watched_satisfies || second_watched_satisfies) {
                continue;
            } else {
                // The first watched variable has died, attempt to watch something new.
                if (clause->watched.first.variable->value != U) {
                    SignedVariable watched_candidate = find_new_watched(clause);
                    // Found a new variable to watch!
                    if (watched_candidate.variable != nullptr) {
                        clause->watched.first = watched_candidate;
                    }
                }
                // The second watched variable has died, attempt to watch something new.
                if (clause->watched.second.variable->value != U) {
                    SignedVariable watched_candidate = find_new_watched(clause);
                    // Found a new variable to watch!
                    if (watched_candidate.variable != nullptr) {
                        clause->watched.second = watched_candidate;
                    }
                }

                // Check whether one of the watched variables satisfies the clause after the reassignment.
                first_watched_satisfies =
                        clause->watched.first.variable->value == T && clause->watched.first.sign > 0 ||
                        clause->watched.first.variable->value == F && clause->watched.first.sign < 0;
                second_watched_satisfies =
                        clause->watched.second.variable->value == T && clause->watched.second.sign > 0 ||
                        clause->watched.second.variable->value == F && clause->watched.second.sign < 0;

                if (first_watched_satisfies || second_watched_satisfies) {
                    continue;
                } else if (clause->watched.first.variable->value != U &&
                           clause->watched.second.variable->value != U) {
                    // If both of the watchers are dead.
                    if (!handle_conflict(clause)) {
                        return false;
                    }
                    break;
                } else if (clause->watched.first.variable->value != U &&
                           clause->watched.second.variable->value == U) {
                    if (!propagate(clause->watched.second, clause, false)) {
                        return false;
                    }
                } else if (clause->watched.second.variable->value != U &&
                           clause->watched.first.variable->value == U) {
                    if (!propagate(clause->watched.first, clause, false)) {
                        return false;
                    }
                }
            }
        } else if (clause->literals.size() == 1) {
            if (clause->watched.first.variable->value == U) {
                if (!propagate(clause->watched.first, clause, false)) {
                    return false;
                }
            } else if (clause->watched.first.variable->value == T && clause->watched.first.sign < 0 ||
                       clause->watched.first.variable->value == F && clause->watched.first.sign > 0) {
                if (!handle_conflict(clause)) {
                    return false;
                }
                break;
            } else {
                continue;
            }
        }
        // We disallow empty clauses.
        if (ASSERT) assert(!clause->literals.empty());
    }
    return true;
}

bool Formula::solve(const BranchingStrategy &branching_strategy) {
    while (true) {
        // Luby restarts.
        random_restart();
        decay(0.95);

        if (!check_clauses(clauses)) return false;

        if (check_all_satisfied()) {
            return true;
        }

        // Choose a variable to branch on.
        const std::pair<Variable *, LiteralValue> assignment = branching_strategy.choose(*this);
        SignedVariable literal{assignment.second == T ? 1 : -1, assignment.first};
        propagate(literal, nullptr, true);
    }
}

void Formula::depropagate(const SignedVariable &eliminated_literal) {
    eliminated_literal.variable->value = U;
    eliminated_literal.variable->implicated_by = nullptr;
}

void Formula::print() const {
    std::cout << "# CLAUSES: " << clauses.size()
              << " # CHOICES: " << choices.size()
              << " # NEW CONFLICTS: " << new_conflicts
              << " # CURRENT CEILING: " << current_ceiling
              << " # ABSOLUTE CEILING: " << absolute_ceiling
              << " # NEW ITERATIONS: " << new_iterations << std::endl;
}

Formula::~Formula() {
    for (auto &variable: variables) {
        delete variable;
    }
    for (auto &clause: clauses) {
        delete clause;
    }
}
