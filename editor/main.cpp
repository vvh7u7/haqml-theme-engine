#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <ThemeEngine/ThemeManager.hpp>

using namespace ThemeEngine;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterSingletonType<ThemeManager>("ThemeEngine", 1, 0, "Theme",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            auto* manager = ThemeManager::instance();
            manager->loadTheme(":/themes/Dark.json");

            return manager;
        }
    );

    QQmlApplicationEngine engine;
    engine.load(QUrl("qrc:/qml/Main.qml"));

    return app.exec();
}