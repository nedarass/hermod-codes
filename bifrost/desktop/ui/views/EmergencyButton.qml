import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 180
    height: 180

    signal emergencyStopClicked()

    // Dış Halka (Sarı uyarı halkası)
    Rectangle {
        id: outerRing
        anchors.fill: parent
        radius: width / 2
        color: "#fdd835" // Endüstriyel sarı
        
        // Çizgili uyarı deseni efekti (Opsiyonel basit çizim)
        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.strokeStyle = "#1a1a1a"
                ctx.lineWidth = 10
                for(var i=0; i<360; i+=45) {
                    var rad = i * Math.PI / 180
                    ctx.beginPath()
                    ctx.moveTo(width/2 + Math.cos(rad)*70, height/2 + Math.sin(rad)*70)
                    ctx.lineTo(width/2 + Math.cos(rad)*90, height/2 + Math.sin(rad)*90)
                    ctx.stroke()
                }
            }
        }
    }

    // Ana Kırmızı Buton
    Rectangle {
        id: mainButton
        width: 140
        height: 140
        radius: 70
        color: "#d32f2f"
        anchors.centerIn: parent
        
        border.color: "#b71c1c"
        border.width: 4

        // Parlama efekti
        Rectangle {
            width: 120
            height: 120
            radius: 60
            anchors.centerIn: parent
            color: "transparent"
            border.color: "#ff5252"
            border.width: 2
        }

        Text {
            text: "E-STOP"
            color: "white"
            font.bold: true
            font.pixelSize: 28
            anchors.centerIn: parent
            style: Text.Outline
            styleColor: "black"
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                mainButton.scale = 0.9
                mainButton.color = "#b71c1c" // Daha koyu kırmızı
            }
            onReleased: {
                mainButton.scale = 1.0
                mainButton.color = "#d32f2f"
            }
            onClicked: {
                root.emergencyStopClicked()
            }
        }

        Behavior on scale { NumberAnimation { duration: 50; easing.type: Easing.OutBounce } }
    }
    
    // Altına gölge efekti
    Rectangle {
        width: 140
        height: 140
        radius: 70
        color: "black"
        opacity: 0.5
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 5
        z: -1
    }
}
