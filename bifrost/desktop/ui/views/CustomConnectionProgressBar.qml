import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 2.15

Item {
    property bool isConnected: TCPClient.isConnected

    Rectangle {
        id: container
        width: isConnected ? 400 : 300
        height: isConnected ? 100 : 120
        radius: 8
        color: isConnected ? "white" : "black"
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        Behavior on color { ColorAnimation { duration: 500 } }

        Loader {
            id: contentLoader
            anchors.fill: parent
            sourceComponent: isConnected ? connectedView : loadingView
        }
    }

    Component {
        id: loadingView
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10

            Text {
                id: headerText
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Bağlantı bekleniyor..."
                font.pixelSize: 24
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Aracı yerel ağa bağlayın."
                font.pixelSize: 16
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                anchors.topMargin: 5
            }

            Item {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 20
                height: 50
            }

            Rectangle {
                id: indeterminateProgress
                width: 40
                height: 40
                radius: 25
                color: "white"
                opacity: 0.5
                anchors.horizontalCenter: parent.horizontalCenter

                SequentialAnimation {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation { target: indeterminateProgress; property: "scale"; from: 1.0; to: 1.3; duration: 800; easing.type: Easing.OutCubic }
                    NumberAnimation { target: indeterminateProgress; property: "scale"; from: 1.3; to: 1.0; duration: 800; easing.type: Easing.OutCubic }
                    NumberAnimation { target: indeterminateProgress; property: "opacity"; from: 1.; to: 0.5; duration: 800; easing.type: Easing.OutCubic }
                }
            }
        }
    }

    Component {
        id: connectedView
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10

            SequentialAnimation {
                    running: true
                    loops: Animation.Infinite
                    NumberAnimation { target: container; property: "scale"; from: 1.0; to: 1.01; duration: 800; easing.type: Easing.OutCubic }
                    NumberAnimation { target: container; property: "scale"; from: 1.01; to: 1.0; duration: 800; easing.type: Easing.OutCubic }
                }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Text {
                        text: "<araç adı>"
                        font.pixelSize: 18
                        font.bold: true
                        color: "black"
                    }

                    Text {
                        text: "192.168.1.128"
                        font.pixelSize: 14
                        color: "gray"
                    }
                }

                Item{
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: 130
                    height: 60

                    Image {
                        source: "../../images/hyperloop_icon_tp.png"
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
        }
    }
}
