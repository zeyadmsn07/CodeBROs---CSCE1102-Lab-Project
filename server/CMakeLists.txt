find_package(Boost REQUIRED COMPONENTS system)

add_executable(server
    server.cpp
    RoomManager.cpp
)

target_link_libraries(server PRIVATE
    Boost::system
    pthread
    shared          # <── NEW: gives access to MessageFactory, MessageTypes, nlohmann_json
)

# client_test stays unchanged
add_executable(client_test
    client_test.cpp
)

target_link_libraries(client_test PRIVATE
    Boost::system
    pthread
)