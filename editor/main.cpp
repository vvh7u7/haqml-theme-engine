
//#include <QGuiApplication>
#include <QApplication>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <ThemeEngine/ThemeManager.hpp>
#include <ThemeEditor/EditorActions.hpp>
#include <ThemeEditor/FileSelector.hpp>

using namespace ThemeEngine;

int main(int argc, char *argv[])
{
    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    qmlRegisterType<EditorActions>("EditorActions", 1, 0, "EditorActions");

    ThemeEditor::FileSelector* fileSelector = new ThemeEditor::FileSelector();

    qmlRegisterSingletonType<ThemeManager>("ThemeEngine", 1, 0, "Theme",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            auto* manager = ThemeManager::instance();
            manager->loadTheme(":/themes/Dark.json");

            return manager;
        }
    );

    engine.load(QUrl("qrc:/qml/Main.qml"));
    engine.rootContext()->setContextProperty("fileSelector", fileSelector);

    return app.exec();
}