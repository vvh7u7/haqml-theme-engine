set(SOURCES
        source/ThemeManager.cpp
        source/ThemeParser.cpp
        source/IconManager.cpp
)

set(HEADERS
        include/ThemeEngine/ThemeManager.hpp
        include/ThemeEngine/ThemeParser.hpp
        include/ThemeEngine/IconManager.hpp
)

qt_add_resources(HA_THEME_RESOURCES haqml_resources.qrc)

add_library(HaThemeEngine STATIC
        ${SOURCES}
        ${HEADERS}
        ${HA_THEME_RESOURCES}
        #[[resources.qrc]]
)

target_include_directories(HaThemeEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(HaThemeEngine
    PUBLIC
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::Svg
)

set_target_properties(HaThemeEngine PROPERTIES
        AUTOMOC ON
        AUTORCC ON
)
