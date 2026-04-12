set(SOURCES
    source/ThemeManager.cpp
    source/ThemeParser.cpp
)

set(HEADERS
    include/ThemeEngine/ThemeManager.hpp
    include/ThemeEngine/ThemeParser.hpp
)

add_library(HaThemeEngine STATIC ${SOURCES} ${HEADERS})

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
)

set_target_properties(HaThemeEngine PROPERTIES
    AUTOMOC ON
    AUTORCC ON
)

qt_add_resources(HaThemeEngine themeengine_resources
    PREFIX "/"
    FILES
        themes/Dark.json
)