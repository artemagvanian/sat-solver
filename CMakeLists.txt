cmake_minimum_required(VERSION 3.18)
project(sat-solver)

set(CMAKE_CXX_STANDARD 14)

add_executable(sat-solver
        src/main.cpp
        src/Formula.h src/Formula.cpp
        src/strategies/branching/BranchingStrategy.h
        src/strategies/branching/VSIDSStrategy.h src/strategies/branching/VSIDSStrategy.cpp
        src/Verifier.h src/Verifier.cpp)

target_include_directories(sat-solver PUBLIC src/)
# target_compile_options(sat-solver PUBLIC -fsanitize=address)
# target_link_options(sat-solver PUBLIC -fsanitize=address)
