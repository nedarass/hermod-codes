import QtQuick 2.15

Item {
    Rectangle{
        anchors.fill:parent
        anchors.margins: 10
        anchors.topMargin: 0
        radius: 8
        color: "black"
        border.color:"white"
        border.width: 2
        
        Text{
            anchors.centerIn:parent 
            text: "Manuel Control"
            color: "white"
        }
    }
    
}
