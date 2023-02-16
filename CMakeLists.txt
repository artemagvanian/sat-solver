cmake_minimum_required(VERSION 3.18)
project(sat-solver)

set(CMAKE_CXX_STANDARD 14)

add_executable(sat-solver
        src/main.cpp
        src/Formula.h src/Formula.cpp
        src/deps/ngraph.hpp
        src/strategies/branching/BranchingStrategy.h
        src/strategies/branching/DLCSStrategy.h src/strategies/branching/DLCSStrategy.cpp
        src/strategies/branching/BohmsStrategy.h src/strategies/branching/BohmsStrategy.cpp
        src/strategies/branching/DLISStrategy.h src/strategies/branching/DLISStrategy.cpp
        src/strategies/branching/MomsStrategy.h src/strategies/branching/MomsStrategy.cpp
        src/strategies/branching/JeroslowWangStrategy.h src/strategies/branching/JeroslowWangStrategy.cpp
        src/strategies/branching/VSIDSStrategy.h src/strategies/branching/VSIDSStrategy.cpp
        src/Verifier.h src/Verifier.cpp )

target_include_directories(sat-solver PUBLIC src/)
#target_compile_options(sat-solver PUBLIC -fsanitize=address)
#target_link_options(sat-solver PUBLIC -fsanitize=address)
