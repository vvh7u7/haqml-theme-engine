cmake_minimum_required(VERSION 4.3.1)
project(ThemeEngineTests LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(tst_themeengine
        tst_parser.cpp
)

target_include_directories(tst_themeengine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../module/include)

target_link_libraries(tst_themeengine
        PRIVATE
        HaThemeEngine
        Qt6::Test
)

add_test(NAME ThemeEngineTests COMMAND tst_themeengine)