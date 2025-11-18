#include "serial_communicator.h"
#include <QDebug>
#include <QtEndian> // Küçük/büyük endian dönüşümü için (byte sırası)


SerialCommunicator::SerialCommunicator(QObject *parent) : QObject(parent) {
    serialPort = new QSerialPort(this);
    // Veri geldiğinde veya hata oluştuğunda sinyalleri bağla
    connect(serialPort, &QSerialPort::readyRead, this, &SerialCommunicator::onReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &SerialCommunicator::handleError);
}

SerialCommunicator::~SerialCommunicator() {
    if(serialPort->isOpen()) serialPort->close();
}

// Portu açar
void SerialCommunicator::openPort(QString portName, int baudRate) {
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    if(!serialPort->open(QIODevice::ReadWrite))
        qDebug() << "Port açılamadı:" << serialPort->errorString();
    else
        qDebug() << "Port açıldı:" << portName;

}

// Portu kapatır
void SerialCommunicator::closePort() {
    if(serialPort->isOpen()) serialPort->close();
}

// Araca veri göndermek için (örneğin manuel kontrol komutları)
void SerialCommunicator::sendData(const QByteArray &data) {
    if(serialPort->isOpen()) serialPort->write(data);
}

// gelen her veri burada parse edilir
// Her veri geldiğinde çağrılır
void SerialCommunicator::onReadyRead() {

     // Gelen veriyi buffer’a ekle
    buffer.append(serialPort->readAll());

    // Frame yapısı: [0xAA][0x55][ID][LEN][DATA][CRC]
    while (buffer.size() >= 6) {
        // HEADER arama
        int startIndex = buffer.indexOf(QByteArray::fromHex("AA55"));
        if (startIndex == -1) {
            buffer.clear();
            return;
        }
        if (startIndex > 0)
            buffer.remove(0, startIndex); // header öncesi gereksiz veriyi at

        if (buffer.size() < 5)
            return; // yeterli veri yok

        quint8 id = static_cast<quint8>(buffer[2]); // Veri tipi ID
        quint8 len = static_cast<quint8>(buffer[3]); // Veri uzunluğu
        int totalFrameSize = 2 + 1 + 1 + len + 1; // AA55 + ID + LEN + DATA + CRC

        if (buffer.size() < totalFrameSize)
            return; // tüm frame henüz gelmemiş

        QByteArray frame = buffer.left(totalFrameSize);
        buffer.remove(0, totalFrameSize);
        parseFrame(frame);
    }
}

// Frame içerisindeki ID’ye göre doğru sinyali gönder
void SerialCommunicator::parseFrame(const QByteArray &frame) {
    quint8 id = static_cast<quint8>(frame[2]);
    quint8 len = static_cast<quint8>(frame[3]);
    QByteArray data = frame.mid(4, len);
    quint8 crc = static_cast<quint8>(frame[4 + len]);

    // CRC kontrolü
    if (calculateCRC(frame.left(4 + len)) != crc) {
        qWarning() << "CRC hatası!";
        return;
    }

    // Gelen ID’ye göre doğru sinyal yayılır
    switch (id) {
    case 0x01: { // Hız
        quint16 raw = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data.constData()));
        emit speedUpdated(raw / 100.0f);
        break;
    }
    case 0x02: { // İvme
        qint16 raw = qFromLittleEndian<qint16>(reinterpret_cast<const uchar*>(data.constData()));
        emit accelUpdated(raw / 100.0f);
        break;
    }
    case 0x03: { // Konum
        float pos;
        memcpy(&pos, data.constData(), sizeof(float));
        emit positionUpdated(pos);
        break;
    }
    case 0x04: { // Voltaj
        quint16 raw = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data.constData()));
        emit voltageUpdated(raw / 100.0f);
        break;
    }
    case 0x05: { // Akım
        qint16 raw = qFromLittleEndian<qint16>(reinterpret_cast<const uchar*>(data.constData()));
        emit currentUpdated(raw / 100.0f);
        break;
    }
    case 0x06: { // Güç
        float pwr;
        memcpy(&pwr, data.constData(), sizeof(float));
        emit powerUpdated(pwr);
        break;
    }
    case 0x07: { // Sıcaklık
        qint16 raw = qFromLittleEndian<qint16>(reinterpret_cast<const uchar*>(data.constData()));
        emit temperatureUpdated(raw / 100.0f);
        break;
    }
    case 0x08: { // Fren
        bool brake = static_cast<bool>(data[0]);
        emit brakeStatusChanged(brake);
        break;
    }
    case 0x09: { // TCP bağlantı durumu
        bool connected = static_cast<bool>(data[0]);
        emit connectionStatusChanged(connected);
        break;
    }
    case 0xF1: { // Ping Süresi (ID_PING_RESPONSE - uint16/int16)
        qint16 ping_ms = qFromLittleEndian<qint16>(reinterpret_cast<const uchar*>(data.constData()));
        // emit pingTimeUpdated(ping_ms); // Bu sinyali QML'e göndermek için yayınlayın
        qDebug() << "Health Check: Ping Alindi:" << ping_ms << "ms";
        break;
    }
    case 0xF2: { // CPU Sıcaklığı (ID_CPU_TEMP - float)
        float cpu_temp;
        std::memcpy(&cpu_temp, data.constData(), sizeof(float));
        // emit cpuTemperatureUpdated(cpu_temp); // Bu sinyali QML'e göndermek için yayınlayın
        qDebug() << "Health Check: CPU Sicaklik:" << cpu_temp << "C";
        break;
    }
    case 0xF3: { // Hata Bayrakları (ID_ERROR_FLAG - uint32/binary)
        quint32 error_flags = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(data.constData()));
        // emit errorFlagsUpdated(error_flags); // Bu sinyali QML'e göndermek için yayınlayın
        qDebug() << "Health Check: Hata Bayraklari:" << Qt::hex << error_flags;

        // Örnek Hata Kontrolü: Eğer MPU hatası varsa (ERR_FLAG_MPU_FAIL = 1<<0)
        if (error_flags & 0x01) {
            qWarning() << "KRITIK HATA: MPU Sensör Bağlantısı Başarısız!";
        }
        break;
    }
    case 0xF4: { // Güç Hattı Durumu (ID_POWER_LINE_STATUS - uint8)
        quint8 status = data[0];
        // emit powerLineStatusUpdated(status); // Bu sinyali QML'e göndermek için yayınlayın
        qDebug() << "Health Check: Power Line Status:" << status;
        break;
    }
    case 0xF5: { // RTOS Task Durumları (ID_RTOS_STATUS - uint8)
        quint8 rtos_status = data[0];
        // emit rtosStatusUpdated(rtos_status); // Bu sinyali QML'e göndermek için yayınlayın
        qDebug() << "Health Check: RTOS Status:" << rtos_status;
        break;
    }
    default:
        qWarning() << "Bilinmeyen ID:" << id;
    }
}

/*// hermod-codes/bifrost/desktop/src/serial_communicator.cpp dosyasında...

// (Dosyanın sonuna veya özel bir 'Command Sending' bloğuna ekleyin)

// Protokolü oluşturan ana helper fonksiyonu (Private)
void SerialCommunicator::sendCommandPacket(quint8 id, quint8 type, const QByteArray &payload)
{
    QByteArray frame;
    quint16 payload_len = payload.size();

    // 1. START Byte
    frame.append(PKT_START);                                    // 1 bayt: 0xAA

    // 2. ID Byte
    frame.append(id);                                           // 1 bayt: 0xA1, 0xA2 vb.

    // 3. TYPE Byte
    frame.append(type);                                         // 1 bayt: 0x01 (U8), 0x07 (F32)

    // 4. LENGTH (2 bayt, Little Endian)
    // C++'ta 16-bit tamsayıyı 2 bayta ayırıp little endian (düşük bayt önce) olarak ekler.
    frame.append(static_cast<char>(payload_len & 0xFF));        // LEN_L (Düşük bayt)
    frame.append(static_cast<char>((payload_len >> 8) & 0xFF)); // LEN_H (Yüksek bayt)

    // 5. PAYLOAD
    frame.append(payload);

    // 6. END Byte
    frame.append(PKT_END);                                      // 1 bayt: 0x55

    // Paketi seri porttan gönder
    if(serialPort->isOpen()) {
        serialPort->write(frame);
        // Debug için
        qDebug() << "Komut Gonderildi. ID:" << Qt::hex << id << "Tip:" << Qt::hex << type << "Boyut:" << frame.size() << "byte.";
    } else {
        qWarning() << "Port Kapalı! Komut gonderilemedi.";
    }
}


// Komut Uygulamaları (Public Slots)
// -----------------------------------------------------------------------------

void SerialCommunicator::sendBrakeCommand(quint8 force)
{
    // Komut ID: 0xA1 (CMD_BRAKE_ACTUATE)
    // Payload: quint8 (1 bayt)
    QByteArray payload;
    payload.append(force);

    sendCommandPacket(0xA1, TYPE_U8, payload);
}

void SerialCommunicator::sendTargetSpeedCommand(float speed_mps)
{
    // Komut ID: 0xA2 (CMD_SET_TARGET_SPEED)
    // Payload: float (4 bayt)
    QByteArray payload;

    // C++ float değerini QByteArray'e kopyalar (Bayt sırası Pulse'a direkt geçer)
    payload.resize(sizeof(float));
    std::memcpy(payload.data(), &speed_mps, sizeof(float));

    sendCommandPacket(0xA2, TYPE_F32, payload);
}

void SerialCommunicator::sendPowerCutCommand()
{
    // Komut ID: 0xA3 (CMD_POWER_CUT_OFF)
    // Payload: Yok (0 bayt)
    sendCommandPacket(0xA3, 0x00, QByteArray()); // Tip 0x00 veya bir ACK tipi olabilir.
}

void SerialCommunicator::sendPingRequest()
{
    // Komut ID: 0xA4 (CMD_PING_REQUEST)
    // Payload: Yok (0 bayt)
    sendCommandPacket(0xA4, 0x00, QByteArray());
}*/

// XOR tabanlı basit CRC doğrulama
quint8 SerialCommunicator::calculateCRC(const QByteArray &data) {
    quint8 crc = 0;
    for (char byte : data)
        crc ^= static_cast<quint8>(byte);
    return crc;
}

void SerialCommunicator::handleError(QSerialPort::SerialPortError error) {
    qDebug() << "Serial Error:" << error;
}
