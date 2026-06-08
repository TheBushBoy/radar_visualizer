import QtQuick
import QtQuick.Window
import QtQuick.Dialogs

Window {
    readonly property color clrBackground: "#111111"
    readonly property color clrSurface: "#1a1a1a"
    readonly property color clrPpiPanel: "#141414"
    readonly property color clrMetrics: "#161616"
    readonly property color clrSeparator: "#2d2d2d"
    readonly property color clrText: "#aaaaaa"
    readonly property color clrBtnActive: "#2a2a2a"
    readonly property color clrBtnInactive: "#222222"

    property url radarFolder: ""
    property int scanCount: 0
    property int currentIndex: -1

    onRadarFolderChanged: {
        if (radarFolder != "")
            backend.scanFolder(radarFolder)
    }

    Connections {
        target: backend
        function onStatusChanged(message) { statusText.text = message }
        function onScanComplete(count) {
            scanCount = count
            currentIndex = count > 0 ? 0 : -1
            statusText.text = "Scan complete: " + count + " files."
        }
    }

    width: 1200
    height: 800
    title: "Radar Visualizer"
    visible: true
    color: clrBackground

    // Top menu
    Rectangle {
        id: topMenu
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 56
        color: clrSurface

        // Open folder button
        Rectangle {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 16
            }
            width: 130
            height: 32
            radius: 4
            color: openArea.containsMouse ? clrBtnActive : clrBtnInactive
            border.color: clrSeparator
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "Open folder"
                color: clrText
                font.pixelSize: 13
            }

            MouseArea {
                id: openArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: folderDialog.open()
            }
        }

        // Previous button
        Rectangle {
            id: prevBtn
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 162
            }
            width: 32
            height: 32
            radius: 4
            property bool active: currentIndex > 0
            color: active && prevArea.containsMouse ? clrBtnActive : clrBtnInactive
            border.color: clrSeparator
            border.width: 1
            opacity: active ? 1.0 : 0.3

            Text {
                anchors.centerIn: parent
                text: "<"
                color: clrText
                font.pixelSize: 14
            }

            MouseArea {
                id: prevArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: prevBtn.active
                onClicked: currentIndex--
            }
        }

        // Next button
        Rectangle {
            id: nextBtn
            anchors {
                verticalCenter: parent.verticalCenter
                left: prevBtn.right
                leftMargin: 4
            }
            width: 32
            height: 32
            radius: 4
            property bool active: currentIndex >= 0 && currentIndex < scanCount - 1
            color: active && nextArea.containsMouse ? clrBtnActive : clrBtnInactive
            border.color: clrSeparator
            border.width: 1
            opacity: active ? 1.0 : 0.3

            Text {
                anchors.centerIn: parent
                text: ">"
                color: clrText
                font.pixelSize: 14
            }

            MouseArea {
                id: nextArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: nextBtn.active
                onClicked: currentIndex++
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Select radar PNG folder"
        onAccepted: {
            radarFolder = selectedFolder
        }
    }

    // Footer
    Rectangle {
        id: footer
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: 32
        color: clrSurface

        Text {
            id: statusText
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 16
            }
            text: "Ready."
            color: clrText
            font.pixelSize: 12
        }
    }

    // PPI panel
    Rectangle {
        id: ppiPanel
        anchors {
            top: topMenu.bottom
            bottom: footer.top
            left: parent.left
        }
        width: parent.width * 3 / 4
        color: clrPpiPanel

        Rectangle {
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            width: 1
            color: clrSeparator
        }

        Text {
            anchors.centerIn: parent
            text: "PPI Display"
            color: clrText
            font.pixelSize: 22
        }
    }

    // Metrics panel
    Rectangle {
        id: metricsPanel
        anchors {
            top: topMenu.bottom
            bottom: footer.top
            right: parent.right
        }
        width: parent.width / 4
        color: clrMetrics

        Text {
            anchors.centerIn: parent
            text: "Metrics"
            color: clrText
            font.pixelSize: 22
        }
    }
}
