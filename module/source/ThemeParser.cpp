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
    m_currentFile = path;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emitError(QString("Cannot open file: %1").arg(path));
        return false;
    }
    const QByteArray data = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emitError(QString("Invalid JSON in file: %1").arg(path));
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
    return true;
}

bool ThemeParser::parseColors(const QJsonObject& colorsObj, ThemeData& outData)
{
    for (auto it = colorsObj.begin(); it != colorsObj.end(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();
        if (!value.isString()) {
            emitError(QString("Color '%1' must be a string").arg(key), colorsObj);
            return false;
        }
        QColor color = parseColorString(value.toString(), outData);
        if (!color.isValid()) {
            emitError(QString("Invalid color value for '%1'").arg(key), colorsObj);
            return false;
        }
        outData.colors[key] = color;
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

QColor ThemeParser::parseColorString(const QString& colorStr, const ThemeData& currentData)
{
    if (colorStr.startsWith('#')) {
        QColor color(colorStr);
        if (color.isValid()) return color;
    }
    QColor namedColor(colorStr);
    if (namedColor.isValid()) return namedColor;
    if (currentData.colors.contains(colorStr)) {
        return currentData.colors[colorStr];
    }
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
    QStringList requiredColors = {"primary", "background", "textPrimary"};
    for (const QString& color : requiredColors) {
        if (!data.colors.contains(color)) {
            qWarning() << "Missing required color:" << color;
            return false;
        }
    }
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
}

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
    return data;
}

} // namespace ThemeEngine