cmake_minimum_required(VERSION 3.19)
project(09025208_3 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Test)

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
    ticketview.cpp
    ticketview.h
    ticketview.ui
    querydialog.cpp
    querydialog.h
    querydialog.ui
    seat.cpp
    train.cpp
    trainsystem.cpp
)

target_link_libraries(09025208_3
    PRIVATE
        Qt::Core
        Qt::Widgets
)

enable_testing()

qt_add_executable(tst_train
    tests/tst_train.cpp
    seat.cpp
    train.cpp
)

target_link_libraries(tst_train
    PRIVATE
        Qt::Core
        Qt::Test
)

target_include_directories(tst_train PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME tst_train COMMAND tst_train)
