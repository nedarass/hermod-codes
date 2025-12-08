import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import "ui/views/."
import "ui/pages/."

ApplicationWindow {
    id: window
    visible: true
    width: 1200
    height: 800
    flags: Qt.FramelessWindowHint | Qt.Window
    color: "black"

    title: "Hermod Interface"

    NavigationMediator { id: navMediator }

    Rectangle {
        id: navMed
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        color: "#111"

        Row {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            anchors.rightMargin: 8

            TitleBarRoundButton {
                roundBackgroundColor: "white"
                roundBorderColor: "white"
                hoverColor: "gray"
                onClicked: window.showNormal()
            }

            TitleBarRoundButton {
                roundBackgroundColor: "white"
                roundBorderColor: "white"
                hoverColor: "gray"
                onClicked: Qt.quit()
            }
        }
    }

    StackView {
        id: stackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: navMed.bottom
        initialItem: EntryPage {
            mediator: navMediator
            stackViewRef: stackView
        }
    }
}




