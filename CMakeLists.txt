cmake_minimum_required(VERSION 3.20)
project(CodeBROs CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------
# Windows-Specific Package Paths (vcpkg / Manual Installation Hints)
# ---------------------------------------------------------------------
if(WIN32)
    # If using vcpkg for nlohmann_json/GTest, this forces CMake to look there.
    # Alternatively, you can pass -DCMAKE_TOOLCHAIN_FILE to CMake in Qt Creator.
    set(CMAKE_WINDOW_EXPORT_ALL_SYMBOLS ON) # Helps build shared libraries easily on Windows
endif()

# ---------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

enable_testing()

# ---------------------------------------------------------------------
# Subdirectories & Feature Flags
# ---------------------------------------------------------------------
add_subdirectory(shared)
add_subdirectory(tests)

option(BUILD_CLIENT "Build the Qt client" ON)
option(BUILD_SERVER "Build the server" ON)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_SERVER)
    add_subdirectory(server)
endif()