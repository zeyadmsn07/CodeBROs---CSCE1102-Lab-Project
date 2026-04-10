set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
find_package(Boost REQUIRED COMPONENTS system)

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
)

target_link_libraries(client PRIVATE
    Qt6::Widgets
    Qt6::Network
    Boost::system
    pthread
    shared
)