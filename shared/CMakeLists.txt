add_library(shared STATIC)

target_sources(shared PRIVATE
    src/trial.cpp
)

target_include_directories(shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shared PUBLIC
    nlohmann_json::nlohmann_json
)
