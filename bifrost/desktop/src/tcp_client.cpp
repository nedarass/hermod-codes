#include "../include/tcp_client.h"
#include <QDebug>

TCPClient::TCPClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &TCPClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TCPClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TCPClient::onReadyRead);
}

void TCPClient::connectToServer(QString host, int port) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Zaten bağlı!";
        return;
    }

    qDebug() << "Bağlanıyor: " << host << ":" << port;
    socket->connectToHost(host, port);
}

void TCPClient::sendMessage(const QString &message) {
    if (!isConnected()) {
        qDebug() << "Bağlı değil! Msaj gönderilemiyor!";
        return;
    }
    socket->write(message.toUtf8());
    socket->flush();
}

bool TCPClient::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

void TCPClient::onConnected() {
    qDebug() << "Sunucuya bağlı!";
    emit connectionChanged();

    qDebug() << "sikinti yok";
}

void TCPClient::onDisconnected() {
    qDebug() << "Sunucu bağlantısı kesildi!";
    emit connectionChanged();
}

void TCPClient::onReadyRead() {
    QString message = QString::fromUtf8(socket->readAll());
    qDebug() << "Mesaj:" << message;
    emit messageReceived(message);

    if(message == "PING")
    {
        sendMessage("PONG");
    }

    if(message == "PONG")
    {
        sendMessage("PING");
    }
}
