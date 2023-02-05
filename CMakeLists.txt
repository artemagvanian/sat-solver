cmake_minimum_required(VERSION 3.18)
project(sat-solver)

set(CMAKE_CXX_STANDARD 14)

add_executable(sat-solver src/main.cpp src/Formula.cpp src/Formula.h src/strategies/branching/DLCSStrategy.cpp src/strategies/branching/DLCSStrategy.h src/strategies/branching/BranchingStrategy.h)
target_include_directories(sat-solver PUBLIC src/)
