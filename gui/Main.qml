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
    property var nonRegResult: null
    property string currentStereoPath: ""

    function nrLabel(key) {
        return ({"mean_noise_floor": "Noise floor",
            "mean_snr_db": "Mean SNR",
            "mean_invalid_azimuths": "Invalid az.",
            "anomaly_noise_pct": "Anom. noise",
            "anomaly_snr_pct": "Anom. SNR",
            "anomaly_invalid_pct": "Anom. invalid"})[key] || key
    }

    function updateCurrent(index) {
        if (backend.hasScan(index)) {
            currentMetrics = backend.metricsAt(index)
            currentStereoPath = backend.stereoPath(index)
            ppiImage.source = "image://ppi/" + index
            statusText.text = "Scan " + (index + 1) + " / " + scanCount + "  —  " + backend.fileName(index)
        } else {
            currentMetrics = {}
            currentStereoPath = ""
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
        function onNonRegressionDone(result) { nonRegResult = result }
        function onFolderReady(count) {
            scanCount = count
            ppiImage.source = ""
            currentStereoPath = ""
            currentIndex = -1
            if (count > 0)
                currentIndex = 0
            statusText.text = "Found " + count + " files."
        }
        function onScanCached(index) {
            if (index === currentIndex)
                updateCurrent(index)
        }
    }

    width: 1200
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    title: "Radar Visualizer"
    visible: true
    color: clrBackground

    Item {
        anchors.fill: parent
        focus: true
        Keys.onLeftPressed: if (currentIndex > 0) currentIndex--
        Keys.onRightPressed: if (currentIndex < scanCount - 1) currentIndex++
    }

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

        Rectangle {
            anchors {
                verticalCenter: parent.verticalCenter
                left: nextBtn.right
                leftMargin: 16
            }
            width: 1
            height: 24
            color: clrSeparator
            id: navSeparator
        }

        // Record reference button
        Rectangle {
            id: recordBtn
            anchors {
                verticalCenter: parent.verticalCenter
                left: navSeparator.right
                leftMargin: 16
            }
            width: 110
            height: 32
            radius: 4
            property bool active: scanCount > 0
            color: active && recordArea.containsMouse ? clrBtnActive : clrBtnInactive
            border.color: clrSeparator
            border.width: 1
            opacity: active ? 1.0 : 0.3

            Text {
                anchors.centerIn: parent
                text: "Record ref."
                color: clrText
                font.pixelSize: 13
            }

            MouseArea {
                id: recordArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: recordBtn.active
                onClicked: backend.recordReference()
            }
        }

        // Non-regression button
        Rectangle {
            id: nonRegBtn
            anchors {
                verticalCenter: parent.verticalCenter
                left: recordBtn.right
                leftMargin: 8
            }
            width: 130
            height: 32
            radius: 4
            property bool active: scanCount > 0
            color: active && nonRegArea.containsMouse ? clrBtnActive : clrBtnInactive
            border.color: clrSeparator
            border.width: 1
            opacity: active ? 1.0 : 0.3

            Text {
                anchors.centerIn: parent
                text: "Non-regression"
                color: clrText
                font.pixelSize: 13
            }

            MouseArea {
                id: nonRegArea
                anchors.fill: parent
                hoverEnabled: true
                enabled: nonRegBtn.active
                onClicked: backend.runNonRegression()
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
            width: Math.min(parent.width, parent.height) - 48
            height: width
            fillMode: Image.PreserveAspectFit
            cache: false
            source: ""
        }

        Repeater {
            model: [0, 45, 90, 135, 180, 225, 270, 315]
            delegate: Text {
                property real rad: modelData * Math.PI / 180
                property real r: ppiImage.width / 2 + 16
                x: ppiPanel.width / 2 + r * Math.sin(rad) - width / 2
                y: ppiPanel.height / 2 - r * Math.cos(rad) - height / 2
                text: modelData + "°"
                color: clrText
                font.pixelSize: 10
                opacity: 0.5
                visible: currentIndex !== -1
            }
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

    // Metrics and stereo panel
    Rectangle {
        id: metricsPanel
        anchors {
            top: topMenu.bottom
            bottom: footer.top
            right: parent.right
        }
        width: parent.width / 4
        color: clrMetrics

        // Stereo camera image
        Rectangle {
            id: stereoSection
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
            height: parent.width
            color: "#0d0d0d"

            Rectangle {
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height: 1
                color: clrSeparator
            }

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: currentStereoPath !== "" ? ("file://" + currentStereoPath) : ""
                cache: false
            }

            Text {
                anchors.centerIn: parent
                visible: currentStereoPath === ""
                text: "No preview image"
                color: clrText
                font.pixelSize: 11
                opacity: 0.2
            }
        }

        // Metrics pannel area
        Flickable {
            anchors { top: parent.top; left: parent.left; right: parent.right; bottom: stereoSection.top }
            contentHeight: metricsColumn.height + 32
            contentWidth: width
            clip: true

        Column {
            id: metricsColumn
            x: 16
            y: 16
            width: parent.width - 32
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

            Rectangle { width: parent.width; height: 1; color: clrSeparator }

            Text {
                text: "NON-REGRESSION"
                color: clrText
                font.pixelSize: 10
                opacity: 0.4
            }

            // Summary
            Text {
                visible: nonRegResult !== null
                text: nonRegResult ? (nonRegResult.passed + "/" + nonRegResult.total + " passed") : ""
                color: nonRegResult && nonRegResult.overall ? "#558855" : "#cc4444"
                font.pixelSize: 12
                font.bold: true
            }
            Text {
                visible: nonRegResult === null
                text: "—"
                color: clrText
                font.pixelSize: 12
                opacity: 0.3
            }

            // Per-metric rows
            component NrRow: Row {
                property string key: ""
                property var d: nonRegResult && nonRegResult.metrics ? nonRegResult.metrics[key] : null
                visible: nonRegResult !== null
                width: parent.width
                spacing: 0
                Text {
                    width: parent.parent.width * 0.55
                    text: nrLabel(parent.key)
                    color: clrText
                    font.pixelSize: 11
                    opacity: 0.6
                }
                Text {
                    width: parent.parent.width * 0.25
                    text: parent.d ? ("+" + parent.d.delta_pct.toFixed(1) + "%") : ""
                    color: clrText
                    font.pixelSize: 11
                    opacity: 0.5
                }
                Text {
                    text: parent.d ? (parent.d.pass ? "OK" : "FAIL") : ""
                    color: parent.d ? (parent.d.pass ? "#558855" : "#cc4444") : clrText
                    font.pixelSize: 11
                }
            }

            NrRow { key: "mean_noise_floor" }
            NrRow { key: "mean_snr_db" }
            NrRow { key: "mean_invalid_azimuths" }
            NrRow { key: "anomaly_noise_pct" }
            NrRow { key: "anomaly_snr_pct" }
            NrRow { key: "anomaly_invalid_pct" }
        }
        } // Flickable
    }
}
