
//#include <QGuiApplication>
#include <QApplication>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QImageReader>

#include <ThemeEngine/ThemeManager.hpp>
#include <ThemeEditor/EditorActions.hpp>
#include <ThemeEditor/FileSelector.hpp>

using namespace ThemeEngine;

int main(int argc, char *argv[])
{
    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);

    qDebug() << "[ThemeEditor] Starting. Core Theme Standard Version:" << ThemeEngine::ThemeManager::SupportedStandard;
    qDebug() << "[Debug] Supported image formats:" << QImageReader::supportedImageFormats();

    //Решение проблемы с регистрацией ресурсов от статических библиотек
    Q_INIT_RESOURCE(haqml_resources);
    QQmlApplicationEngine engine;

    ThemeManager::instance()->setMaxSupportedStandard(1);

    qmlRegisterType<EditorActions>("EditorActions", 1, 0, "EditorActions");
    ThemeEditor::FileSelector* fileSelector = new ThemeEditor::FileSelector();

    auto* manager = ThemeManager::instance();
    if (!manager->loadTheme(":/assets/themes/Dark.json")) {
        qWarning() << "[Main] Failed to pre-load embedded Dark.json from resources!";
    }

    qmlRegisterSingletonType<ThemeManager>("ThemeEngine", 1, 0, "Theme",
        [](QQmlEngine*, QJSEngine*) -> QObject* {
            return ThemeManager::instance();
        }
    );

    engine.load(QUrl("qrc:/qml/Main.qml"));
    engine.rootContext()->setContextProperty("fileSelector", fileSelector);

    return app.exec();
}