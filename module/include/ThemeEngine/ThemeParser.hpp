#pragma once

#include <QObject>
#include <QColor>
#include <QJsonObject>
#include <QHash>
#include <QVariantMap>

namespace ThemeEngine {

/**
 * @struct ThemeData
 * @brief Container structure holding all deserialized theme configuration properties.
 */
struct ThemeData {
    int standardVersion = 0;
    QHash<QString, QColor> colors;           ///< Color palette values
    QHash<QString, int> spacing;             ///< Sizing and spacing dimensions
    QHash<QString, int> radius;              ///< Element corner radiuses
    QHash<QString, QVariantMap> components;  ///< Component-specific UI properties
    QHash<QString, QString> meta;            ///< Informational metadata (name, version)
    QHash<QString, QString> assets;          ///< Names and aliases of asset packs/icons

    /** @brief Fully clears the structure contents */
    void clear() {
        colors.clear();
        spacing.clear();
        radius.clear();
        components.clear();
        meta.clear();
        assets.clear();
    }

    /** @brief Checks if the theme is empty (evaluates critical UI elements only) */
    bool isEmpty() const {
        return colors.isEmpty() && spacing.isEmpty() && radius.isEmpty();
    }
};

/**
 * @class ThemeParser
 * @brief Utility parser class providing JSON schema validation, import, and export.
 */
class ThemeParser : public QObject {
    Q_OBJECT

public:
    static constexpr int CurrentStandardVersion = 1;

    explicit ThemeParser(QObject* parent = nullptr);

    /** @brief Parses a theme mapping structure from a physical file or QRC resource path */
    bool parseFromFile(const QString& path, ThemeData& outData);
    /** @brief Parses a theme mapping structure out of a raw, unformatted JSON string */
    bool parseFromJson(const QString& jsonString, ThemeData& outData);
    /** @brief Parses a theme mapping directly from an active QJsonObject tree node */
    bool parseFromJsonObject(const QJsonObject&root, ThemeData &outData);

    /** @brief Validates if the given theme object contains all critical required layout keys */
    static bool validateTheme(const ThemeData& data);
    /** @brief Returns a path list of missing mandatory fields (for convenient error tracing) */
    static QStringList getMissingRequiredFields(const ThemeData& data);

    /** @brief Serializes a ThemeData instance into a pretty-printed JSON string format */
    static QString exportToJson(const ThemeData& data);
    /** @brief Writes the given theme structure out onto a disk target location as JSON */
    static bool saveToFile(const ThemeData& data, const QString& path);

    /** @brief Merges two themes, overriding existing base keys with overlay values */
    static void mergeThemes(ThemeData& base, const ThemeData& overlay);
    
    /** @brief Creates and returns a hardcoded "Default Theme" configuration fallback block */
    static ThemeData createDefaultTheme();

signals:
    /** @brief Emitted when encountering syntax or structural faults during JSON decoding */
    void parseError(const QString& error, int line = -1, int column = -1);

private:
    // Internal node processing parsers
    bool parseColors(const QJsonObject& colorsObj, ThemeData& outData);
    bool parseSpacing(const QJsonObject& spacingObj, ThemeData& outData);
    bool parseRadius(const QJsonObject& radiusObj, ThemeData& outData);
    bool parseComponents(const QJsonObject& componentsObj, ThemeData& outData);
    bool parseMeta(const QJsonObject& metaObj, ThemeData& outData);
    bool parseAssets(const QJsonObject& assetsObj, ThemeData& outData);

    // Primitive element decoders
    static QColor parseColorString(const QString& colorStr, const ThemeData& currentData);
    static int parseIntValue(const QJsonValue& value, int defaultValue = 0);

    void emitError(const QString& error, const QJsonObject& context = QJsonObject());

    QString m_currentFile; ///< Track active path of the file currently processing (for debug logging).
};

} // namespace ThemeEngine
