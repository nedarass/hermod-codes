import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../views"

Item {
    id: monitoringPage
    anchors.fill: parent

    // --- VERİ YAPILARI (Değiştirilmedi) ---
    // Bu değişkenler UI elemanlarına bağlıdır.
    property real sysSpeed: 0.0
    property real sysPosition: 0.0
    property real sysAcceleration: 0.0
    property real sysCurrent: 0.0
    property real sysPower: 0.0
    property real sysVoltage: 48.0 // Başlangıç voltajı
    property real sysTemp: 25.0    // Oda sıcaklığı
    property bool sysBrakeEngaged: true
    property bool sysConnected: false

    // --- TCPClient SİNYAL BAĞLANTILARI (YENİ EKLENDİ) ---
       Connections {
           target: TCPClient

           onSpeedUpdated: function(speed) {
               sysSpeed = speed * 3.6; // m/s -> km/h
           }

           onPositionUpdated: function(position) {
               sysPosition = position;
           }

           onVoltageUpdated: function(voltage) {
               sysVoltage = voltage;
           }

           onTemperatureUpdated: function(temp) {
               sysTemp = temp;
           }

           onBrakeStatusChanged: function(engaged) {
               sysBrakeEngaged = engaged;
           }

           onConnectionChanged: function(connected) {
               sysConnected = connected;
           }


           onAccelerationUpdated: function(accel) {
               sysAcceleration = accel;
           }

           onCurrentUpdated: function(current) {
               sysCurrent = current;
           }

           onPowerUpdated: function(power) {
               sysPower = power;
           }


           onErrorFlagsUpdated: function(flags) {
               if (flags > 0) {
                   console.log("Hata bayrakları: 0x" + flags.toString(16));
               }
           }
       }


    Rectangle {
        anchors.fill: parent
        color: "#0a0a0a"

        // --- 1. ÜST BAR (Durum LED'leri) ---
        Rectangle {
            id: topBar
            width: parent.width
            height: 60
            color: "#111"

            Rectangle {
                            width: parent.width
                            height: 1 // Çizgi kalınlığı
                            color: "#222" // Çizgi rengi
                            anchors.bottom: parent.bottom
                        }


            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20

                ConnectionLed {
                    label: "SYSTEM LINK"
                    active: sysConnected
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "HYPERLOOP TELEMETRY (TEST MODE)"
                    color: "white"
                    font.bold: true
                    font.pixelSize: 18
                    font.letterSpacing: 2
                }

                Item { Layout.fillWidth: true }

                BrakeLed {
                    engaged: sysBrakeEngaged
                }
            }
        }

        // --- 2. ANA PANEL ---
        Item {
            anchors.top: topBar.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20

            // SOL: Güç Göstergeleri
            PowerStats {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                current: sysCurrent
                power: sysPower
                voltage: sysVoltage
            }

            // ORTA: Sürüş Verileri
            Column {
                anchors.centerIn: parent
                spacing: 40

                SpeedGauge {
                    width: 280
                    height: 280
                    speed: sysSpeed
                    maxSpeed: 400
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                AccelerationDisplay {
                    acceleration: sysAcceleration
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                PositionBar {
                    width: 600
                    height: 60
                    currentPosition: sysPosition
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            // SAĞ: Sıcaklık Göstergesi
            ThermometerGauge {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                temperature: sysTemp
            }
        }
    }

    // ============================================================
    //  TEST SİMÜLASYONU (Backend Bağlanınca BURAYI SİL)
    // ============================================================
    /*
    Timer {
        interval: 100 // 100ms'de bir güncelle (10 FPS)
        running: true
        repeat: true

        // Simülasyon mantığı değişkenleri
        property bool accelerating: true

        onTriggered: {
            // 1. Bağlantı var gibi davran
            sysConnected = true;

            // 2. Basit Hız ve İvme Mantığı
            if (sysSpeed >= 350) accelerating = false; // Hızlanma bitti
            if (sysSpeed <= 0) { accelerating = true; sysPosition = 0; } // Durdu, tekrar başla

            if (accelerating) {
                sysBrakeEngaged = false;
                sysAcceleration = 2.5 + (Math.random() - 0.5); // ~2.5 m/s2 ivme
                sysSpeed += sysAcceleration * 0.1; // v = a * t
            } else {
                sysBrakeEngaged = true; // Fren yapıyoruz
                sysAcceleration = -5.0 + (Math.random() - 0.5); // Frenleme
                sysSpeed += sysAcceleration * 0.1;
                if (sysSpeed < 0) sysSpeed = 0;
            }

            // 3. Konum Güncellemesi (x = v * t)
            sysPosition += (sysSpeed * 0.1);
            if(sysPosition > 2000) sysPosition = 0;

            // 4. Batarya ve Güç Simülasyonu
            // Hız arttıkça akım artar, voltaj çok az düşer (sag)
            sysVoltage = 48.0 - (sysCurrent * 0.05) + (Math.random() * 0.1);
            sysCurrent = (sysSpeed / 4) + (Math.random() * 2); // Hıza bağlı akım
            sysPower = sysVoltage * sysCurrent;

            // 5. Sıcaklık
            // Akım çekildikçe ısınır, durunca soğur
            if (sysCurrent > 10) sysTemp += 0.1;
            else sysTemp -= 0.05;
            if(sysTemp > 80) sysTemp = 80;
        }
    }
}  */ }
