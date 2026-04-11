set(SOURCES
        source/EditorActions.cpp
        source/FileSelector.cpp
)

set(HEADERS
        include/ThemeEditor/EditorActions.hpp
        include/ThemeEditor/FileSelector.hpp
)

add_executable(ThemeEditor
        WIN32
        MACOSX_BUNDLE
        main.cpp
        ${HEADERS}
        ${SOURCES}
)

qt_add_resources(ThemeEditor "editor_qrc"
        PREFIX "/"
        FILES
            qml/Main.qml
            qml/Actions.qml
)

target_include_directories(HaThemeEngine
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
)

target_link_libraries(ThemeEditor
        PRIVATE
            HaThemeEngine
            Qt6::Quick
            Qt6::Widgets
)
