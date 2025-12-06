#ifndef SERIAL_COMMUNICATOR_H
#define SERIAL_COMMUNICATOR_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include "communication_ids.h" // <--- EKLENDİ

class SerialCommunicator : public QObject
{
    Q_OBJECT

public:
    explicit SerialCommunicator(QObject *parent = nullptr);
    ~SerialCommunicator();

    Q_INVOKABLE void openPort(QString portName, int baudRate = 115200);
    Q_INVOKABLE void closePort();
    Q_INVOKABLE void sendData(const QByteArray &data);

public slots:
    void sendBrakeCommand(quint8 force);
    void sendTargetSpeedCommand(float speed_mps);
    void sendPowerCutCommand();
    void sendPingRequest();

private:
    // ESKİ DEFINE'LAR SİLİNDİ (PKT_START vs. artık include dosyasından geliyor)
    void sendCommandPacket(quint8 id, quint8 type, const QByteArray &payload);
    quint8 calculateCRC(const QByteArray &data);

    void parseFrame(const QByteArray &frame); // Private helper

signals:
    void speedUpdated(float speed);
    void accelUpdated(float accel);
    void positionUpdated(float position);
    void voltageUpdated(float voltage);
    void currentUpdated(float current);
    void powerUpdated(float power);
    void temperatureUpdated(float temp);
    void brakeStatusChanged(bool engaged);
    void connectionStatusChanged(bool connected);
    void errorFlagsUpdated(int flags);

private slots:
    void onReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serialPort;
    QByteArray buffer;
};

#endif
