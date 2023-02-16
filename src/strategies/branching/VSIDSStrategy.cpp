#include "DLCSStrategy.h"
#include "VSIDSStrategy.h"

std::pair<Variable *, LiteralValue> VSIDSStrategy::choose(const Formula &formula) const {
    size_t vsids_max = 0;
    LiteralValue literal = std::rand() % 2 == 0 ? T : F;
    Variable *target_variable = nullptr;

    for (const auto &variable: formula.variables) {
        if (variable->value == U) {
            if (variable->vsids_score > vsids_max) {
                vsids_max = variable->vsids_score;
                target_variable = variable;
            }
        }
    }

    if (vsids_max == 0) {
        return DLCSStrategy().choose(formula);
    }

    assert(target_variable != nullptr);
    return {target_variable, literal};
}
