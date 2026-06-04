#pragma once

#include <QObject>
#include <QString>
#include <QDir>

namespace ThemeEngine {

class IconManager : public QObject {
        Q_OBJECT

public:
        explicit IconManager(QObject* parent = nullptr);

        Q_INVOKABLE QString getIcon(const QString& iconName) const;

        void setCustomIconsPath(const QString &path);

        void setDefaultIconsPath(const QString &path);

private:
        QString m_defaultIconsPath;
        QString m_customIconsPath;
};

} // namespace ThemeEngine