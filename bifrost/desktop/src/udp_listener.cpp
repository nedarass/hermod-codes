#include "udp_listener.h"
#include <QHostAddress>
#include <QDebug>

UDPListener::UDPListener(uint16_t port, QObject *parent)
    : QObject(parent), port(port), udpSocket(new QUdpSocket(this)) {}

UDPListener::~UDPListener() {
    stopListening();
}

void UDPListener::startListening() {
    if (udpSocket->bind(QHostAddress::Any, port)) {
        connect(udpSocket, &QUdpSocket::readyRead, this, &UDPListener::processPendingDatagrams);
        qDebug() << "UDP Listener started on port:" << port;
    } else {
        qWarning() << "Failed to bind UDP port:" << port;
    }
}

void UDPListener::stopListening() {
    if (udpSocket->isOpen()) {
        udpSocket->close();
        qDebug() << "UDP Listener stopped.";
    }
}

void UDPListener::processPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString message = QString::fromUtf8(datagram);

        QStringList mList = message.split("\n");
        
        QString identifier = mList[0];
        QString vehicle_name = mList[1];
        QStringList ip_port = mList[2].split(":");

        qDebug() << "Received UDP Message from" << sender.toString() << ":" << ip_port[0] << "-" << ip_port[1];

        QMetaObject::invokeMethod(
            tcpClient,          // Pointer to QObject instance
            "connectToServer",    // Name of the method as a string
            Qt::AutoConnection, // Connection type (optional)
            Q_ARG(QString, ip_port[0]),  // Argument 1
            Q_ARG(int, ip_port[1].toInt())   // Argument 2 (if any)
        );

        emit receivedMessage(message);
    }
}

void UDPListener::setTCPClient(TCPClient *client){
    this->tcpClient = client;
}
