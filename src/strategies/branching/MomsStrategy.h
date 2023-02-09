#pragma once

#include "strategies/branching/BranchingStrategy.h"

class MomsStrategy : public BranchingStrategy {
private:
    long k;
public:
    MomsStrategy(long k) {
        this->k = k;
    }

    std::pair<Variable *, LiteralValue> choose(const Formula &formula) const override;
};
