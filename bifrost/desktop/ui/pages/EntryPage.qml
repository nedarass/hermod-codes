import QtQuick 2.15
import "../views/."
import "."

Item {
    anchors.fill: parent
    property var mediator

    Column {
        anchors.top: parent.top
        anchors.topMargin: 50
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10

        Image {
            width: 120
            height: 120
            source: "../../images/logo_app_inverted.png"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Hermod Bifröst"
            font.pixelSize: 24
            font.bold: true
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Kontrol ve izleme yazılımı."
            font.pixelSize: 16
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    CustomButton {
        height:40
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 100
        buttonContentText: "Hyperloop'a Bağlanın"
        onClicked: stackView.push("ConnectPage.qml", {mediator: mediator})
    }
}
