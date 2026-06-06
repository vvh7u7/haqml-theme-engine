#pragma once

#include <QQmlEngine>
#include <QStandardPaths>
#include <ThemeEngine/ThemeParser.hpp>

namespace ThemeEngine {

class ThemeManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantMap meta READ meta NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap colors READ colors NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap spacing READ spacing NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap radius READ radius NOTIFY themeChanged)
    Q_PROPERTY(QVariantMap components READ components NOTIFY themeChanged)

public:
    static ThemeManager* create(const QQmlEngine* qmlEngine, const QJSEngine* jsEngine);
    static ThemeManager* instance();

    explicit ThemeManager(QObject* parent = nullptr);

    Q_INVOKABLE bool loadTheme(const QString& path);
    Q_INVOKABLE bool loadThemeFromJson(const QString& jsonString);
    Q_INVOKABLE void resetToDefault();
    Q_INVOKABLE void setAppResourcePrefix(const QString& prefix);
    void setBaseIconsPath(const QString& path);

    Q_INVOKABLE QColor color(const QString& name) const;
    Q_INVOKABLE int spacing(const QString& name) const;
    Q_INVOKABLE int radius(const QString& name) const;
    Q_INVOKABLE QVariantMap component(const QString& name) const;
    Q_INVOKABLE QString icon(const QString& name) const;

    QVariantMap colors() const;
    QVariantMap spacing() const;
    QVariantMap radius() const;
    QVariantMap components() const;
    QVariantMap meta() const;

    Q_INVOKABLE void setColor(const QString& name, const QColor& color);
    Q_INVOKABLE void setSpacing(const QString& name, int value);
    Q_INVOKABLE void setRadius(const QString& name, int value);

    Q_INVOKABLE QString exportTheme() const;
    Q_INVOKABLE bool saveTheme(const QString& path) const;

    Q_INVOKABLE bool mergeTheme(const QString& path);
    Q_INVOKABLE void pushTheme();
    Q_INVOKABLE bool popTheme();

signals:
    void themeChanged();
    void themeLoadError(const QString& error);
    void colorChanged(const QString& name);
    void spacingChanged(const QString& name);
    void radiusChanged(const QString& name);

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
};

} // namespace ThemeEngine