import QtQuick 2.15
import QtQuick.Layouts 2.15
import QtQuick.Controls


Item {
    id: navbar
    property var mediator: mediator

    Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            height:50
            spacing: 8

            Button {
                id: monitoringButton
                property int tabIndex: 0
                text: qsTr("İzleme")
                topPadding: 5
                bottomPadding: 5
                rightPadding: 10
                leftPadding: 10
                anchors.verticalCenter: parent.verticalCenter
                onClicked: updateSelection(monitoringButton, "MonitoringPage.qml")
                property bool isSelected: true
                background: Rectangle {
                    color: parent.isSelected ? "white" : "transparent"
                    border.color: "black"
                    border.width: 1
                    radius: 8
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.isSelected ? "black" : "white"
                }

                onHoveredChanged: {
                    monitoringButton.background.border.color = monitoringButton.hovered ? "white" : "black";
                }
            }

            Button {
                id: manuelControlButton
                property int tabIndex: 1
                text: qsTr("Elle Kontrol")
                topPadding: 5
                bottomPadding: 5
                rightPadding: 10
                leftPadding: 10
                anchors.verticalCenter: parent.verticalCenter
                onClicked: updateSelection(manuelControlButton, "ManuelControlPage.qml")
                property bool isSelected: false
                background: Rectangle {
                    color: parent.isSelected ? "white" : "transparent"
                    border.color: "black"
                    radius: 8
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.isSelected ? "black" : "white"
                }

                onHoveredChanged: {
                    manuelControlButton.background.border.color = manuelControlButton.hovered ? "white" : "black";
                }
            }

            Button {
                id: hermodPulseButton
                property int tabIndex: 2
                text: qsTr("Hermod Pulse")
                topPadding: 5
                bottomPadding: 5
                rightPadding: 10
                leftPadding: 10
                anchors.verticalCenter: parent.verticalCenter
                onClicked: updateSelection(hermodPulseButton, "HermodPulsePage.qml")
                property bool isSelected: false
                background: Rectangle {
                    color: parent.isSelected ? "white" : "transparent"
                    border.color: "black"
                    radius: 8
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.isSelected ? "black" : "white"
                }

                onHoveredChanged: {
                    hermodPulseButton.background.border.color = hermodPulseButton.hovered ? "white" : "black";
                }
            }
    }


    function updateSelection(selectedButton, openPage) {
        monitoringButton.isSelected = false
        manuelControlButton.isSelected = false
        hermodPulseButton.isSelected = false

        selectedButton.isSelected = true
        mediator.setPage(openPage)
    }

}
