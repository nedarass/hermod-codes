import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    property color hoverColor: "red"
    property color roundBackgroundColor: "transparent"
    property color roundBorderColor: "transparent"

    id: closeButton
    width: 20
    height: 20
    padding: 12
    font.pixelSize: 16
    font.bold: true
    background: Rectangle {
        id: buttonBackground
        color: roundBackgroundColor
        border.color: roundBorderColor
        border.width: 2
        radius: width/2
    }

    onHoveredChanged: {
        buttonBackground.color = closeButton.hovered ? hoverColor : roundBackgroundColor;
        buttonBackground.border.color = closeButton.hovered ? hoverColor : roundBorderColor;
    }

    onPressed: {
        closeButton.scale = 0.97;
    }
    onReleased: {
        closeButton.scale = 1.0;
    }
}
