#pragma once

#include "Formula.h"

class BranchingStrategy {
public:
    virtual std::pair<Variable, LiteralValue> choose(const Formula &initial) const = 0;
};
