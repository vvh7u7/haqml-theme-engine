#include "ThemeEngine/IconManager.hpp"

#include <qcoreapplication.h>
#include <QUrl>

namespace ThemeEngine {

ThemeEngine::IconManager::IconManager(QObject *parent) {
    // By default, icons are looked up in the application launch folder: /assets/icons/material/
    m_defaultIconsPath = QCoreApplication::applicationDirPath() + "/assets/icons/material/";
}

QString IconManager::getIcon(const QString &iconName) const {
     // Automatically append the .svg extension if not specified by the user
    QString svgName = iconName.endsWith(".svg") ? iconName : iconName + ".svg";

    // First, check if the icon exists in the custom directory
    if (!m_customIconsPath.isEmpty()) {
        QFile customFile(m_customIconsPath + svgName);
        if (customFile.exists()) {
            return QUrl::fromLocalFile(customFile.fileName()).toString();
        }
    }

    // If not found in custom, search the default directory
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
