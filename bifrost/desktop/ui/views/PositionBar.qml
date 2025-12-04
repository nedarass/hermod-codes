import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    // Varsayılan genişlik (kullanıldığı yerde değişebilir)
    width: 600
    height: 80

    property real currentPosition: 0   // Anlık konum (metre)
    property real totalLength: 200    // Toplam tünel uzunluğu (metre)

    // --- Görselleştirme ---

    // 1. Tünel Hattı (Arka Plan Çizgisi)
    Rectangle {
        id: trackLine
        width: parent.width
        height: 6
        color: "#333333"
        radius: 3
        anchors.centerIn: parent
    }

    // Başlangıç ve Bitiş İşaretçileri (Dik çizgiler)
    Rectangle { width: 2; height: 20; color: "gray"; anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left }
    Rectangle { width: 2; height: 20; color: "gray"; anchors.verticalCenter: parent.verticalCenter; anchors.right: parent.right }

    // 2. Pod (Hareket Eden İmleç)
    Rectangle {
        id: podCursor
        width: 50
        height: 24
        radius: 12
        color: "#FF5722" // Turuncu
        border.color: "white"
        border.width: 2
        anchors.verticalCenter: parent.verticalCenter

        // X koordinatını konuma göre hesapla
        // (x = oran * toplam_genişlik)
        x: (root.currentPosition / root.totalLength) * (root.width - width)

        // Animasyonlu geçiş (Veri anlık sıçrasa bile yumuşak görünsün)
        Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.OutQuad } }

        // Pod üzerindeki ok işareti (Süsleme)
        Text {
            anchors.centerIn: parent
            text: ">"
            color: "white"
            font.bold: true
            font.pixelSize: 14
        }
    }

    // 3. Alt Metin Bilgileri
    Text {
        text: "Konum: " + Math.round(root.currentPosition) + " m"
        color: "white"
        font.pixelSize: 18
        font.bold: true
        anchors.top: trackLine.bottom
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // Mesafe Etiketleri
    Text { text: "0m"; color: "gray"; anchors.bottom: trackLine.top; anchors.bottomMargin: 10; anchors.left: parent.left }
    Text { text: root.totalLength + "m"; color: "gray"; anchors.bottom: trackLine.top; anchors.bottomMargin: 10; anchors.right: parent.right }
}
