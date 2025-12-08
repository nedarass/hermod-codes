import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 200
    height: 60

    property bool brakesEngaged: false

    signal brakeEngaged()
    signal brakeReleased()

    Rectangle {
        id: buttonBackground
        anchors.fill: parent
        radius: 10
        // Frenler çekiliyse Turuncu, değilse Koyu Gri
        color: brakesEngaged ? "#FF9800" : "#2a2a2a"
        border.color: brakesEngaged ? "#FFB74D" : "#555555"
        border.width: 2

        // Glow effect (Hafif parlama)
        layer.enabled: brakesEngaged
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.color: "#FF9800"
            border.width: 2
            opacity: 0.5
            scale: 1.05
            visible: brakesEngaged
        }

        Row {
            anchors.centerIn: parent
            spacing: 15

            // Fren İkonu (Basit çizim)
            Canvas {
                width: 24
                height: 24
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0,0,width,height)
                    ctx.strokeStyle = brakesEngaged ? "black" : "#aaaaaa"
                    ctx.lineWidth = 3
                    ctx.beginPath()
                    ctx.arc(12, 12, 10, 0, Math.PI * 2) // Dış daire
                    ctx.stroke()

                    // Balata kısımları
                    ctx.beginPath()
                    ctx.arc(12, 12, 6, -Math.PI/2, Math.PI/2)
                    ctx.stroke()
                }
                // Durum değişince rengi güncelle
                onVisibleChanged: requestPaint()
            }

            Text {
                text: root.brakesEngaged ? "BRAKES ENGAGED" : "BRAKES RELEASED"
                color: root.brakesEngaged ? "black" : "#aaaaaa"
                font.pixelSize: 16
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.brakesEngaged = !root.brakesEngaged
                if (root.brakesEngaged) {
                    root.brakeEngaged()
                } else {
                    root.brakeReleased()
                }
            }
            onPressed: buttonBackground.scale = 0.95
            onReleased: buttonBackground.scale = 1.0
        }

        Behavior on scale { NumberAnimation { duration: 100 } }
        Behavior on color { ColorAnimation { duration: 200 } }
    }
}
