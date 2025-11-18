#ifndef SERIAL_COMMUNICATOR_H
#define SERIAL_COMMUNICATOR_H

#include <QObject>
#include <QSerialPort> // Ağ soketi yerine seri port kütüphanesi
#include <QByteArray>  // İkili (Binary) veri yönetimi için

/**
 * @class SerialCommunicator
 * @brief STM32 ile USB Sanal Seri Port üzerinden iletişimi yönetir.
 * * Bu sınıf, TCPClient'ın işlevini devralarak güvenilir komut ve
 * kritik veri alışverişini seri port üzerinden gerçekleştirir.
 */
class SerialCommunicator : public QObject {
    Q_OBJECT
    // Seri portun açık olup olmadığını izler
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY connectionChanged)

public:
    explicit SerialCommunicator(QObject *parent = nullptr);
    ~SerialCommunicator();

    // Ağ bağlantısı yerine seri porta bağlanmayı sağlar
    Q_INVOKABLE void openPort(QString portName, int baudRate = 115200);
    Q_INVOKABLE void closePort();

    // İkili (Binary) veri göndermek için uyarlanmıştır
    // QString yerine QByteArray kullanmak daha verimlidir
    Q_INVOKABLE void sendData(const QByteArray &data);
    
    bool isOpen() const;

signals:
    void connectionChanged();
    
    // Gelen ham (Raw) ikili veriyi (Telemetry/Command) sinyalle bildirir
    void rawDataReceived(const QByteArray &data);

private slots:
    // Seri port açıldığında/kapandığında QSerialPort sinyallerini yakalar
    void onPortOpened();
    void onPortClosed();
    
    // Yeni veri geldiğinde (STM32'den) tetiklenir
    void onReadyRead();
    
    // Hata durumlarını ele almak için
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serialPort;
};

#endif // SERIAL_COMMUNICATOR_H

/* #ifndef TCPCLIENT_H
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

#endif*/
