cmake_minimum_required(VERSION 3.20)
project(CodeBROs CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

enable_testing()
add_subdirectory(shared)
add_subdirectory(tests)
#add_subdirectory(client)
#add_subdirectory(server)

option(BUILD_CLIENT "Build the Qt client" ON)
option(BUILD_SERVER "Build the server" ON)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_SERVER)
    add_subdirectory(server)
endif()
