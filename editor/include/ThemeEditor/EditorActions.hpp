#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

class EditorActions : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit EditorActions(QObject* parent = nullptr);

    Q_INVOKABLE void openJsonFile();
    Q_INVOKABLE void saveJsonFile(const QString& defaultName = QString());

signals:
    void fileSelected(const QString& filePath);
    void fileSaved(const QString& filePath);
    void errorOccurred(const QString& errorMessage);
};

