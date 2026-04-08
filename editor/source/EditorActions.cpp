#include "ThemeEditor/EditorActions.hpp"

#include <QFileDialog>
#include <QUrl>
#include <QDebug>

EditorActions::EditorActions(QObject *parent) {

}

void EditorActions::openJsonFile() {
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        tr("Open Json File with theme"),
        QString(),
        tr("JSON files (*.json);;Only files (*)")
    );

    if (!filePath.isEmpty()) {
        emit fileSelected(filePath);
        qDebug() << "Selected file:" << filePath;
    } else {
        emit errorOccurred(tr("File not found"));
    }
}

void EditorActions::saveJsonFile(const QString &defaultName) {

}
