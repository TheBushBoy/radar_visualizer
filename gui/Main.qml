import QtQuick
import QtQuick.Window

Window {
    width: 800
    height: 600
    title: "Radar Visualizer"
    visible: true

    Text {
        anchors.centerIn: parent
        text: "Radar Visualizer"
        font.pixelSize: 32
        color: "white"
    }

    Rectangle {
        anchors.fill: parent
        color: "#000020"
        z: -1
    }
}
