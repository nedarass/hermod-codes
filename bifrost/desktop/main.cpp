#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include "include/udp_listener.h"
#include "include/tcp_client.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    TCPClient client;
    
    UDPListener udp_listener(12321);
    udp_listener.setTCPClient(&client);

    QQmlApplicationEngine engine;
    // context property leri modu eklemeeden once cagir

    engine.rootContext()->setContextProperty("UDPListener", &udp_listener);
    engine.rootContext()->setContextProperty("TCPClient", &client);



    QString mainQmlPath = "/Users/ubeydhamzatoptas/Downloads/hermod-codes-stm32/bifrost/desktop/Main.qml";
    qDebug() << "Loading QML from ABSOLUTE path:" << mainQmlPath;
    qDebug() << "File exists:" << QFile::exists(mainQmlPath);
    engine.load(QUrl::fromLocalFile(mainQmlPath));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            qDebug() << "QML loading failed!";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    return app.exec();
}
