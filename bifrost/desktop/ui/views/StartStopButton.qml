import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 160
    height: 160

    property bool motorRunning: false

    signal startClicked()
    signal stopClicked()

    Rectangle {
        id: buttonBackground
        anchors.centerIn: parent
        width: 140
        height: 140
        radius: 70
        color: motorRunning ? "#1a1a1a" : "#1a1a1a"
        border.color: motorRunning ? "#ff4444" : "#4CAF50"
        border.width: 4

        // Glow effect
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.color: motorRunning ? "#ff4444" : "#4CAF50"
            border.width: 2
            opacity: 0.3
            scale: 1.1
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: {
                if (motorRunning) {
                    root.stopClicked()
                } else {
                    root.startClicked()
                }
                root.motorRunning = !root.motorRunning
            }

            onPressed: {
                buttonBackground.scale = 0.95
            }

            onReleased: {
                buttonBackground.scale = 1.0
            }
        }

        Behavior on scale {
            NumberAnimation {
                duration: 100
                easing.type: Easing.OutQuad
            }
        }

        // Power icon or text
        Column {
            anchors.centerIn: parent
            spacing: 10

            // Power symbol
            Canvas {
                id: powerIcon
                width: 50
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.strokeStyle = motorRunning ? "#ff4444" : "#4CAF50"
                    ctx.lineWidth = 3
                    ctx.lineCap = "round"

                    ctx.beginPath()
                    ctx.moveTo(width/2, 12)
                    ctx.lineTo(width/2, 28)
                    ctx.stroke()

                    // Arc (circular part of power symbol)

                    ctx.beginPath()
                    ctx.arc(width/2, height/2 + 2, 16, -Math.PI * 0.3, -Math.PI * 0.7, false)
                    ctx.stroke()
                }

                Connections {
                    target: root
                    function onMotorRunningChanged() {
                        powerIcon.requestPaint()
                    }
                }
            }

            Text {
                text: motorRunning ? "STOP" : "START"
                color: motorRunning ? "#ff4444" : "#4CAF50"
                font.pixelSize: 20
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: motorRunning ? "MOTOR RUNNING" : "MOTOR STOPPED"
                color: motorRunning ? "#ff6666" : "#66bb66"
                font.pixelSize: 10
                anchors.horizontalCenter: parent.horizontalCenter
                topPadding: -5
            }
        }

        // Rotating animation when motor is running
        RotationAnimator {
            target: buttonBackground.border
            from: 0
            to: 360
            duration: 2000
            loops: Animation.Infinite
            running: motorRunning
        }
    }

    // Pulse animation
    SequentialAnimation {
        running: motorRunning
        loops: Animation.Infinite

        NumberAnimation {
            target: buttonBackground
            property: "opacity"
            from: 1.0
            to: 0.7
            duration: 800
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            target: buttonBackground
            property: "opacity"
            from: 0.7
            to: 1.0
            duration: 800
            easing.type: Easing.InOutQuad
        }
    }
}

    
 
