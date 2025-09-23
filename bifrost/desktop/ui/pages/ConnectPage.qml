import QtQuick 2.15
import QtQuick.Controls 2.15
import "../views/."
import "."

Item {
    id: connectPage
    visible: true
    property var mediator

    Component.onCompleted: {
        UDPListener.startListening();  // c++ tan fonksiyon cagrisi
    }

    

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 100
        spacing: 10

        CustomConnectionProgressBar {
                width: 300
                height: 400
                id: progressBar
        }
    }

    CustomButton {
        buttonContentText: "İlerle"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 100
        height: progressBar.isConnected ? 40 : 1
        opacity: progressBar.isConnected ? 1 : 0
        Behavior on height { NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }
        Behavior on opacity { NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }

        onClicked: {
            stackView.push("DashboardPage.qml", {mediator: mediator})
            navigationPills.opacity = 1
            navigationPills.visible = true
        }
    }
}
