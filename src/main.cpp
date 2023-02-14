#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "Formula.h"
#include "Verifier.h"
#include "strategies/branching/JeroslowWangStrategy.h"
#include "strategies/branching/BohmsStrategy.h"
#include "strategies/branching/MomsStrategy.h"
#include "strategies/branching/DLISStrategy.h"
#include "strategies/branching/DLCSStrategy.h"

#define VERIFY true

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

    const BranchingStrategy &strategy = BohmsStrategy(1, 2);

    auto start = std::chrono::high_resolution_clock::now();
    auto sat = formula.solve(strategy);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::string path = argv[1];

    std::cout << std::boolalpha;

    if (VERIFY && sat.first) {
        assert(Verifier::verify(formula.clauses));
    }

    std::cout << R"({"Instance": ")" << path.substr(path.find_last_of("/\\") + 1) << "\", " <<
              R"("Time": ")" << std::fixed << std::setprecision(2) << (float) duration.count() / 1000 << "\", " <<
              R"("Result": ")" << (sat.first ? "SAT" : "UNSAT") << "\"";

    if (sat.first) {
        std::cout << ", " << R"("Solution": ")";
        std::sort(formula.variables.begin(), formula.variables.end(),
                  [](const auto &a, const auto &b) {
                      return a->id < b->id;
                  });

        std::string solution;
        for (auto &variable: formula.variables) {
            const auto &id = variable->id;
            const auto &value = variable->value;

            solution.append(std::to_string(id));
            solution.append(" ");
            if (value != U) {
                solution.append(value == T ? "true" : "false");
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
