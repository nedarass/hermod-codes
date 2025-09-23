import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import "ui/views/."
import "ui/pages/."

ApplicationWindow {
    id: window
    visible: true
    width: 850
    height: 500
    flags: Qt.FramelessWindowHint | Qt.Window
    title: "EntryPage"
    color: "transparent"


    NavigationMediator { id: navMediator }

    Rectangle {
        id: background
        anchors.fill: parent
        color: "black"
        border.color: "white"
        border.width: 2
        radius: 12

        Rectangle {
            id: titleBar
            anchors.left: parent.left
            anchors.right: parent.right
            height: 45
            color: "transparent"
            radius: 12

            MouseArea { anchors.fill: parent; onPressed: window.startSystemMove() }

            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8
                
                NavigationPillList {
                    id: navigationPills
                    anchors.verticalCenter: parent.verticalCenter
                    height: 45
                    opacity: 0
                    visible: opacity > 0
                    mediator: navMediator
                    anchors.left: parent.left
                    anchors.leftMargin: 10

                    Behavior on opacity { NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }
                }
            }

            Row {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 10
                anchors.rightMargin: 10
                spacing: 5
                
                TitleBarRoundButton {
                    roundBackgroundColor: "white"
                    roundBorderColor: "white"
                    hoverColor: "gray"
                    onClicked: window.visibility = (window.visibility === Window.FullScreen) ? Window.Windowed : Window.FullScreen
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
            anchors.fill: parent
            anchors.topMargin: titleBar.height
            initialItem: EntryPage { mediator: navMediator }
        }
    }
}
