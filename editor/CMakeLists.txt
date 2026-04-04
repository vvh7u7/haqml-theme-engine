add_executable(ThemeEditor
        main.cpp
)

target_link_libraries(ThemeEditor
        PRIVATE
        HaThemeEngine
        Qt6::Quick
)

qt_add_resources(ThemeEditor "editor_qrc"
        PREFIX "/"
        FILES
        qml/Main.qml
)