#pragma once

#include "Formula.h"

class BranchingStrategy {
public:
    virtual std::pair<int, LiteralValue> choose(const Formula &initial) const = 0;
};
