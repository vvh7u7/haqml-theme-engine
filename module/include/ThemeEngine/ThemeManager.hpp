#pragma once

#include <QQmlEngine>
#include <QStandardPaths>
#include <ThemeEngine/ThemeParser.hpp>

namespace ThemeEngine {

/**
 * @class ThemeManager
 * @brief Main singleton theme manager exported to QML.
 * * Manages the current theme state of the application, exposes properties for bindings
 * in the QML interface, supports a theme stack (push/pop), and configuration merging.
 */
class ThemeManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /** @brief Metadata about the theme (name, version, author, etc.) */
    Q_PROPERTY(QVariantMap meta READ meta NOTIFY themeChanged)
    /** @brief Theme color map (e.g., "primary" -> QColor) */
    Q_PROPERTY(QVariantMap colors READ colors NOTIFY themeChanged)
    /** @brief Spacing and layout grid map (e.g., "m" -> 16) */
    Q_PROPERTY(QVariantMap spacing READ spacing NOTIFY themeChanged)
    /** @brief Corner radius map (e.g., "s" -> 4) */
    Q_PROPERTY(QVariantMap radius READ radius NOTIFY themeChanged)
    /** @brief Custom UI property mappings for specific individual components */
    Q_PROPERTY(QVariantMap components READ components NOTIFY themeChanged)
public:
    /** Factory method to integrate the singleton into the QML engine */
    static ThemeManager* create(const QQmlEngine* qmlEngine, const QJSEngine* jsEngine);
    /** Returns the global pointer to the manager instance */
    static ThemeManager* instance();

    explicit ThemeManager(QObject* parent = nullptr);

    /** @brief Loads a theme from a JSON file at the specified path (supports QRC) */
    Q_INVOKABLE bool loadTheme(const QString& path);
    /** @brief Parses and applies a theme directly from a raw JSON string */
    Q_INVOKABLE bool loadThemeFromJson(const QString& jsonString);
    /** @brief Resets the current configuration back to the hardcoded default theme */
    Q_INVOKABLE void resetToDefault();
    /** @brief Sets a custom resource path prefix for the application (e.g., ":/my_resources/") */
    Q_INVOKABLE void setAppResourcePrefix(const QString& prefix);
    /** @brief Changes the base path for resolving icon assets */
    void setBaseIconsPath(const QString& path);

    // Atomic getters for convenient C++ and QML invocations
    Q_INVOKABLE QColor color(const QString& name) const;
    Q_INVOKABLE int spacing(const QString& name) const;
    Q_INVOKABLE int radius(const QString& name) const;
    Q_INVOKABLE QVariantMap component(const QString& name) const;
    /** @brief Resolves an icon based on the current theme and returns its correct path/url */
    Q_INVOKABLE QString icon(const QString& name) const;

    // Getters for Q_PROPERTIES
    QVariantMap colors() const;
    QVariantMap spacing() const;
    QVariantMap radius() const;
    QVariantMap components() const;
    QVariantMap meta() const;

    // Setters for dynamic "on-the-fly" runtime theme adjustments
    Q_INVOKABLE void setColor(const QString& name, const QColor& color);
    Q_INVOKABLE void setSpacing(const QString& name, int value);
    Q_INVOKABLE void setRadius(const QString& name, int value);

    /** @brief Exports the current active theme into a formatted JSON string */
    Q_INVOKABLE QString exportTheme() const;
    /** @brief Saves the current theme configuration to a file on disk */
    Q_INVOKABLE bool saveTheme(const QString& path) const;

    /** @brief Overlays (merges) properties from another theme file on top of the current one */
    Q_INVOKABLE bool mergeTheme(const QString& path);
    /** @brief Pushes a snapshot of the current theme onto the state stack */
    Q_INVOKABLE void pushTheme();
    /** @brief Pops and restores the previous theme state from the stack */
    Q_INVOKABLE bool popTheme();

signals:
    void themeChanged();                         ///< Triggered on any global theme state alteration
    void themeLoadError(const QString& error);   ///< Signals a JSON parsing or theme validation failure
    void colorChanged(const QString& name);      ///< Triggered when a specific color is updated
    void spacingChanged(const QString& name);    ///< Triggered when a specific spacing is updated
    void radiusChanged(const QString& name);     ///< Triggered when a specific radius is updated

private:
    void applyThemeData(const ThemeData& data);
    void emitDataChanged();

    QString resolveQrcPath(const QString& relativePath) const;

    QString m_appResourcePrefix = ":/app/";

    ThemeData m_currentTheme;
    QList<ThemeData> m_themeStack;
    ThemeParser m_parser;
    static ThemeManager* s_instance;
    QString m_baseIconsPath;
    QString m_externalIconsPath;

    mutable QHash<QString, QString> m_iconCache;
};

} // namespace ThemeEngine
