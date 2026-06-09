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
    property var currentMetrics: ({})

    function updateCurrent(index) {
        if (backend.hasScan(index)) {
            currentMetrics = backend.metricsAt(index)
            ppiImage.source = "image://ppi/" + index
            statusText.text = "Scan " + (index + 1) + " / " + scanCount + "  —  " + backend.fileName(index)
        } else {
            currentMetrics = {}
            statusText.text = "Loading..."
        }
    }

    onRadarFolderChanged: {
        if (radarFolder != "")
            backend.openFolder(radarFolder)
    }

    onCurrentIndexChanged: {
        if (currentIndex >= 0) {
            backend.navigate(currentIndex)
            updateCurrent(currentIndex)
        }
    }

    Connections {
        target: backend
        function onStatusChanged(message) { statusText.text = message }
        function onFolderReady(count) {
            scanCount = count
            currentIndex = count > 0 ? 0 : -1
            statusText.text = "Found " + count + " files."
        }
        function onScanCached(index) {
            if (index === currentIndex)
                updateCurrent(index)
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

        Image {
            id: ppiImage
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height) - 20
            height: width
            fillMode: Image.PreserveAspectFit
            cache: false
            source: ""
        }

        Text {
            anchors.centerIn: parent
            text: "Radar Visualizer"
            color: clrText
            font.pixelSize: 22
            opacity: 0.3
            visible: currentIndex === -1
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

        Column {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: 16
            }
            spacing: 10

            Text {
                text: "METRICS"
                color: clrText
                font.pixelSize: 10
                opacity: 0.4
            }

            Rectangle { width: parent.width; height: 1; color: clrSeparator }

            // Metric row helper
            component MetricRow: Row {
                property string label: ""
                property string value: "—"
                width: parent.width
                spacing: 0
                Text {
                    width: parent.parent.width * 0.6
                    text: label
                    color: clrText
                    font.pixelSize: 12
                    opacity: 0.6
                }
                Text {
                    text: value
                    color: clrText
                    font.pixelSize: 12
                }
            }

            MetricRow {
                label: "Noise floor"
                value: currentMetrics.meanNoiseFloor !== undefined
                    ? currentMetrics.meanNoiseFloor.toFixed(4) : "—"
            }
            MetricRow {
                label: "Mean SNR"
                value: currentMetrics.meanSnrDb !== undefined
                    ? currentMetrics.meanSnrDb.toFixed(1) + " dB" : "—"
            }
            MetricRow {
                label: "Invalid az."
                value: currentMetrics.invalidAzimuths !== undefined
                    ? currentMetrics.invalidAzimuths : "—"
            }

            Rectangle { width: parent.width; height: 1; color: clrSeparator }

            Text {
                text: "ANOMALIES"
                color: clrText
                font.pixelSize: 10
                opacity: 0.4
            }

            // Anomaly row helper
            component AnomalyRow: Row {
                property string label: ""
                property bool triggered: false
                width: parent.width
                spacing: 0
                Text {
                    width: parent.parent.width * 0.6
                    text: label
                    color: clrText
                    font.pixelSize: 12
                    opacity: 0.6
                }
                Text {
                    text: triggered ? "!" : "OK"
                    color: triggered ? "#cc4444" : "#558855"
                    font.pixelSize: 12
                }
            }

            AnomalyRow {
                label: "Noise"
                triggered: currentMetrics.anomalyNoise === true
            }
            AnomalyRow {
                label: "SNR"
                triggered: currentMetrics.anomalySnr === true
            }
            AnomalyRow {
                label: "Invalid az."
                triggered: currentMetrics.anomalyInvalid === true
            }
        }
    }
}
