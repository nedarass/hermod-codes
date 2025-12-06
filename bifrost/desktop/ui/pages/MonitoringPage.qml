// ui/pages/MonitoringPage.qml
/* import QtQuick 2.15
import QtQuick.Controls 2.15
import "../views/."

// MonitoringPage arayüzü: sistemden gelen telemetri verilerini gösterir
Item {
    id: monitoringPage
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "black"
        border.color: "white"
        border.width: 2
        radius: 8

        Column {           
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 20
            anchors.leftMargin: 20
            spacing: 15

             // --- Fren durumu göstergesi ---
            Row {
                spacing: 8
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    text: "Brake Status:"
                    color: "white"
                    font.pixelSize: 13
                    verticalAlignment: Text.AlignVCenter
                }

                 // BrakeLed -> views klasöründe tanımlı özel bileşen
                BrakeLed {
                    id: brakeLed
                    anchors.verticalCenter: parent.verticalCenter
                }


        }
             // --- TCP bağlantı durumu göstergesi ---
            Row {
                id: commRow
                spacing: 8

                Text {
                      text: "Communication Status:"
                       color: "white"
                       font.pixelSize: 16
                        verticalAlignment: Text.AlignVCenter
                    }

                    // Bağlı LED
                    ConnectionLed {
                        id: connectedLed
                        active: true
                        activeColor: "green"
                        inactiveColor: "gray"
                     }

                    // Bağlı değil LED
                    ConnectionLed {
                        id: disconnectedLed
                        active: false
                        activeColor: "red"
                        inactiveColor: "gray"
                    }
                }

            // Test butonu (şu an veri yoksa deneme amaçlı)
            Button {
                text: "Simulate Brake Toggle"
                onClicked: brakeLed.brakeEngaged = !brakeLed.brakeEngaged
            }
        }
    }
/*
// ---------------------------------------------
    // SPEED GAUGE + POSITION BAR (üst orta)
    // ---------------------------------------------
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 40
        spacing: 30

        // -------- SPEED GAUGE --------
        SpeedGauge {
            id: speedGauge
            width: 200
            height: 200
            maxSpeed: 250

            // Test animasyonu
            SequentialAnimation on speed {
                running: true
                loops: Animation.Infinite
                NumberAnimation { from: 0; to: 200; duration: 4000 }
                NumberAnimation { from: 200; to: 0; duration: 4000 }
            }
        }

        // -------- POSITION BAR --------
        PositionBar {
            id: positionBar
            width: 700
            height: 80
            totalLength: 200

            SequentialAnimation on currentPosition {
                running: true
                loops: Animation.Infinite

                NumberAnimation { from: 0; to: 150; duration: 4000 }
                NumberAnimation { from: 150; to: 200; duration: 2000 }
                PauseAnimation { duration: 800 }
                PropertyAction { target: positionBar; property: "currentPosition"; value: 0 }
            }
        }
    }
/*

            // ----- Termometre -------
            Rectangle {
                id: thermometerRoot
                width: 135
                height: 240
                color: "#1a1a1a"
                radius: 8

                // Sağ üst köşeye sabitleme
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 20
                anchors.topMargin: 20

                property real temperature: 120  // Test değeri
                property real minTemp: 0
                property real maxTemp: 150
                property real warningTemp: 100
                property real criticalTemp: 130

                Column {
                    anchors.centerIn: parent
                    spacing: 12

                    // Başlık
                    Text {
                        text: "Temparature"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#ffffff"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Simge
                    Item {
                        width: 38
                        height: 135
                        anchors.horizontalCenter: parent.horizontalCenter
                        clip: true  // Taşan çizgileri kırp

                        // Termometre gövdesi
                        Rectangle {
                            id: tube
                            width: 15
                            height: 113
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: 15
                            color: "#2a2a2a"
                            border.color: "#4a4a4a"
                            border.width: 1
                            radius: 10

                            // Sıcaklık dolgusu
                            Rectangle {
                                id: mercury
                                anchors.bottom: parent.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: parent.width - 6
                                height: {
                                    var range = thermometerRoot.maxTemp - thermometerRoot.minTemp;
                                    var tempRatio = (thermometerRoot.temperature - thermometerRoot.minTemp) / range;
                                    return Math.max(0, Math.min(1, tempRatio)) * (parent.height - 6);
                                }
                                anchors.bottomMargin: 3
                                radius: 8

                                gradient: Gradient {
                                    GradientStop {
                                        position: 0.0
                                        color: thermometerRoot.temperature >= thermometerRoot.criticalTemp ? "#ff0000" :
                                               thermometerRoot.temperature >= thermometerRoot.warningTemp ? "#ff6600" : "#a8d8ff"
                                    }
                                    GradientStop {
                                        position: 1.0
                                        color: thermometerRoot.temperature >= thermometerRoot.criticalTemp ? "#cc0000" :
                                               thermometerRoot.temperature >= thermometerRoot.warningTemp ? "#cc4400" : "#7cc3ff"
                                    }
                                }

                                Behavior on height {
                                    NumberAnimation { duration: 500; easing.type: Easing.OutQuad }
                                }
                            }

                        }

                        // Termometre ampulü
                        Rectangle {
                            id: bulb
                            width: 21
                            height: 21
                            radius: 11
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            color: thermometerRoot.temperature >= thermometerRoot.criticalTemp ? "#ff0000" :
                                   thermometerRoot.temperature >= thermometerRoot.warningTemp ? "#ff6600" : "#a8d8ff"
                            border.color: "#4a4a4a"
                            border.width: 1

                            Behavior on color {
                                ColorAnimation { duration: 300 }
                            }
                        }
                    }

                    // Dijital değer göstergesi
                    Rectangle {
                        width: 97
                        height: 37
                        color: "#2a2a2a"
                        border.color: "#4a4a4a"
                        border.width: 1
                        radius: 4
                        anchors.horizontalCenter: parent.horizontalCenter

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                text: thermometerRoot.temperature.toFixed(1) + " °C"
                                font.pixelSize: 17
                                font.bold: true
                                color: thermometerRoot.temperature >= thermometerRoot.criticalTemp ? "#ff0000" :
                                       thermometerRoot.temperature >= thermometerRoot.warningTemp ? "#ff6600" : "#a8d8ff"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: thermometerRoot.temperature >= thermometerRoot.criticalTemp ? "CRITICAL!" :
                                      thermometerRoot.temperature >= thermometerRoot.warningTemp ? "WARNING" : "NORMAL"
                                font.pixelSize: 8
                                color: thermometerRoot.temperature >= thermometerRoot.criticalTemp ? "#ff0000" :
                                       thermometerRoot.temperature >= thermometerRoot.warningTemp ? "#ff6600" : "#a8d8ff"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        // Yanıp sönen uyarı efekti
                        SequentialAnimation on opacity {
                            running: thermometerRoot.temperature >= thermometerRoot.warningTemp
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 500 }
                            NumberAnimation { to: 1.0; duration: 500 }
                        }
                    }
                }
            }

            // ------ Akım Güç --------

            Rectangle {
                id: powerGaugeRoot
                width: 150
                height: 160
                color: "#1a1a1a"
                radius: 8

                // SAĞ ALT KÖŞEYE yerleştirme
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: 170
                anchors.topMargin: 20

                // Test verileri
                property real currentValue: 45.5  // Akım (A)
                property real maxCurrent: 100.0   // Maksimum akım
                property real powerValue: 3200.0  // Güç (W)
                property real maxPower: 5000.0    // Maksimum güç
                property real voltage: 48.0       // Voltaj (V)

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    // Başlık
                    Text {
                        text: "Motor BMS"
                        font.pixelSize: 11
                        font.bold: true
                        color: "#ffffff"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // Akım Göstergesi
                    Column {
                        width: parent.width
                        spacing: 4

                        Row {
                            width: parent.width

                            Text {
                                text: "Akım"
                                font.pixelSize: 9
                                color: "#aaaaaa"
                                width: parent.width * 0.5
                            }

                            Text {
                                text: powerGaugeRoot.currentValue.toFixed(1) + " A"
                                font.pixelSize: 9
                                font.bold: true
                                color: "#a8d8ff"
                                horizontalAlignment: Text.AlignRight
                                width: parent.width * 0.5
                            }
                        }

                        // Akım Gauge Bar
                        Rectangle {
                            width: parent.width
                            height: 16
                            color: "#2a2a2a"
                            radius: 8
                            border.color: "#4a4a4a"
                            border.width: 1

                            Rectangle {
                                id: currentBar
                                height: parent.height - 4
                                width: {
                                    var ratio = powerGaugeRoot.currentValue / powerGaugeRoot.maxCurrent;
                                    return Math.max(0, Math.min(1, ratio)) * (parent.width - 4);
                                }
                                anchors.left: parent.left
                                anchors.leftMargin: 2
                                anchors.verticalCenter: parent.verticalCenter
                                radius: 6

                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: "#a8d8ff" }
                                    GradientStop { position: 0.7; color: "#8fc8ff" }
                                    GradientStop { position: 1.0; color: "#7cc3ff" }
                                }

                                Behavior on width {
                                    NumberAnimation { duration: 400; easing.type: Easing.OutQuad }
                                }
                            }
                        }
                    }

                    // Güç Göstergesi
                    Column {
                        width: parent.width
                        spacing: 4

                        Row {
                            width: parent.width

                            Text {
                                text: "Güç"
                                font.pixelSize: 9
                                color: "#aaaaaa"
                                width: parent.width * 0.5
                            }

                            Text {
                                text: (powerGaugeRoot.powerValue / 1000).toFixed(2) + " kW"
                                font.pixelSize: 9
                                font.bold: true
                                color: "#a8d8ff"
                                horizontalAlignment: Text.AlignRight
                                width: parent.width * 0.5
                            }
                        }

                        // Güç Gauge Bar
                        Rectangle {
                            width: parent.width
                            height: 16
                            color: "#2a2a2a"
                            radius: 8
                            border.color: "#4a4a4a"
                            border.width: 1

                            Rectangle {
                                id: powerBar
                                height: parent.height - 4
                                width: {
                                    var ratio = powerGaugeRoot.powerValue / powerGaugeRoot.maxPower;
                                    return Math.max(0, Math.min(1, ratio)) * (parent.width - 4);
                                }
                                anchors.left: parent.left
                                anchors.leftMargin: 2
                                anchors.verticalCenter: parent.verticalCenter
                                radius: 6

                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: "#a8d8ff" }
                                    GradientStop { position: 0.7; color: "#8fc8ff" }
                                    GradientStop { position: 1.0; color: "#7cc3ff" }
                                }

                                Behavior on width {
                                    NumberAnimation { duration: 400; easing.type: Easing.OutQuad }
                                }
                            }
                        }
                    }

                    // Voltaj Göstergesi
                    Column {
                        width: parent.width
                        spacing: 4

                        Row {
                            width: parent.width

                            Text {
                                text: "Voltaj"
                                font.pixelSize: 9
                                color: "#aaaaaa"
                                width: parent.width * 0.5
                            }

                            Text {
                                text: powerGaugeRoot.voltage.toFixed(1) + " V"
                                font.pixelSize: 9
                                font.bold: true
                                color: "#a8d8ff"
                                horizontalAlignment: Text.AlignRight
                                width: parent.width * 0.5
                            }
                        }

                        // Voltaj Gauge Bar
                        Rectangle {
                            width: parent.width
                            height: 16
                            color: "#2a2a2a"
                            radius: 8
                            border.color: "#4a4a4a"
                            border.width: 1

                            Rectangle {
                                id: voltageBar
                                height: parent.height - 4
                                width: {
                                    var ratio = powerGaugeRoot.voltage / 60.0;  // 60V max varsayımı
                                    return Math.max(0, Math.min(1, ratio)) * (parent.width - 4);
                                }
                                anchors.left: parent.left
                                anchors.leftMargin: 2
                                anchors.verticalCenter: parent.verticalCenter
                                radius: 6

                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: "#a8d8ff" }
                                    GradientStop { position: 0.7; color: "#8fc8ff" }
                                    GradientStop { position: 1.0; color: "#7cc3ff" }
                                }

                                Behavior on width {
                                    NumberAnimation { duration: 400; easing.type: Easing.OutQuad }
                                }
                            }
                        }
                    }
                }
            }

            // TEST DEĞERLERİNİ DEĞİŞTİRMEK İÇİN:
            // property real currentValue: 75.0    // Akımı 75A yap
            // property real powerValue: 4200.0    // Gücü 4.2kW yap
            // property real voltage: 52.0         // Voltajı 52V yap

    // --- Sinyal bağlantıları ---
    // SerialCommunicator C++ tarafından QML'e aktarılmıştır.
    Connections {
        target: SerialCommunicator
        // Fren durumu değiştiğinde LED güncellenir
        function onBrakeStatusChanged(brake) {
            brakeLed.brakeEngaged = brake
        }
    }

    Connections {
            target: TCPClient
            // TCP bağlantı durumu değiştiğinde LED'ler güncellenir
            function onConnectionStatusChanged(isConnected) {
                connectedLed.active = isConnected
                disconnectedLed.active = !isConnected
            }
        }
}






// MonitoringPage.qml (TEST İÇİN)

/*
Rectangle {
    anchors.fill: parent
    color: "darkgreen" // Siyah olmadığı belli olsun diye koyu yeşil yaptık.

    Label {
        text: "İZLEME SAYFASI BAŞARILI"
        anchors.centerIn: parent
        font.pixelSize: 30
        color: "yellow"
    }
}
*/
