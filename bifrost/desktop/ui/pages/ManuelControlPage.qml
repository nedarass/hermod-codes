import QtQuick 2.15
import QtQuick.Controls 2.15
import "../views"

Item {
    id: manualControlPage
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#0a0a0a"
    }

    Column {
        anchors.centerIn: parent
        spacing: 30


        // Motor Start/Stop Button
        StartStopButton {
            id: motorButton
            anchors.horizontalCenter: parent.horizontalCenter

            onStartClicked: {
                console.log("Motor başlatılıyor...")
                // TODO: VESC6 motor sürücüsüne START komutu gönderilecek
                // vescInterface.startMotor()
                statusText.text = "Motor başlatıldı"
                statusText.color = "#4CAF50"
            }

            onStopClicked: {
                console.log("Motor durduruluyor...")
                // TODO: VESC6 motor sürücüsüne STOP komutu gönderilecek
                // vescInterface.stopMotor()
                statusText.text = "Motor durduruldu"
                statusText.color = "#ff4444"
            }
        }
        // Speed Control Slider
                SpeedControlSlider {
                    id: speedSlider
                    anchors.horizontalCenter: parent.horizontalCenter

                    onSpeedChanged: function(value) {
                        console.log("PWM değeri değişti: " + value + "%")
                        // TODO: VESC6 motor sürücüsüne PWM değeri gönderilecek
                        // vescInterface.setPWM(value)
                        if (motorButton.motorRunning) {
                            statusText.text = "Motor çalışıyor - Hız: " + value + "%"
                        }
                    }
                }
    }
}
