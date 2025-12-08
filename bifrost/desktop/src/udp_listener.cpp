#include "udp_listener.h"
#include <QHostAddress>
#include <QDebug>

UDPListener::UDPListener(uint16_t port, QObject *parent)
    : QObject(parent), port(port), udpSocket(new QUdpSocket(this)) {}

UDPListener::~UDPListener() {
    stopListening();
}

void UDPListener::startListening() {
    // ShareAddress: Aynı portu birden fazla uygulama dinleyebilsin diye
    if (udpSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress)) {
        connect(udpSocket, &QUdpSocket::readyRead, this, &UDPListener::processPendingDatagrams);
        qDebug() << "UDP Listener basladi. Port:" << port;
    } else {
        qWarning() << "UDP Port acilamadi:" << port;
    }
}

void UDPListener::stopListening() {
    if (udpSocket->isOpen()) {
        udpSocket->close();
        qDebug() << "UDP Listener stopped.";
    }
}

void UDPListener::setTCPClient(TCPClient *client) {
    this->tcpClient = client;
}

void UDPListener::processPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString message = QString::fromUtf8(datagram);

        // Mesaj Formatı: "hermod-polaris-broadcast\nNAME\nIP:PORT"
        QStringList mList = message.split("\n");
        if (mList.size() < 3) return; // Hatalı mesaj

        // 1. Güvenlik Kontrolü: Bizim araç mı?
        if (mList[0] != "hermod-polaris-broadcast") return;

        // 2. IP ve Portu Al
        QStringList ip_port = mList[2].split(":");
        if (ip_port.size() < 2) return;

        QString targetIP = ip_port[0];
        int targetPort = ip_port[1].toInt();

        // 3. Otomatik Bağlanma (Sadece bağlı değilsek!)
        if (tcpClient && !tcpClient->isConnected()) {
            qDebug() << "Polaris Bulundu:" << targetIP << ":" << targetPort;
            
            // GUI Thread'inde güvenli çağrı
            QMetaObject::invokeMethod(tcpClient, "connectToServer", 
                                      Qt::AutoConnection, 
                                      Q_ARG(QString, targetIP), 
                                      Q_ARG(int, targetPort));
        }
        
        emit receivedMessage(message);
    }
}
