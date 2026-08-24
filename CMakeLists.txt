cmake_minimum_required(VERSION 3.19)
project(09025208_3 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(09025208_3
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    addtraindialog.cpp
    addtraindialog.h
    addtraindialog.ui
    ticketdialog.cpp
    ticketdialog.h
    ticketdialog.ui
    seat.cpp
    train.cpp
    trainsystem.cpp
)

target_link_libraries(09025208_3
    PRIVATE
        Qt::Core
        Qt::Widgets
)
