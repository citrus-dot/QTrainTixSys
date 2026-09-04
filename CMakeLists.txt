cmake_minimum_required(VERSION 3.19)
project(ticket_system LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Test)

qt_standard_project_setup()

# 确保 .qrc 资源文件被 AUTORCC 编译进可执行文件
set(CMAKE_AUTORCC ON)

qt_add_executable(ticket_system
    WIN32 MACOSX_BUNDLE
    main.cpp
    donutchart.cpp
    donutchart.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    statisticspage.cpp
    statisticspage.h
    sidebarwidget.cpp
    sidebarwidget.h
    trainlistview.cpp
    trainlistview.h
    querypage.cpp
    querypage.h
    querypage.ui
    addtraindialog.cpp
    addtraindialog.h
    addtraindialog.ui
    ticketdialog.cpp
    ticketdialog.h
    ticketdialog.ui
    ticketview.cpp
    ticketview.h
    ticketview.ui
    seattableview.cpp
    seattableview.h
    seattabledialog.cpp
    seattabledialog.h
    registerdialog.cpp
    registerdialog.h
    seatstatustable.cpp
    seatstatustable.h
    selldialog.cpp
    selldialog.h
    refunddialog.cpp
    refunddialog.h
    seatmappopup.cpp
    seatmappopup.h
    fadedialog.cpp
    fadedialog.h
    seat.cpp
    train.cpp
    trainsystem.cpp
    resources.qrc
)

set_target_properties(ticket_system PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "列车售票系统"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.2.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.2.0"
)

target_link_libraries(ticket_system
    PRIVATE
        Qt::Core
        Qt::Widgets
)

target_include_directories(ticket_system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

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

qt_add_executable(tst_system
    tests/tst_system.cpp
    seat.cpp
    train.cpp
    trainsystem.cpp
)

target_link_libraries(tst_system
    PRIVATE
        Qt::Core
        Qt::Test
)

target_include_directories(tst_system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME tst_system COMMAND tst_system)