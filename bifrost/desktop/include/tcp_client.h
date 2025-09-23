#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include "udp_listener.h"

class TCPClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)

public:
    explicit TCPClient(QObject *parent = nullptr);

    Q_INVOKABLE void connectToServer(QString host, int port);
    Q_INVOKABLE void sendMessage(const QString &message);
    bool isConnected() const;

signals:
    void connectionChanged();
    void messageReceived(const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();

private:
    QTcpSocket *socket;
};

#endif
