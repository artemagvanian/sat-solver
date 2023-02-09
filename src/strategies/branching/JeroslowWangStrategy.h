#pragma once

#include "strategies/branching/BranchingStrategy.h"

class JeroslowWangStrategy : public BranchingStrategy {
public:
    std::pair<Variable, LiteralValue> choose(const Formula &formula) const override;
};

