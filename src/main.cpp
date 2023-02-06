#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Formula.h"
#include "strategies/branching/DLCSStrategy.h"

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
            formula.set_variables_count(std::stol(params[2]));
            formula.set_clauses_count(std::stol(params[3]));

            while (std::getline(file, line)) {
                formula.add_clause(line);
            }
            break;
        }
    }

    const BranchingStrategy &strategy = DLCSStrategy();

    auto start = std::chrono::high_resolution_clock::now();
    bool sat = formula.solve(strategy);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::string path = argv[1];

    std::cout << R"({"Instance": ")" << path.substr(path.find_last_of("/\\") + 1) << "\", " <<
              R"("Time": ")" << std::fixed << std::setprecision(2) << (float) duration.count() / 1000 << "\", " <<
              R"("Result": ")" << (sat ? "SAT" : "UNSAT") << "\"";

    if (sat) {
        std::cout << ", " << R"("Solution": ")";
        std::vector<std::pair<int, LiteralValue>> assignments_vec(formula.assignments.begin(),
                                                                  formula.assignments.end());
        std::sort(assignments_vec.begin(), assignments_vec.end(),
                  [](const auto &a, const auto &b) {
                      return a.first < b.first;
                  });

        std::string solution;
        for (auto &pair: assignments_vec) {
            const auto &variable = pair.first;
            const auto &assignment = pair.second;

            solution.append(std::to_string(variable));
            solution.append(" ");
            if (assignment != U) {
                solution.append(assignment == T ? "true" : "false");
            } else {
                solution.append("true");
            }
            solution.append(" ");
        }
        solution.pop_back();
        std::cout << solution << "\"}" << std::endl;
    } else {
        std::cout << "}" << std::endl;
    }

    return 0;
}
