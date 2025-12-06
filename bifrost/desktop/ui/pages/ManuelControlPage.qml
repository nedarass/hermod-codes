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

       // --- 1. GRUP: Motor Kontrolleri ---

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

        // --- 2. GRUP: Fren Sistemi ---

        // Mechanical Brake Button
        BrakeButton {
            id: brakeButton
            anchors.horizontalCenter: parent.horizontalCenter
            
            onBrakeEngaged: {
                console.log("Mekanik frenler SIKILDI")
                // Eğer motor çalışıyorsa güvenli duruş için motoru kesmek isteyebilirsin:
                if(motorButton.motorRunning) {
                     console.log("Uyarı: Motor çalışırken fren yapıldı!")
                     // motorButton.stopClicked() // Opsiyonel: Otomatik motor durdurma
                }
                statusText.text = "Frenler Devrede"
                statusText.color = "#FF9800"
            }
            
            onBrakeReleased: {
                console.log("Mekanik frenler BIRAKILDI")
                statusText.text = "Frenler Serbest"
            }
        }

        // Biraz boşluk bırakarak Acil Durum butonunu ayıralım
        Item { width: 1; height: 20 }

       // --- 3. GRUP: Acil Durum ---

        EmergencyButton {
            id: emergencyButton
            anchors.horizontalCenter: parent.horizontalCenter
            
            onEmergencyStopClicked: {
                console.log("!!! ACİL DURDURMA TETİKLENDİ !!!")
                
                // 1. Motoru anında kes
                if(motorButton.motorRunning) {
                    motorButton.stopClicked()
                    motorButton.motorRunning = false
                }
                
                // 2. Hızı sıfırla
                speedSlider.speedValue = 0
                
                // 3. Frenleri kilitle
                if(!brakeButton.brakesEngaged) {
                    brakeButton.brakesEngaged = true
                    brakeButton.brakeEngaged()
                }

                statusText.text = "ACİL DURDURMA AKTİF!"
                statusText.color = "red"
            }
        }
    }
}
