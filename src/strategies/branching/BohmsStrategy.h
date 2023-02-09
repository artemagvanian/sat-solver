#pragma once

#include "strategies/branching/BranchingStrategy.h"

class BohmsStrategy : public BranchingStrategy {
private:
    long alpha;
    long beta;
public:
    BohmsStrategy(long alpha, long beta) {
        this->alpha = alpha;
        this->beta = beta;
    }

    std::pair<Variable *, LiteralValue> choose(const Formula &initial) const override;
};
