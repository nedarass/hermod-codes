import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 300
    height: 150

    property int speedValue: 0
    property int minSpeed: 0
    property int maxSpeed: 100

    signal speedChanged(int value)

    Column {
        anchors.fill: parent
        spacing: 15

        // Title and value display
        Row {
            width: parent.width
            height: 50

            Text {
                text: "Speed Control: "
                color: "white"
                font.pixelSize: 14
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }


            Rectangle {
                width: 100
                height: 45
                color: "#1a1a1a"
                border.color: "#8fc8ff"
                border.width: 2
                radius: 8
                anchors.verticalCenter: parent.verticalCenter

                Column {
                    anchors.centerIn: parent
                    spacing: 2

                    Text {
                        text: root.speedValue + "%"
                        color: "#8fc8ff"
                        font.pixelSize: 20
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "PWM"
                        color: "#8fc8ff"
                        font.pixelSize: 10
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }

        // Slider container
        Item {
            width: parent.width
            height: 80

            // Track background
            Rectangle {
                id: trackBackground
                width: parent.width
                height: 8
                anchors.verticalCenter: parent.verticalCenter
                color: "#1a1a1a"
                radius: 4
                border.color: "#333333"
                border.width: 1

                // Progress fill
                Rectangle {
                    width: (parent.width - 4) * (root.speedValue / root.maxSpeed)
                    height: parent.height - 4
                    x: 2
                    y: 2
                    radius: 3

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#8fc8ff" }
                        GradientStop { position: 1.0; color: "#66bb66" }
                    }
                }

                // Tick marks
                Row {
                    anchors.fill: parent
                    spacing: (parent.width - 20) / 4
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10

                    Repeater {
                        model: 5
                        Rectangle {
                            width: 2
                            height: 12
                            color: "#555555"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            // Slider handle
            Rectangle {
                id: handle
                width: 30
                height: 30
                radius: 15
                x: (trackBackground.width - width) * (root.speedValue / root.maxSpeed)
                anchors.verticalCenter: trackBackground.verticalCenter
                color: "#4CAF50"
                border.color: "#66bb66"
                border.width: 3

                // Inner circle
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    anchors.centerIn: parent
                    color: "white"
                }

                // Glow effect
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    color: "transparent"
                    border.color: "#4CAF50"
                    border.width: 2
                    opacity: 0.4
                    scale: 1.3
                }

                Behavior on x {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.OutQuad
                    }
                }

                Behavior on scale {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.OutQuad
                    }
                }
            }

            // Mouse area for interaction
            MouseArea {
                id: mouseArea
                anchors.fill: parent

                function updateValue(mouse) {
                    var pos = Math.max(0, Math.min(mouse.x, width))
                    var newValue = Math.round((pos / width) * root.maxSpeed)
                    if (newValue !== root.speedValue) {
                        root.speedValue = newValue
                        root.speedChanged(newValue)
                    }
                }

                onPressed: function(mouse) {
                    handle.scale = 1.2
                    updateValue(mouse)
                }

                onPositionChanged: function(mouse) {
                    if (pressed) {
                        updateValue(mouse)
                    }
                }

                onReleased: {
                    handle.scale = 1.0
                }
            }

            // Min/Max labels
            Row {
                anchors.top: trackBackground.bottom
                anchors.topMargin: 8
                width: parent.width

                Text {
                    text: root.minSpeed + "%"
                    color: "#888888"
                    font.pixelSize: 11
                }

                Item { width: parent.width - 80 }

                Text {
                    text: root.maxSpeed + "%"
                    color: "#888888"
                    font.pixelSize: 11
                }
            }
        }
    }
}
