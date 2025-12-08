#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "include/udp_listener.h"
#include "include/tcp_client.h"
//
#include "include/serial_communicator.h"

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

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // serial communication

    SerialCommunicator serial;
    serial.openPort("/dev/ttyUSB0", 115200); // test için port adı değiştir


    engine.rootContext()->setContextProperty("SerialCommunicator", &serial);


    engine.loadFromModule("desktop", "Main");

    

    return app.exec();
}
