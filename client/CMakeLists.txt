set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find packages cleanly
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
find_package(Boost REQUIRED COMPONENTS system)
find_package(CURL REQUIRED) 

# Define the executable with WIN32 flag conditionally so it runs as a pure GUI on Windows
if(WIN32)
    add_executable(client WIN32
        main.cpp
        MainWindow.cpp
        MainWindow.h
        LoginWidget.cpp
        LoginWidget.h
        RegisterWidget.cpp
        RegisterWidget.h
        DashboardWidget.cpp
        DashboardWidget.h
        NetworkClient.cpp
        NetworkClient.h
        resources.qrc
        RoomWidget.cpp
        AiHelper.cpp
        Highlighter.cpp
        Highlighter.h
        config.h
        TaskWidget.cpp
        TaskWidget.h
    )
else()
    add_executable(client
        main.cpp
        MainWindow.cpp
        MainWindow.h
        LoginWidget.cpp
        LoginWidget.h
        RegisterWidget.cpp
        RegisterWidget.h
        DashboardWidget.cpp
        DashboardWidget.h
        NetworkClient.cpp
        NetworkClient.h
        resources.qrc
        RoomWidget.cpp
        AiHelper.cpp
        Highlighter.cpp
        Highlighter.h
        config.h
        TaskWidget.cpp
        TaskWidget.h
    )
endif()

# Link everything seamlessly
target_link_libraries(client PRIVATE
    Qt6::Widgets
    Qt6::Network
    Boost::system
    shared
    CURL::libcurl
)

# Handle threading cross-platform (replaces hardcoded pthread)
find_package(Threads REQUIRED)
target_link_libraries(client PRIVATE Threads::Threads)