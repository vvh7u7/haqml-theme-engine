#pragma once

#include <QObject>
#include <QString>
#include <QDir>

namespace ThemeEngine {

/**
 * @class IconManager
 * @brief Class for managing icon paths and resolving them dynamically.
 * * Provides transparent operation with icon assets (primarily SVGs),
 * allowing custom folders to override base (default) icon paths.
 */
class IconManager : public QObject {
        Q_OBJECT

public:
        /**
         * @brief Constructor for the icon manager.
         * @param parent Parent QObject instance.
         */
        explicit IconManager(QObject* parent = nullptr);

        /**
         * @brief Returns the full local path or QUrl string to the icon.
         * @param iconName The name of the icon (with or without the .svg extension).
         * @return QString Full path to the file as a string, or an empty string if not found.
         */
        Q_INVOKABLE QString getIcon(const QString& iconName) const;

        /**
         * @brief Sets the path to the custom (user-defined) icons directory.
         * @note In the current implementation, this method overrides m_defaultIconsPath.
         * @param path Absolute or relative path to the folder.
         */
        void setCustomIconsPath(const QString &path);

        /**
         * @brief Sets the path to the default icons directory.
         * @note In the current implementation, this method overrides m_customIconsPath.
         * @param path Absolute or relative path to the folder.
         */
        void setDefaultIconsPath(const QString &path);

private:
        QString m_defaultIconsPath; ///< Default path to system/base icons.
        QString m_customIconsPath; ///< Path to user/custom icon overrides.
};

} // namespace ThemeEngine
