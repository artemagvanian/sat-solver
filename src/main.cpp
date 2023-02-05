#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <optional>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

std::vector<std::string> split(const std::string &str, char delim) {
    std::vector<std::string> items;
    std::stringstream str_stream(str);
    std::string item;
    while (std::getline(str_stream, item, delim)) {
        items.push_back(item);
    }
    return items;
}

class Formula {
private:
    std::vector<std::unordered_set<int>> clauses;
    std::unordered_map<int, std::optional<bool>> assignments;
    int variables_count = 0;
    int clauses_count = 0;

public:
    Formula() = default;

    void set_variables_count(int n) {
        variables_count = n;
    }

    void set_clauses_count(int n) {
        clauses_count = n;
    }

    void add_clause(const std::string &clause_str) {
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

    std::unordered_map<int, std::optional<bool>> get_assignments() {
        return assignments;
    }

    std::optional<int> find_unit_clause() {
        for (const auto &clause: clauses) {
            if (clause.size() == 1) {
                return *clause.begin();
            }
        }
        return {};
    }

    // We are assuming that the literal appears in some unit clause
    void propagate_unit_clause(int literal) {
        assert(literal != 0);

        if (literal < 0) {
            assignments[-literal] = false;
        } else {
            assignments[literal] = true;
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

    std::optional<int> find_pure_literal() {
        for (const auto &[variable, assignment]: assignments) {
            bool pure = true;
            if (!assignment.has_value()) {
                std::optional<int> variable_instance = {};
                for (const auto &clause: clauses) {
                    if (!variable_instance.has_value()) {
                        if (clause.find(variable) != clause.end()) {
                            variable_instance = variable;
                        } else if (clause.find(-variable) != clause.end()) {
                            variable_instance = -variable;
                        }
                    } else {
                        if (clause.find(variable_instance.value()) == clause.end()) {
                            pure = false;
                            break;
                        }
                    }
                }

                if (variable_instance.has_value() && pure) {
                    return variable_instance;
                }
            }
        }
        return {};
    }

    // We are assuming that the literal appears only in this form
    void eliminate_pure_literal(int literal) {
        assert(literal != 0);

        if (literal < 0) {
            assignments[-literal] = false;
        } else {
            assignments[literal] = true;
        }

        // Remove all clauses including literal
        clauses.erase(std::remove_if(clauses.begin(), clauses.end(),
                                     [&](auto &clause) {
                                         return clause.find(literal) != clause.end();
                                     }), clauses.end());
    }

    void print() {
        std::cout << "FORMULA WITH " << variables_count << " VARS AND " << clauses_count << " CLAUSES" << std::endl;
        std::cout << "CLAUSES: ";
        for (const auto &clause: clauses) {
            std::cout << "(";
            for (const auto &literal: clause) {
                std::cout << literal << ",";
            }
            std::cout << ") ";
        }

        std::cout << std::endl << "ASSIGNMENTS: ";
        for (const auto &[variable, assignment]: assignments) {
            std::cout << variable << " = ";
            if (assignment.has_value()) {
                std::cout << (assignment.value() == 1 ? "T" : "F") << "; ";
            } else {
                std::cout << "?; ";
            }
        }
    }

    bool solve() {
        std::optional<int> unit_clause;
        while ((unit_clause = this->find_unit_clause()).has_value()) {
            this->propagate_unit_clause(unit_clause.value());
        }

        std::optional<int> pure_literal;
        while ((pure_literal = this->find_pure_literal()).has_value()) {
            this->eliminate_pure_literal(pure_literal.value());
        }

        if (clauses.empty()) {
            return true;
        } else if (std::find_if(clauses.begin(), clauses.end(),
                                [](const auto &clause) {
                                    return clause.empty();
                                }) != clauses.end()) {
            return false;
        }

        for (auto &[variable, assignment]: assignments) {
            if (!assignment.has_value()) {
                Formula branch_true = *this;
                branch_true.assignments[variable] = true;
                branch_true.propagate_unit_clause(variable);

                if (branch_true.solve()) {
                    *this = branch_true;
                    return true;
                } else {
                    Formula branch_false = *this;
                    branch_false.assignments[variable] = false;
                    branch_false.propagate_unit_clause(-variable);

                    if (branch_false.solve()) {
                        *this = branch_false;
                        return true;
                    }
                }
            }
        }

        return false;
    }
};

int main(int argc, char **argv) {
    std::ifstream file(argv[1]);

    Formula formula;

    std::string line;
    while (std::getline(file, line)) {
        if (line[0] == 'c') {
            continue;
        } else if (line[0] == 'p') {
            std::vector<std::string> params = split(line, ' ');

            assert(params.size() == 4);
            formula.set_variables_count(std::stoi(params[2]));
            formula.set_clauses_count(std::stoi(params[3]));

            while (std::getline(file, line)) {
                formula.add_clause(line);
            }
            break;
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    bool sat = formula.solve();
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << R"({"Instance": ")" << argv[1] << "\", " <<
              R"("Time": ")" << std::fixed << std::setprecision(2) << (float) duration.count() / 1000 << "\", " <<
              R"("Result": ")" << (sat ? "SAT" : "UNSAT") << "\"";

    if (sat) {
        std::cout << ", " << R"("Solution": ")";
        std::string solution;
        for (auto &[variable, assignment]: formula.get_assignments()) {
            solution.append(std::to_string(variable));
            solution.append(" ");
            if (assignment.has_value()) {
                solution.append(assignment.value() ? "true" : "false");
            } else {
                solution.append("true");
            }
            solution.append(" ");
        }
        solution.pop_back();
        std::cout << solution << "\"}";
    } else {
        std::cout << "}";
    }

    return 0;
}
