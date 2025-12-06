import QtQuick 2.15

Item {
    id: root
    width: 100
    height: 240

    // Dışarıdan kontrol edilecek özellikler
    property real temperature: 0
    property real minTemp: 0
    property real maxTemp: 150
    property real warningTemp: 100
    property real criticalTemp: 130

    Column {
        anchors.centerIn: parent
        spacing: 12

        // Başlık
        Text {
            text: "Temperature"
            font.pixelSize: 14
            font.bold: true
            color: "#ffffff"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Simge ve Tüp
        Item {
            width: 38
            height: 135
            anchors.horizontalCenter: parent.horizontalCenter
            clip: true

            // Termometre gövdesi (Tüp)
            Rectangle {
                width: 15
                height: 113
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 15
                color: "#2a2a2a"
                border.color: "#4a4a4a"
                border.width: 1
                radius: 10

                // Cıva (Sıcaklık dolgusu)
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 6
                    height: {
                        var range = root.maxTemp - root.minTemp;
                        var tempRatio = (root.temperature - root.minTemp) / range;
                        return Math.max(0, Math.min(1, tempRatio)) * (parent.height - 6);
                    }
                    anchors.bottomMargin: 3
                    radius: 8

                    color: root.temperature >= root.criticalTemp ? "#ff0000" :
                           root.temperature >= root.warningTemp ? "#ff6600" : "#a8d8ff"

                    Behavior on height { NumberAnimation { duration: 500; easing.type: Easing.OutQuad } }
                    Behavior on color { ColorAnimation { duration: 300 } }
                }
            }

            // Termometre Ampulü (Alt yuvarlak)
            Rectangle {
                width: 21
                height: 21
                radius: 11
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                color: root.temperature >= root.criticalTemp ? "#ff0000" :
                       root.temperature >= root.warningTemp ? "#ff6600" : "#a8d8ff"
                border.color: "#4a4a4a"
                border.width: 1
                Behavior on color { ColorAnimation { duration: 300 } }
            }
        }

        // Dijital Değer Kutusu
        Rectangle {
            width: 90
            height: 40
            color: "#1a1a1a"
            border.color: root.temperature >= root.warningTemp ? (root.temperature >= root.criticalTemp ? "red" : "orange") : "#4a4a4a"
            border.width: 1
            radius: 4
            anchors.horizontalCenter: parent.horizontalCenter

            Column {
                anchors.centerIn: parent
                Text {
                    text: root.temperature.toFixed(1) + " °C"
                    font.pixelSize: 16
                    font.bold: true
                    color: "white"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: root.temperature >= root.criticalTemp ? "CRITICAL" :
                          root.temperature >= root.warningTemp ? "WARNING" : "NORMAL"
                    font.pixelSize: 9
                    color: root.temperature >= root.criticalTemp ? "#ff0000" :
                           root.temperature >= root.warningTemp ? "#ff6600" : "#aaaaaa"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}
