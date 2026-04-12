import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

//import EditorActions 1.0

QtObject {
    id: root

    property Action newFile: Action {
        text: qsTr("&New File")
        shortcut: StandardKey.New
        icon.name: "document-new"
        onTriggered: {
            console.log("New File")
        }
    }
    property Action openFile: Action {
        text: qsTr("&Open File")
        shortcut: StandardKey.Open
        icon.name: "document-open"
        onTriggered: {
            console.log("Open File")
            let filePath = fileSelector.openJsonFile()
            if (filePath !== "") {
                let jsonData = fileSelector.loadJson(filePath)

                console.log("Файл открыт:", filePath)
                console.log("JSON содержимое:", JSON.stringify(jsonData))

                label.text = "Загружено: " + jsonData["someKey"] || "JSON прочитан"
            } else {
                console.log("Пользователь отменил выбор")
            }
        }
    }
    property Action saveFile: Action {
        text: qsTr("&Save")
        shortcut: StandardKey.Save
        icon.name: "document-save"
        onTriggered: {
            console.log("Save file")
        }
    }
    property Action quit: Action {
        text: qsTr("&Quit")
        shortcut: StandardKey.Quit
        icon.name: "exit"
        onTriggered: {
            console.log("Quit")
            Qt.quit()
        }
    }
}
