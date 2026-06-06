#include "ThemeEngine/ThemeManager.hpp"

#include <qcoreapplication.h>
#include <QDebug>
#include <QDir>

namespace ThemeEngine {

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::create(const QQmlEngine* qmlEngine, const QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent) : QObject(parent)
{
    connect(&m_parser, &ThemeParser::parseError, this, &ThemeManager::themeLoadError);
    //m_baseIconsPath = QCoreApplication::applicationDirPath() + "/assets/themes/icons/";
    m_baseIconsPath = ":/assets/themes/icons/";
    m_externalIconsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/icons/";
    resetToDefault();
}

bool ThemeManager::loadTheme(const QString& path)
{
    ThemeData newData;
    if (!m_parser.parseFromFile(path, newData)) {
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

void ThemeManager::setBaseIconsPath(const QString &path) {
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
    return true;
}

void ThemeManager::applyThemeData(const ThemeData& data)
{
    m_currentTheme = data;
    emitDataChanged();
}

void ThemeManager::emitDataChanged()
{
    emit themeChanged();
}

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
    QString svgName = name.endsWith(".svg") ? name : name + ".svg";

    if (m_currentTheme.assets.contains("icons")) {
        QString customDirName = m_currentTheme.assets["icons"];
        QString customPath = m_externalIconsPath + customDirName + "/" + svgName;

        if (QFile::exists(customPath)) {
            return QUrl::fromLocalFile(customPath).toString();
        }
    }

    QString externalDefaultPath = m_externalIconsPath + "material/" + svgName;
    if (QFile::exists(externalDefaultPath)) {
        return QUrl::fromLocalFile(externalDefaultPath).toString();
    }

    if (!m_baseIconsPath.startsWith(":/")) {
        QString localPath = m_baseIconsPath + "material/" + svgName;
        if (QFile::exists(localPath)) {
            return QUrl::fromLocalFile(localPath).toString();
        }
    }

    QString qrcPath = m_baseIconsPath + "material/" + svgName;
    if (QFile::exists(qrcPath)) {
        QString finalUrl = qrcPath;
        finalUrl.replace(0, 1, "qrc://");
        return finalUrl;
    }

    qWarning() << "[ThemeManager] Icon absolutely not found:" << name;
    return QString();
}

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
