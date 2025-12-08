import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    // Varsayılan boyutlar (Dışarıdan override edilebilir)
    implicitWidth: 600
    implicitHeight: 60

    // --- Özellikler (Properties) ---
    // Bu değerleri C++ veya diğer QML dosyalarından güncelleyeceksin
    property real currentPosition: 0.0  // Aracın anlık konumu (metre)
    property real maxDistance: 200.0    // Pistin toplam uzunluğu (metre)

    // Renk Ayarları
    property color trackColor: "#2d3436"       // Pist arka plan rengi
    property color progressColor: "#00b894"    // İlerleme çubuğu rengi
    property color indicatorColor: "#ffffff"   // Araç (Pod) rengi

    Rectangle {
        id: trackContainer
        anchors.centerIn: parent
        width: parent.width
        height: 12
        color: root.trackColor
        radius: height / 2

        // 1. Dolum Çubuğu (Gidilen mesafe kadar dolar)
        Rectangle {
            id: progressBar
            height: parent.height
            // Genişlik hesabı: (Anlık Konum / Toplam Yol) * Çubuk Genişliği
            width: (root.currentPosition / root.maxDistance) * parent.width
            color: root.progressColor
            radius: parent.radius

            // Konum aniden değişirse yumuşak geçiş yapması için animasyon
            Behavior on width {
                NumberAnimation { duration: 100; easing.type: Easing.Linear }
            }
        }

        // 2. Pod Göstergesi (Hareket eden yuvarlak/ikon)
        Rectangle {
            id: podIndicator
            width: 24
            height: 24
            radius: width / 2
            color: root.indicatorColor
            border.color: root.progressColor
            border.width: 3

            anchors.verticalCenter: parent.verticalCenter

            // X konumu: Progress bar'ın bittiği yer, ama dışarı taşmasın diye ufak bir ayar
            x: progressBar.width - (width / 2)

            // Gölge efekti (Opsiyonel, şıklık katar)
            layer.enabled: true

            Behavior on x {
                NumberAnimation { duration: 100; easing.type: Easing.Linear }
            }
        }
    }

    // 3. Metin Bilgisi (Opsiyonel: Pod'un altında metreyi yazar)
    Text {
        anchors.top: trackContainer.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: trackContainer.left
        // Metni Pod ile birlikte hareket ettirmek için margin kullanıyoruz
        anchors.horizontalCenterOffset: (root.currentPosition / root.maxDistance) * root.width

        text: Math.floor(root.currentPosition) + "m"
        color: "#dfe6e9"
        font.pixelSize: 14
        font.bold: true
    }

    // Başlangıç ve Bitiş Etiketleri
    Text {
        anchors.bottom: trackContainer.top
        anchors.bottomMargin: 5
        anchors.left: trackContainer.left
        text: "0m"
        color: "#636e72"
        font.pixelSize: 12
    }

    Text {
        anchors.bottom: trackContainer.top
        anchors.bottomMargin: 5
        anchors.right: trackContainer.right
        text: root.maxDistance + "m"
        color: "#636e72"
        font.pixelSize: 12
    }
}
