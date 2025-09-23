#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include "../include/tcp_client.h"

class TCPClient;

class UDPListener : public QObject {
    Q_OBJECT

public:
    explicit UDPListener(uint16_t port, QObject *parent = nullptr);
    ~UDPListener();

    Q_INVOKABLE void startListening();
    void stopListening();


    void setTCPClient(TCPClient *client);

signals:
    void receivedMessage(const QString &message);

private slots:
    void processPendingDatagrams();

private:
    uint16_t port;
    QUdpSocket *udpSocket;

    TCPClient* tcpClient;
};

#endif
