#include "ThemeEngine/ThemeParser.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QRegularExpression>

namespace ThemeEngine {

ThemeParser::ThemeParser(QObject* parent) : QObject(parent) {}

bool ThemeParser::parseFromFile(const QString& path, ThemeData& outData)
{
    QString cleanPath = path;

    // Strip out standard URL schemes if the path argument comes from QML components like FileDialog
    if (cleanPath.startsWith("qrc:///")) {
        cleanPath = cleanPath.mid(6);
    } else if (cleanPath.startsWith("qrc:/")) {
        cleanPath = cleanPath.mid(3);
    }

    m_currentFile = cleanPath;
    QFile file(cleanPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emitError(QString("Cannot open file: %1").arg(cleanPath));
        return false;
    }
    const QByteArray data = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emitError(QString("Invalid JSON in file: %1").arg(cleanPath));
        return false;
    }
    if (!doc.isObject()) {
        emitError("JSON root must be an object");
        return false;
    }

    const bool result = parseFromJsonObject(doc.object(), outData);
    m_currentFile.clear();
    return result;
}

bool ThemeParser::parseFromJson(const QString& jsonString, ThemeData& outData)
{
    const QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (doc.isNull()) {
        emitError("Invalid JSON string");
        return false;
    }
    if (!doc.isObject()) {
        emitError("JSON root must be an object");
        return false;
    }
    return parseFromJsonObject(doc.object(), outData);
}

bool ThemeParser::parseFromJsonObject(const QJsonObject& root, ThemeData& outData)
{
    outData.clear();

    if (root.contains("meta")) {
        if (!parseMeta(root["meta"].toObject(), outData)) {
            return false;
        }
    }
    if (root.contains("colors")) {
        if (!parseColors(root["colors"].toObject(), outData)) {
            return false;
        }
    }
    if (root.contains("spacing")) {
        if (!parseSpacing(root["spacing"].toObject(), outData)) {
            return false;
        }
    }
    if (root.contains("radius")) {
        if (!parseRadius(root["radius"].toObject(), outData)) {
            return false;
        }
    }
    if (root.contains("components")) {
        if (!parseComponents(root["components"].toObject(), outData)) {
            return false;
        }
    }
    if (root.contains("assets")) {
        if (!parseAssets(root["assets"].toObject(), outData)) {
            return false;
        }
    }

    return true;
}

bool ThemeParser::parseColors(const QJsonObject& colorsObj, ThemeData& outData)
{
    QHash<QString, QString> unresolvedAliases;

    for (auto it = colorsObj.begin(); it != colorsObj.end(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();

        if (!value.isString()) {
            emitError(QString("Color '%1' must be a string").arg(key), colorsObj);
            return false;
        }

        QString colorStr = value.toString();

        if (colorStr.startsWith('#') || colorStr.startsWith("rgb") || QColor::isValidColorName(colorStr)) {
            QColor color = parseColorString(colorStr, outData);
            if (!color.isValid()) {
                emitError(QString("Invalid color value for '%1'").arg(key), colorsObj);
                return false;
            }
            outData.colors[key] = color;
        } else {
            unresolvedAliases[key] = colorStr;
        }
    }

    int maxIterations = unresolvedAliases.size();
    bool progress = true;

    while (!unresolvedAliases.isEmpty() && progress && maxIterations > 0) {
        progress = false;
        --maxIterations;

        auto it = unresolvedAliases.begin();
        while (it != unresolvedAliases.end()) {
            QString aliasKey = it.key();
            QString targetKey = it.value();

            if (outData.colors.contains(targetKey)) {
                outData.colors[aliasKey] = outData.colors[targetKey];
                it = unresolvedAliases.erase(it);
                progress = true;
            } else {
                ++it;
            }
        }
    }

    // Обработка ошибок
    if (!unresolvedAliases.isEmpty()) {
        for (auto it = unresolvedAliases.begin(); it != unresolvedAliases.end(); ++it) {
            emitError(QString("Broken alias or missing base color '%1' for key '%2'")
                      .arg(it.value(), it.key()), colorsObj);
        }
        return false;
    }

    return true;
}

bool ThemeParser::parseSpacing(const QJsonObject& spacingObj, ThemeData& outData)
{
    for (auto it = spacingObj.begin(); it != spacingObj.end(); ++it) {
        int value = parseIntValue(it.value(), -1);
        if (value < 0) {
            emitError(QString("Invalid spacing value for '%1'").arg(it.key()), spacingObj);
            return false;
        }
        outData.spacing[it.key()] = value;
    }
    return true;
}

bool ThemeParser::parseRadius(const QJsonObject& radiusObj, ThemeData& outData)
{
    for (auto it = radiusObj.begin(); it != radiusObj.end(); ++it) {
        int value = parseIntValue(it.value(), -1);
        if (value < 0) {
            emitError(QString("Invalid radius value for '%1'").arg(it.key()), radiusObj);
            return false;
        }
        outData.radius[it.key()] = value;
    }
    return true;
}

bool ThemeParser::parseComponents(const QJsonObject& componentsObj, ThemeData& outData)
{
    for (auto it = componentsObj.begin(); it != componentsObj.end(); ++it) {
        if (!it.value().isObject()) {
            emitError(QString("Component '%1' must be an object").arg(it.key()), componentsObj);
            return false;
        }
        outData.components[it.key()] = it.value().toObject().toVariantMap();
    }
    return true;
}

bool ThemeParser::parseMeta(const QJsonObject& metaObj, ThemeData& outData)
{
    for (auto it = metaObj.begin(); it != metaObj.end(); ++it) {
        if (it.value().isString()) {
            outData.meta[it.key()] = it.value().toString();
        }
    }
    return true;
}

bool ThemeParser::parseAssets(const QJsonObject& assetsObj, ThemeData& outData)
{
    for (auto it = assetsObj.begin(); it != assetsObj.end(); ++it) {
        if (it.value().isString()) {
            outData.assets[it.key()] = it.value().toString();
        }
    }
    return true;
}

QColor ThemeParser::parseColorString(const QString& colorStr, const ThemeData& currentData)
{
     // HEX Format parsing support (#FFF, #RRGGBB, #AARRGGBB)
    if (colorStr.startsWith('#')) {
        QColor color(colorStr);
        if (color.isValid()) return color;
    }

    // Standard W3C named colors support (e.g., "red", "transparent")
    QColor namedColor(colorStr);
    if (namedColor.isValid()) return namedColor;
    
    // Cross-referencing alias lookup matching previously parsed colors within the active configuration
    if (currentData.colors.contains(colorStr)) {
        return currentData.colors[colorStr];
    }

    // Functional CSS descriptor string parsing support: rgb(255, 255, 255)
    QRegularExpression rgbRegex("rgb\\((\\d+),\\s*(\\d+),\\s*(\\d+)\\)");
    QRegularExpressionMatch match = rgbRegex.match(colorStr);
    if (match.hasMatch()) {
        return QColor(match.captured(1).toInt(), match.captured(2).toInt(), match.captured(3).toInt());
    }
    return QColor();
}

int ThemeParser::parseIntValue(const QJsonValue& value, int defaultValue)
{
    if (value.isDouble()) return static_cast<int>(value.toDouble());
    if (value.isString()) {
        bool ok;
        int result = value.toString().toInt(&ok);
        if (ok) return result;
    }
    return defaultValue;
}

void ThemeParser::emitError(const QString& error, const QJsonObject& context)
{
    Q_UNUSED(context)
    qWarning() << "[ThemeParser] Error:" << error;
    if (!m_currentFile.isEmpty()) {
        qWarning() << "  File:" << m_currentFile;
    }
    emit parseError(error);
}

bool ThemeParser::validateTheme(const ThemeData& data)
{
    // Ensure critical color configuration requirements are satisfied
    QStringList requiredColors = {"primary", "background", "textPrimary"};
    for (const QString& color : requiredColors) {
        if (!data.colors.contains(color)) {
            qWarning() << "Missing required color:" << color;
            return false;
        }
    }

    // Ensure critical spacing scale model requirements are satisfied
    QStringList requiredSpacing = {"xs", "s", "m", "l"};
    for (const QString& spacing : requiredSpacing) {
        if (!data.spacing.contains(spacing)) {
            qWarning() << "Missing required spacing:" << spacing;
            return false;
        }
    }
    return true;
}

QStringList ThemeParser::getMissingRequiredFields(const ThemeData& data)
{
    QStringList missing;
    QStringList requiredColors = {"primary", "background", "textPrimary"};
    for (const QString& color : requiredColors) {
        if (!data.colors.contains(color)) missing << "colors." + color;
    }
    QStringList requiredSpacing = {"xs", "s", "m", "l"};
    for (const QString& spacing : requiredSpacing) {
        if (!data.spacing.contains(spacing)) missing << "spacing." + spacing;
    }
    return missing;
}

QString ThemeParser::exportToJson(const ThemeData& data)
{
    QJsonObject root;
    if (!data.meta.isEmpty()) {
        QJsonObject metaObj;
        for (auto it = data.meta.begin(); it != data.meta.end(); ++it) {
            metaObj[it.key()] = it.value();
        }
        root["meta"] = metaObj;
    }
    if (!data.colors.isEmpty()) {
        QJsonObject colorsObj;
        for (auto it = data.colors.begin(); it != data.colors.end(); ++it) {
            colorsObj[it.key()] = it.value().name(QColor::HexArgb);
        }
        root["colors"] = colorsObj;
    }
    if (!data.spacing.isEmpty()) {
        QJsonObject spacingObj;
        for (auto it = data.spacing.begin(); it != data.spacing.end(); ++it) {
            spacingObj[it.key()] = it.value();
        }
        root["spacing"] = spacingObj;
    }
    if (!data.radius.isEmpty()) {
        QJsonObject radiusObj;
        for (auto it = data.radius.begin(); it != data.radius.end(); ++it) {
            radiusObj[it.key()] = it.value();
        }
        root["radius"] = radiusObj;
    }
    if (!data.components.isEmpty()) {
        QJsonObject componentsObj;
        for (auto it = data.components.begin(); it != data.components.end(); ++it) {
            componentsObj[it.key()] = QJsonObject::fromVariantMap(it.value());
        }
        root["components"] = componentsObj;
    }
    if (!data.assets.isEmpty()) {
        QJsonObject assetsObj;
        for (auto it = data.assets.begin(); it != data.assets.end(); ++it) {
            assetsObj[it.key()] = it.value();
        }
        root["assets"] = assetsObj;
    }

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

bool ThemeParser::saveToFile(const ThemeData& data, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot save theme to:" << path;
        return false;
    }
    file.write(exportToJson(data).toUtf8());
    return true;
}

void ThemeParser::mergeThemes(ThemeData& base, const ThemeData& overlay)
{
    for (auto it = overlay.colors.begin(); it != overlay.colors.end(); ++it) {
        base.colors[it.key()] = it.value();
    }
    for (auto it = overlay.spacing.begin(); it != overlay.spacing.end(); ++it) {
        base.spacing[it.key()] = it.value();
    }
    for (auto it = overlay.radius.begin(); it != overlay.radius.end(); ++it) {
        base.radius[it.key()] = it.value();
    }
    for (auto it = overlay.components.begin(); it != overlay.components.end(); ++it) {
        base.components[it.key()] = it.value();
    }
    for (auto it = overlay.meta.begin(); it != overlay.meta.end(); ++it) {
        base.meta[it.key()] = it.value();
    }
    for (auto it = overlay.assets.begin(); it != overlay.assets.end(); ++it) {
        base.assets[it.key()] = it.value();
    }
}

// ЕБАНОЕ ГРЯЗНОЕ ДЕРЬМО

/**
 * @brief Creates a default theme layout structure as a fallback on asset setup failures.
 * @note Note to developers: contains hardcoded core values matching the base Material Dark palette.
 */
ThemeData ThemeParser::createDefaultTheme()
{
    ThemeData data;
    data.colors["primary"] = QColor("#2196F3");
    data.colors["background"] = QColor("#121212");
    data.colors["surface"] = QColor("#1E1E1E");
    data.colors["textPrimary"] = QColor("#FFFFFF");
    data.colors["textSecondary"] = QColor("#B0B0B0");
    data.spacing["xs"] = 4;
    data.spacing["s"] = 8;
    data.spacing["m"] = 16;
    data.spacing["l"] = 24;
    data.spacing["xl"] = 32;
    data.radius["s"] = 4;
    data.radius["m"] = 8;
    data.radius["l"] = 12;
    data.meta["name"] = "DefaultTheme";
    data.meta["version"] = "1.0";
    data.assets["icons"] = "material";

    return data;
}

} // namespace ThemeEngine
