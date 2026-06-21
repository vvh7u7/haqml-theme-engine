cmake_minimum_required(VERSION 4.3.1)
project(HaThemeEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Qml
        Quick
        Widgets
        Svg
)

find_package(Qt6Svg REQUIRED)

add_subdirectory(module)

option(BUILD_EDITOR "Build theme editor" OFF)

if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()

enable_testing()
add_subdirectory(tests)