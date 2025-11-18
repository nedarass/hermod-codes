#ifndef SERIAL_COMMUNICATOR_H
#define SERIAL_COMMUNICATOR_H


#include <QObject>
#include <QSerialPort>
#include <QByteArray>


// Araçtan gelen veriler binary formatta gelir ve burada çözülür.

class SerialCommunicator : public QObject
{
    Q_OBJECT

public:
    explicit SerialCommunicator(QObject *parent = nullptr);
    ~SerialCommunicator();

    // Seri portu açmak ve kapatmak için QML'den erişilebilir fonksiyonlar
    Q_INVOKABLE void openPort(QString portName, int baudRate = 115200);
    Q_INVOKABLE void closePort();
    Q_INVOKABLE void sendData(const QByteArray &data);

    /*public slots:
    // QML'den çağrılacak yeni komut gönderme fonksiyonları
    void sendBrakeCommand(quint8 force);             // CMD_BRAKE_ACTUATE (0xA1)
    void sendTargetSpeedCommand(float speed_mps);    // CMD_SET_TARGET_SPEED (0xA2)
    void sendPowerCutCommand();                      // CMD_POWER_CUT_OFF (0xA3)
    void sendPingRequest();                          // CMD_PING_REQUEST (0xA4)

    // Port işlemleri
    void openPort(QString portName, int baudRate);
    void closePort();


private:
    // Protokol tanımları (normalde communication_ids.h'den gelir, C++'a kopyalanabilir)
    static constexpr quint8 PKT_START = 0xAA;
    static constexpr quint8 PKT_END = 0x55;

    // Protokoldeki tipler
    static constexpr quint8 TYPE_U8 = 0x01;
    static constexpr quint8 TYPE_F32 = 0x07;
    // ... Diğer protokol ID'leri buraya (0xA1, 0xA2, vs.)

    // Ana veri gönderme helper fonksiyonu
    void sendCommandPacket(quint8 id, quint8 type, const QByteArray &payload);*/

signals:
    // Ham veri (işlenmemiş haliyle)
    void rawDataReceived(const QByteArray &data);

    // Parse sonrası yayınlanan telemetri sinyalleri
    // Bunlar QML tarafında dinlenip ekranda gösterilir
    void speedUpdated(float speed);
    void accelUpdated(float accel);
    void positionUpdated(float position);
    void voltageUpdated(float voltage);
    void currentUpdated(float current);
    void powerUpdated(float power);
    void temperatureUpdated(float temp);
    void brakeStatusChanged(bool engaged);
    void connectionStatusChanged(bool connected);


private slots:
    // Yeni veri geldiğinde tetiklenen Qt slotu
    void onReadyRead();
    // Yeni veri geldiğinde tetiklenen Qt slotu
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serialPort;
    QByteArray buffer;
    void parseFrame(const QByteArray &frame);
    quint8 calculateCRC(const QByteArray &data);
};

#endif
