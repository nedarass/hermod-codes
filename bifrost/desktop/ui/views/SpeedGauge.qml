import QtQuick 2.15

Item {
    id: gaugeItem
    width: 200
    height: 200

    property real speed: 0       // Dışarıdan verilecek hız değeri
    property real maxSpeed: 250  // Maksimum hız sınırı

    Canvas {
        id: speedCanvas
        anchors.fill: parent

        // Hız değiştiğinde yeniden çizim isteği gönder
        onPaint: {
            var ctx = getContext("2d");
            var centerX = width / 2;
            var centerY = height / 2;
            var radius = width / 2 - 10;

            // Göstergenin başlangıç ve bitiş açıları (Sol alttan sağ alta)
            var startAngle = Math.PI * 0.75;
            var endAngle = Math.PI * 2.25;
            var angleRange = endAngle - startAngle;

            ctx.reset();

            // 1. Arka Plan Yayı (Koyu Gri)
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, startAngle, endAngle);
            ctx.lineWidth = 15;
            ctx.strokeStyle = "#333333";
            ctx.lineCap = "round";
            ctx.stroke();

            // 2. Hız Yayı (Renkli)
            // Mevcut hızın açısal karşılığını hesapla
            var currentAngle = startAngle + (angleRange * (gaugeItem.speed / gaugeItem.maxSpeed));

            // Açının bitiş açısını geçmemesini sağla
            if (currentAngle > endAngle) currentAngle = endAngle;

            ctx.beginPath();
            ctx.arc(centerX, centerY, radius, startAngle, currentAngle);
            ctx.lineWidth = 15;

            // Renk geçişi (Turuncu -> Sarı)
            var gradient = ctx.createLinearGradient(0, 0, width, 0);
            gradient.addColorStop(0, "#FF5722"); // Turuncu
            gradient.addColorStop(1, "#FFC107"); // Sarı
            ctx.strokeStyle = gradient;
            ctx.lineCap = "round";
            ctx.stroke();
        }
    }

    // Hız değeri değiştiğinde Canvas'ı güncelle
    onSpeedChanged: speedCanvas.requestPaint()

    // Ortadaki Hız Metni
    Column {
        anchors.centerIn: parent
        Text {
            text: Math.round(gaugeItem.speed)
            color: "white"
            font.pixelSize: 42
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: "km/h"
            color: "#aaaaaa"
            font.pixelSize: 14
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
