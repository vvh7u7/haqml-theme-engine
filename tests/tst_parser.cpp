#include <QtTest>
#include <QSignalSpy>
#include "ThemeEngine/ThemeParser.hpp"
#include "ThemeEngine/ThemeManager.hpp"

using namespace ThemeEngine;

class TestThemeEngine : public QObject
{
    Q_OBJECT

private slots:
    void init() {
        ThemeManager::instance()->resetToDefault();
    }

    // TEST 1: Check base right JSON
    void test_parseValidJson() {
        QString json = R"({
            "colors": {
                "primary": "#00FF00",
                "background": "#000000",
                "textPrimary": "#FFFFFF"
            },
            "spacing": { "xs": 2, "s": 4, "m": 8, "l": 16 }
        })";

        ThemeParser parser;
        ThemeData data;

        QVERIFY(parser.parseFromJson(json, data));
        QVERIFY(ThemeParser::validateTheme(data));
        QCOMPARE(data.colors.value("primary"), QColor("#00FF00"));
        QCOMPARE(data.spacing.value("m"), 8);
    }

    // TEST 2: Alphabet alias
    void test_colorAliasesOrder() {
        QString json = R"({
            "colors": {
                "accent": "primary",
                "background": "#000000",
                "primary": "#2196F3",
                "textPrimary": "#FFFFFF"
            },
            "spacing": { "xs": 4, "s": 8, "m": 16, "l": 24 }
        })";

        ThemeParser parser;
        ThemeData data;

        QVERIFY(parser.parseFromJson(json, data));
        QCOMPARE(data.colors.value("accent"), QColor("#2196F3"));
    }

    // TEST 3: (Slots & Signals) QSignalSpy
    void test_themeManagerSignals() {
        ThemeManager* manager = ThemeManager::instance();

        QSignalSpy spyChanged(manager, &ThemeManager::themeChanged);
        QSignalSpy spyColor(manager, &ThemeManager::colorChanged);

        manager->setColor("primary", QColor("#FF0000"));

        QCOMPARE(spyChanged.count(), 1);
        QCOMPARE(spyColor.count(), 1);

        QList<QVariant> arguments = spyColor.takeFirst();
        QCOMPARE(arguments.at(0).toString(), QString("primary"));
    }

    // TEST 4: Stack work (Push / Pop)
    void test_themeStack() {
        ThemeManager* manager = ThemeManager::instance();
        manager->setColor("primary", QColor("#111111"));

        manager->pushTheme();

        manager->setColor("primary", QColor("#999999"));
        QCOMPARE(manager->color("primary"), QColor("#999999"));

        QVERIFY(manager->popTheme());

        QCOMPARE(manager->color("primary"), QColor("#111111"));
    }
};

QTEST_MAIN(TestThemeEngine);
#include "tst_parser.moc"