#ifndef THEMEEDITOR_FILESELECTOR_HPP
#define THEMEEDITOR_FILESELECTOR_HPP

#include <QObject>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>


namespace ThemeEditor {

class FileSelector : public QObject {
    Q_OBJECT

public:
    explicit FileSelector(QObject *parent = nullptr);

    Q_INVOKABLE QString openJsonFile();
    Q_INVOKABLE QVariantMap loadJson(const QString& filePath);
};

}


#endif//FILESELECTOR_HPP