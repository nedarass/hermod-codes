import QtQuick 2.15
import "../views/."
import "."

//bu view panelleri icerier. tam aklimdaki net degil ya gorev baslangici/ gorev sureci/ bitirme seklinde ya da monitor/kontrol eklinde bolecez

Item {
    property var mediator

    Loader{
        id: dashboardContent
        anchors.fill: parent
        active: true                    // sayfayi yukle
        asynchronous: true              // asenkron yurut
        source: mediator.currentPage     // navigation pillden secili sayfayi yukle.
    }

    Connections {
        target: mediator
        function onCurrentPageChanged(newPage) {
            dashboardContent.source = mediator.currentPage;
        }
    }

    signal currentPageChanged(string newPage)

}
