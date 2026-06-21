#include "ThemeEngine/ThemeManager.hpp"

#include <qcoreapplication.h>
#include <QDebug>
#include <QDir>
#include <qqml.h>

namespace ThemeEngine {

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::create(const QQmlEngine* qmlEngine, const QJSEngine* jsEngine)
{
    Q_UNUSED(jsEngine)
    if (!s_instance) {
        s_instance = new ThemeManager(const_cast<QQmlEngine*>(qmlEngine));
    }
    return s_instance;
}

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager(QCoreApplication::instance());
    }
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent) : QObject(parent)
{
    // Forward parser errors to the manager's public interface
    connect(&m_parser, &ThemeParser::parseError, this, &ThemeManager::themeLoadError);
    
    //m_baseIconsPath = QCoreApplication::applicationDirPath() + "/assets/themes/icons/";
    m_baseIconsPath = ":/assets/themes/icons/";
    m_externalIconsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/icons/";
    
    // Initialize with the default theme configuration on startup
    resetToDefault();
}

bool ThemeManager::loadTheme(const QString& path)
{
    QString jsonFileName = path.endsWith(".json") ? path : path + ".json";

    if (jsonFileName.contains("/")) {
        jsonFileName = jsonFileName.section('/', -1);
    }

    QString relativeJsonPath = "assets/themes/" + jsonFileName;
    QString resolvedPath = resolveQrcPath(relativeJsonPath);

    QString finalPathToParse = path;

    if (!resolvedPath.isEmpty()) {
        QUrl url(resolvedPath);
        if (url.scheme() == "qrc") {
            finalPathToParse = ":" + url.path();
        } else {
            finalPathToParse = url.toLocalFile();
        }
        qDebug() << "[ThemeManager] Theme resolved to:" << finalPathToParse;
    }

    ThemeData newData;
    if (!m_parser.parseFromFile(finalPathToParse, newData)) {
        return false;
    }
    
    // Strict schema validation before applying the parsed theme
    if (!ThemeParser::validateTheme(newData)) {
        const QStringList missing = ThemeParser::getMissingRequiredFields(newData);
        emit themeLoadError(QString("Missing required fields: %1").arg(missing.join(", ")));
        return false;
    }
    applyThemeData(newData);
    return true;
}

bool ThemeManager::loadThemeFromJson(const QString& jsonString)
{
    ThemeData newData;
    if (!m_parser.parseFromJson(jsonString, newData)) {
        return false;
    }
    if (!ThemeParser::validateTheme(newData)) {
        const QStringList missing = ThemeParser::getMissingRequiredFields(newData);
        emit themeLoadError(QString("Missing required fields: %1").arg(missing.join(", ")));
        return false;
    }
    applyThemeData(newData);
    return true;
}

void ThemeManager::resetToDefault()
{
    applyThemeData(ThemeParser::createDefaultTheme());
}

void ThemeManager::setAppResourcePrefix(const QString &prefix)
{
    QString cleanPrefix = prefix;
    if (!cleanPrefix.startsWith(":/")) {
        cleanPrefix = ":/" + cleanPrefix;
    }
    if (!cleanPrefix.endsWith("/")) {
        cleanPrefix += "/";
    }
    m_appResourcePrefix = cleanPrefix;
}

void ThemeManager::setBaseIconsPath(const QString &path)
{
    m_baseIconsPath = QDir(path).absolutePath() + "/";
}

bool ThemeManager::mergeTheme(const QString& path)
{
    ThemeData overlay;
    if (!m_parser.parseFromFile(path, overlay)) {
        return false;
    }
    ThemeParser::mergeThemes(m_currentTheme, overlay);
    applyThemeData(m_currentTheme);
    return true;
}

void ThemeManager::pushTheme()
{
    m_themeStack.push_back(m_currentTheme);
}

bool ThemeManager::popTheme()
{
    if (m_themeStack.isEmpty()) {
        return false;
    }
    const ThemeData previous = m_themeStack.takeLast();
    applyThemeData(previous);

    for (const QString& key : m_currentTheme.colors.keys()) { emit colorChanged(key); }
    for (const QString& key : m_currentTheme.spacing.keys()) { emit spacingChanged(key); }
    for (const QString& key : m_currentTheme.radius.keys()) { emit radiusChanged(key); }

    return true;
}

void ThemeManager::applyThemeData(const ThemeData& data)
{
    m_iconCache.clear();
    m_currentTheme = data;
    emitDataChanged();
}

void ThemeManager::emitDataChanged()
{
    emit themeChanged();
}

QString ThemeManager::resolveQrcPath(const QString &relativePath) const
{
    if (!m_appResourcePrefix.isEmpty() && m_appResourcePrefix != ":/") {
        QString appPath = m_appResourcePrefix + relativePath;
        if (QFile::exists(appPath)) {
            return appPath.replace(0, 1, "qrc://");
        }
    }

    QString bqPath = QString(":/haqml/") + relativePath;
    if (QFile::exists(bqPath)) {
        return bqPath.replace(0, 1, "qrc://");
    }

    return QString();
}

// Secure fallback to solid red color if the key is missing from configuration
QColor ThemeManager::color(const QString& name) const
{
    return m_currentTheme.colors.value(name, QColor(Qt::red));
}

int ThemeManager::spacing(const QString& name) const
{
    return m_currentTheme.spacing.value(name, 0);
}

int ThemeManager::radius(const QString& name) const
{
    return m_currentTheme.radius.value(name, 0);
}

QVariantMap ThemeManager::component(const QString& name) const
{
    return m_currentTheme.components.value(name);
}

QString ThemeManager::icon(const QString &name) const {
    if (m_iconCache.contains(name)) {
        return m_iconCache.value(name);
    }

    QString svgName = name.endsWith(".svg") ? name : name + ".svg";

    // Search within external (downloaded/user-defined) icon directories
    if (m_currentTheme.assets.contains("icons")) {
        QString customDirName = m_currentTheme.assets["icons"];
        QString customPath = m_externalIconsPath + customDirName + "/" + svgName;

        if (QFile::exists(customPath)) {
            QString res = QUrl::fromLocalFile(customPath).toString();
            m_iconCache[name] = res;
            return res;
        }
    }

    // Search inside external default icon assets (Material fallback)
    QString externalDefaultPath = m_externalIconsPath + "material/" + svgName;
    if (QFile::exists(externalDefaultPath)) {
        QString res = QUrl::fromLocalFile(externalDefaultPath).toString();
        m_iconCache[name] = res; // Сохраняем в кэш
        return res;
    }

    // Search via local file system paths (if the path string is not QRC)
    if (!m_baseIconsPath.startsWith(":/")) {
        QString localPath = m_baseIconsPath + "material/" + svgName;
        if (QFile::exists(localPath)) {
            QString res = QUrl::fromLocalFile(localPath).toString();
            m_iconCache[name] = res; // Сохраняем в кэш
            return res;
        }
    }

    // Resolve fallback via built-in embedded app resources
    QString relativeIconPath = "assets/themes/icons/material/" + svgName;
    QString resolvedQrc = resolveQrcPath(relativeIconPath);
    if (!resolvedQrc.isEmpty()) {
        m_iconCache[name] = resolvedQrc; // Сохраняем в кэш
        return resolvedQrc;
    }

    // Dynamic search using theme QRC resource path prefixes
    if (m_baseIconsPath.startsWith(":/")) {
        QString dynamicQrcPath = m_baseIconsPath + "material/" + svgName;
        if (QFile::exists(dynamicQrcPath)) {
            QString res = dynamicQrcPath.replace(0, 1, "qrc://");
            m_iconCache[name] = res; // Сохраняем в кэш
            return res;
        }
    }

    qWarning() << "[ThemeManager] Icon absolutely not found:" << name;

    m_iconCache[name] = QString();
    return QString();
}

// Convert internal QHash structures into QVariantMap containers for QML bridging

QVariantMap ThemeManager::colors() const
{
    QVariantMap result;
    for (auto it = m_currentTheme.colors.begin(); it != m_currentTheme.colors.end(); ++it) {
        result[it.key()] = it.value();
    }
    return result;
}

QVariantMap ThemeManager::spacing() const
{
    QVariantMap result;
    for (auto it = m_currentTheme.spacing.begin(); it != m_currentTheme.spacing.end(); ++it) {
        result[it.key()] = it.value();
    }
    return result;
}

QVariantMap ThemeManager::radius() const
{
    QVariantMap result;
    for (auto it = m_currentTheme.radius.begin(); it != m_currentTheme.radius.end(); ++it) {
        result[it.key()] = it.value();
    }
    return result;
}

QVariantMap ThemeManager::components() const
{
    QVariantMap result;
    for (auto it = m_currentTheme.components.begin(); it != m_currentTheme.components.end(); ++it) {
        result[it.key()] = it.value();
    }
    return result;
}

QVariantMap ThemeManager::meta() const
{
    QVariantMap result;
    for (auto it = m_currentTheme.meta.begin(); it != m_currentTheme.meta.end(); ++it) {
        result[it.key()] = it.value();
    }
    return result;
}

void ThemeManager::setColor(const QString& name, const QColor& color)
{
    if (m_currentTheme.colors.value(name) != color) {
        m_currentTheme.colors[name] = color;
        emit colorChanged(name);
        emit themeChanged();
    }
}

void ThemeManager::setSpacing(const QString& name, int value)
{
    if (m_currentTheme.spacing.value(name) != value) {
        m_currentTheme.spacing[name] = value;
        emit spacingChanged(name);
        emit themeChanged();
    }
}

void ThemeManager::setRadius(const QString& name, int value)
{
    if (m_currentTheme.radius.value(name) != value) {
        m_currentTheme.radius[name] = value;
        emit radiusChanged(name);
        emit themeChanged();
    }
}

QString ThemeManager::exportTheme() const
{
    return ThemeParser::exportToJson(m_currentTheme);
}

bool ThemeManager::saveTheme(const QString& path) const
{
    return ThemeParser::saveToFile(m_currentTheme, path);
}

} // namespace ThemeEngine
