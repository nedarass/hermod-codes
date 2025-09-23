import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    property alias buttonContentText: buttonText.text
    id: connectButton
    padding: 12
    font.pixelSize: 16
    font.bold: true
    anchors.horizontalCenter: parent.horizontalCenter

    background: Rectangle {
        id: buttonBackground
        color: "transparent"
        border.color: "white"
        border.width: 2
        radius: 8
        implicitWidth: contentItem.implicitWidth + 40
        implicitHeight: contentItem.implicitHeight + 20
    }

    contentItem: Text {
        id: buttonText
        color: "white"
        font.pixelSize: 16
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    onHoveredChanged: {
        buttonText.color = connectButton.hovered ? "black" : "white";
        buttonBackground.color = connectButton.hovered ? "white" : "black";
    }

    onPressed: { connectButton.scale = 0.97; }
    onReleased: { connectButton.scale = 1.0; }
}
