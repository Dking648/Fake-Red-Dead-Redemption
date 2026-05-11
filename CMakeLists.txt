cmake_minimum_required(VERSION 3.16)
project(tangjiaqi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

add_executable(tangjiaqi
    main.cpp
    MainWindow.cpp
    MenuWidget.cpp
    LevelSelectWidget.cpp
    GameWidget.cpp
    CowboyRunGame.cpp        # ← 新增
    Character.cpp
    Player.cpp
    NPC.cpp
    resources.qrc
    Character.h CowboyRunGame.h gamewidget.h LevelSelectWidget.h MainWindow.h MenuWidget.h NPC.h Player.h
)


target_link_libraries(tangjiaqi PRIVATE Qt6::Widgets Qt6::Multimedia)
