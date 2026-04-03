cmake_minimum_required(VERSION 3.20)
project(CodeBROs CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

enable_testing()
add_subdirectory(shared)
add_subdirectory(tests)
