import QtQuick 2.15
import QtQuick.Controls 2.15
import ThemeEngine 1.0

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    title: "Theme Editor - " + (Theme.meta ? Theme.meta["name"] : "Editor")
    color: Theme.color("background")

    Text {
        anchors.fill: parent
        text: "At the moment, the editor is only in the planning stages, and I'll most likely abandon this idea anyway))"
        color: Theme.color("textPrimary")
        font.pixelSize: 16
    }

    Column {
        anchors.centerIn: parent
        spacing: Theme.spacing.m

        Text {
            text: "Theme Editor"
            color: Theme.color("textPrimary")
            font.pixelSize: 24
        }

        Text {
            text: "Current theme: " + (Theme.meta ? Theme.meta.name : "Unknown")
            color: Theme.color("textSecondary")
        }

        Rectangle {
            width: 200
            height: 100
            color: Theme.color("primary")
            radius: Theme.radius.m

            Text {
                anchors.centerIn: parent
                text: "Primary Color"
                color: Theme.color("textPrimary")
            }
        }
    }

    Component.onCompleted: {
        console.log("Theme Editor Started")
        console.log("Theme:", Theme.meta ? Theme.meta.name : "No theme loaded")
    }
}