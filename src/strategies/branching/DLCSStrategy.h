#pragma once

#include "strategies/branching/BranchingStrategy.h"

class DLCSStrategy : public BranchingStrategy {
public:
    std::pair<Variable, LiteralValue> choose(const Formula &initial) const override;
};
