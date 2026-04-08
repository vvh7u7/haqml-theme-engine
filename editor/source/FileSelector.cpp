#include "ThemeEditor/FileSelector.hpp"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>


namespace ThemeEditor {

FileSelector::FileSelector(QObject *parent): QObject(parent) {  }

QString FileSelector::openJsonFile() {
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        tr("Open Json File with theme"),
        "",
        "JSON files (*.json);;All files (*.*)"
        );

    return filePath;
}

QVariantMap FileSelector::loadJson(const QString &filePath) {
    if (filePath.isEmpty()) { return {}; }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return {}; }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || !jsonDocument.isObject()) { return {}; }

    return jsonDocument.object().toVariantMap();
}

}//namespace ThemeEditor