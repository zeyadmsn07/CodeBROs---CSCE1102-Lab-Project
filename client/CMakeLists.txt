set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(client
    main.cpp
    MainWindow.cpp
    MainWindow.h
    LoginWidget.cpp
    RegisterWidget.cpp
    RegisterWidget.h
    LoginWidget.h
    resources.qrc
) #remeber to move all header files to a directory and use target_include_directories later...

target_link_libraries(client PRIVATE Qt6::Widgets Qt6::Network shared)
