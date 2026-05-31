find_package(Boost REQUIRED COMPONENTS system)

# ---------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------
add_executable(server server.cpp RoomManager.cpp)
add_executable(client_test client_test.cpp)

# ---------------------------------------------------------------------
# Cross-Platform Threading (Replaces raw 'pthread')
# ---------------------------------------------------------------------
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------
# Target Link Libraries
# ---------------------------------------------------------------------
target_link_libraries(server PRIVATE
    Boost::system
    Threads::Threads
    shared
)

target_link_libraries(client_test PRIVATE
    Boost::system
    Threads::Threads
)

# ---------------------------------------------------------------------
# Windows Networking Dependencies (Winsock)
# ---------------------------------------------------------------------
if(WIN32)
    # Link Winsock2 libs to both targets so Boost.Asio compiles on Windows
    target_link_libraries(server PRIVATE ws2_32 mswsock)
    target_link_libraries(client_test PRIVATE ws2_32 mswsock)
endif()