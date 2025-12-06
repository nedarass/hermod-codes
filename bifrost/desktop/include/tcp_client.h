#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QByteArray>
#include "communication_ids.h" // <--- EKLENDİ

class TCPClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)

public:
    explicit TCPClient(QObject *parent = nullptr);
    ~TCPClient();

    Q_INVOKABLE void connectToServer(QString host, int port);
    Q_INVOKABLE void disconnectFromServer();
    bool isConnected() const;

    // Komutlar
    Q_INVOKABLE void sendBrakeCommand(quint8 force);
    Q_INVOKABLE void sendTargetSpeedCommand(float speed);
    Q_INVOKABLE void sendPowerCutCommand();
    Q_INVOKABLE void sendPingRequest();

signals:
    void connectionChanged(bool connected);
    void errorOccurred(QString errorMsg);
    
    // Telemetri Sinyalleri
    void speedUpdated(float speed);
    void positionUpdated(float position);
    void voltageUpdated(float voltage);
    void temperatureUpdated(float temp);
    void brakeStatusChanged(bool engaged);
    void errorFlagsUpdated(int flags);
    void messageReceived(const QString &msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead(); // Binary okuma
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket;
    QByteArray buffer;

    void parseFrame(const QByteArray &frame);
    quint8 calculateCRC(const QByteArray &data);
    void sendCommandPacket(quint8 id, quint8 type, const QByteArray &payload);
};

#endif
