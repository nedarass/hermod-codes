import QtQuick 2.15

Rectangle {
    id: root
    width: 180
    height: 170
    color: "#1a1a1a"
    radius: 8
    border.color: "#444"
    border.width: 1

    property real current: 0.0     // Amper
    property real maxCurrent: 100.0
    property real power: 0.0       // Watt
    property real maxPower: 5000.0
    property real voltage: 0.0     // Volt
    property real maxVoltage: 60.0

    Column {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        Text {
            text: "Battery / Motor BMS"
            font.pixelSize: 12
            font.bold: true
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // --- Helper Component: Stat Row ---
        Component {
            id: statRowComponent
            Column {
                property string label: ""
                property string valueText: ""
                property real ratio: 0
                property var barColors: ["#a8d8ff", "#7cc3ff"] // Default colors

                width: parent.width
                spacing: 4

                Row {
                    width: parent.width
                    Text { text: label; font.pixelSize: 10; color: "#aaaaaa"; width: parent.width * 0.4 }
                    Text { text: valueText; font.pixelSize: 10; font.bold: true; color: "white"; width: parent.width * 0.6; horizontalAlignment: Text.AlignRight }
                }
                Rectangle {
                    width: parent.width; height: 10; color: "#2a2a2a"; radius: 5; border.color: "#444"
                    Rectangle {
                        height: parent.height - 2; radius: 4; x: 1; y: 1
                        width: Math.max(0, Math.min(1, ratio)) * (parent.width - 2)
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: barColors[0] }
                            GradientStop { position: 1.0; color: barColors[1] }
                        }
                        Behavior on width { NumberAnimation { duration: 300 } }
                    }
                }
            }
        }

        // 1. Akım (Current)
        Loader {
            sourceComponent: statRowComponent
            width: parent.width
            onLoaded: {
                item.label = "Current"
                item.valueText = root.current.toFixed(1) + " A"
                item.ratio = root.current / root.maxCurrent
            }
            // Özellikler değiştiğinde Loader içini güncelle
            Binding { target: item; property: "ratio"; value: root.current / root.maxCurrent }
            Binding { target: item; property: "valueText"; value: root.current.toFixed(1) + " A" }
        }

        // 2. Güç (Power)
        Loader {
            sourceComponent: statRowComponent
            width: parent.width
            onLoaded: {
                item.label = "Power"
                item.valueText = (root.power / 1000).toFixed(2) + " kW"
                item.ratio = root.power / root.maxPower
            }
            Binding { target: item; property: "ratio"; value: root.power / root.maxPower }
            Binding { target: item; property: "valueText"; value: (root.power / 1000).toFixed(2) + " kW" }
        }

        // 3. Voltaj (Voltage)
        Loader {
            sourceComponent: statRowComponent
            width: parent.width
            onLoaded: {
                item.label = "Voltage"
                item.valueText = root.voltage.toFixed(1) + " V"
                item.ratio = root.voltage / root.maxVoltage
            }
            Binding { target: item; property: "ratio"; value: root.voltage / root.maxVoltage }
            Binding { target: item; property: "valueText"; value: root.voltage.toFixed(1) + " V" }
        }
    }
}
