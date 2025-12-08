import QtQuick 2.15

Item {
    id: root
    width: 120
    height: 40

    property bool engaged: false // Dışarıdan kontrol edilecek durum

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: root.engaged ? "#330000" : "#1a1a1a" // Aktifken koyu kırmızı zemin
        border.color: root.engaged ? "#ff3333" : "#444"
        border.width: 1

        Row {
            anchors.centerIn: parent
            spacing: 8

            // LED Işığı
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: root.engaged ? "#ff0000" : "#333"
                border.color: root.engaged ? "#ffcccc" : "transparent"

                // Yanıp sönme efekti (Fren çekiliyken)
                SequentialAnimation on opacity {
                    running: root.engaged
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.4; duration: 500 }
                    NumberAnimation { from: 0.4; to: 1.0; duration: 500 }
                }
            }

            Text {
                text: root.engaged ? "BRAKES ON" : "BRAKES OFF"
                font.pixelSize: 12
                font.bold: true
                color: root.engaged ? "#ff3333" : "#666"
            }
        }
    }
}
