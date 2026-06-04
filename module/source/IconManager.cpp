#include "ThemeEngine/IconManager.hpp"

#include <qcoreapplication.h>
#include <QUrl>

namespace ThemeEngine {

ThemeEngine::IconManager::IconManager(QObject *parent) {
    m_defaultIconsPath = QCoreApplication::applicationDirPath() + "/assets/icons/material/";
}

QString IconManager::getIcon(const QString &iconName) const {
    QString svgName = iconName.endsWith(".svg") ? iconName : iconName + ".svg";

    if (!m_customIconsPath.isEmpty()) {
        QFile customFile(m_customIconsPath + svgName);
        if (customFile.exists()) {
            return QUrl::fromLocalFile(customFile.fileName()).toString();
        }
    }

    QFile defaultFile(m_defaultIconsPath + svgName);
    if (!defaultFile.exists()) {
        return QUrl::fromLocalFile(defaultFile.fileName()).toString();
    }

    return QString();
}

void ThemeEngine::IconManager::setCustomIconsPath(const QString &path) {
    m_defaultIconsPath = QDir(path).absolutePath() + "/";
}

void ThemeEngine::IconManager::setDefaultIconsPath(const QString &path) {
    m_customIconsPath = QDir(path).absolutePath() + "/";
}

} // namespace ThemeEngine