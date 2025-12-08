import QtQuick 2.15

Item {
    id: root
    width: 150
    height: 80

    property real acceleration: 0.0 // m/s²

    Column {
        anchors.centerIn: parent
        spacing: 5

        Text {
            text: "ACCELERATION"
            color: "#666"
            font.pixelSize: 10
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 4

            Text {
                text: root.acceleration.toFixed(2)
                color: "white"
                font.pixelSize: 32
                font.bold: true
            }

            Text {
                text: "m/s²"
                color: "#ff9800"
                font.pixelSize: 14
                font.bold: true
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 6
            }
        }

        // G-Force Çubuğu (Görsel süsleme)
        Rectangle {
            width: 100
            height: 4
            color: "#333"
            radius: 2
            anchors.horizontalCenter: parent.horizontalCenter

            Rectangle {
                height: parent.height
                width: Math.min(parent.width, Math.abs(root.acceleration) * 10) // Basit ölçekleme
                color: "#ff9800"
                radius: 2
                anchors.centerIn: parent
            }
        }
    }
}
