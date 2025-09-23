import QtQuick 2.15

QtObject {
    property string currentPage: "MonitoringPage.qml"
    signal pageChanged(string newPage)

    function setPage(newPage) {
        if (currentPage !== newPage) {
            currentPage = newPage;
            pageChanged(newPage);
        }
    }
}
