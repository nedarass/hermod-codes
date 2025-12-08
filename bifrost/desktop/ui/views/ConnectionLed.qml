import QtQuick 2.15

Item {
    id: root
    width: 100
    height: 30

    property bool active: false
    property string label: "STATUS"

    Row {
        anchors.centerIn: parent
        spacing: 8

        // Durum LED'i
        Rectangle {
            width: 10
            height: 10
            radius: 5
            color: root.active ? "#00ff00" : "#555"

            // Neon Parlama Efekti
            layer.enabled: root.active
            Rectangle {
                anchors.centerIn: parent
                width: 16; height: 16; radius: 8
                color: "transparent"
                border.color: "#00ff00"
                opacity: 0.5
            }
        }

        Text {
            text: root.label
            font.pixelSize: 12
            font.bold: true
            color: root.active ? "#00ff00" : "#555"
        }
    }
}
