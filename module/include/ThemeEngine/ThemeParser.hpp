#ifndef THEMEENGINE_THEMEPARSER_HPP
#define THEMEENGINE_THEMEPARSER_HPP

#include <QObject>
#include <QColor>
#include <QJsonObject>
#include <QHash>
#include <QVariantMap>

namespace ThemeEngine {

struct ThemeData {
    QHash<QString, QColor> colors;
    QHash<QString, int> spacing;
    QHash<QString, int> radius;
    QHash<QString, QVariantMap> components;
    QHash<QString, QString> meta;

    void clear() {
        colors.clear();
        spacing.clear();
        radius.clear();
        components.clear();
        meta.clear();
    }

    bool isEmpty() const {
        return colors.isEmpty() && spacing.isEmpty() && radius.isEmpty();
    }
};

class ThemeParser : public QObject {
    Q_OBJECT

public:
    explicit ThemeParser(QObject* parent = nullptr);

    bool parseFromFile(const QString& path, ThemeData& outData);
    bool parseFromJson(const QString& jsonString, ThemeData& outData);
    bool parseFromJsonObject(const QJsonObject& root, ThemeData& outData);

    static bool validateTheme(const ThemeData& data);
    static QStringList getMissingRequiredFields(const ThemeData& data);

    static QString exportToJson(const ThemeData& data);
    static bool saveToFile(const ThemeData& data, const QString& path);

    static void mergeThemes(ThemeData& base, const ThemeData& overlay);
    static ThemeData createDefaultTheme();

signals:
    void parseError(const QString& error, int line = -1, int column = -1);

private:
    bool parseColors(const QJsonObject& colorsObj, ThemeData& outData);
    bool parseSpacing(const QJsonObject& spacingObj, ThemeData& outData);
    bool parseRadius(const QJsonObject& radiusObj, ThemeData& outData);
    bool parseComponents(const QJsonObject& componentsObj, ThemeData& outData);
    bool parseMeta(const QJsonObject& metaObj, ThemeData& outData);

    static QColor parseColorString(const QString& colorStr, const ThemeData& currentData);
    static int parseIntValue(const QJsonValue& value, int defaultValue = 0);

    void emitError(const QString& error, const QJsonObject& context = QJsonObject());

    QString m_currentFile;
};

} // namespace ThemeEngine

#endif