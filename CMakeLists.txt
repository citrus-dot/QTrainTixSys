cmake_minimum_required(VERSION 3.19)
project(ticket_system LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Test)

qt_standard_project_setup()

# MSVC 默认按系统 ANSI 代码页解读无 BOM 源文件，显式指定 UTF-8 保证中文字面量正确
if(MSVC)
    add_compile_options(/utf-8)
endif()

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
    ticketcard.cpp
    ticketcard.h
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
    tableutil.h
    palette.h
    resources.qrc
)

# 应用图标按平台接入：macOS 用 .icns 进 bundle Resources；Windows 经 .rc 编译进 exe
if(APPLE)
    target_sources(ticket_system PRIVATE assets/appicon.icns)
    set_source_files_properties(assets/appicon.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
elseif(WIN32)
    target_sources(ticket_system PRIVATE assets/appicon.rc)
endif()

# Windows 产物名用 ASCII：中文 exe 名在 GBK 代码页下与 CMake 的 UTF-8 之间会乱码
if(WIN32)
    set(APP_OUTPUT_NAME QTrainTixSys)
else()
    set(APP_OUTPUT_NAME "列车售票系统")
endif()

set_target_properties(ticket_system PROPERTIES
    OUTPUT_NAME "${APP_OUTPUT_NAME}"
    MACOSX_BUNDLE_BUNDLE_NAME "列车售票系统"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.3.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.3.0"
    MACOSX_BUNDLE_ICON_FILE "appicon.icns"
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