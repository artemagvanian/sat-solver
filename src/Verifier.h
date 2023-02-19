#pragma once

#include "Formula.h"

class Verifier {
public:
    static bool verify(const std::list<Clause *> &clauses);
};
