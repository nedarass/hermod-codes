#include "serial_communicator.h"
#include <QDebug>
#include <QtEndian>

SerialCommunicator::SerialCommunicator(QObject *parent) : QObject(parent) {
    serialPort = new QSerialPort(this);
    connect(serialPort, &QSerialPort::readyRead, this, &SerialCommunicator::onReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &SerialCommunicator::handleError);
}

SerialCommunicator::~SerialCommunicator() {
    if(serialPort->isOpen()) serialPort->close();
}

void SerialCommunicator::openPort(QString portName, int baudRate) {
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    if(!serialPort->open(QIODevice::ReadWrite))
        qDebug() << "Port Hatasi:" << serialPort->errorString();
}

void SerialCommunicator::closePort() {
    if(serialPort->isOpen()) serialPort->close();
}

void SerialCommunicator::sendData(const QByteArray &data) {
    if(serialPort->isOpen()) serialPort->write(data);
}

void SerialCommunicator::onReadyRead() {
    buffer.append(serialPort->readAll());

    while (buffer.size() >= 6) {
        // ID dosyasındaki define'ları kullanıyoruz
        if ((quint8)buffer[0] != PKT_START || (quint8)buffer[1] != PKT_END) {
             buffer.remove(0, 1);
             continue;
        }

        quint8 len = static_cast<quint8>(buffer[3]); 
        int totalFrameSize = 4 + len + 1; 

        if (buffer.size() < totalFrameSize) return;

        QByteArray frame = buffer.left(totalFrameSize);
        buffer.remove(0, totalFrameSize);
        parseFrame(frame);
    }
}

void SerialCommunicator::parseFrame(const QByteArray &frame) {
    quint8 id = static_cast<quint8>(frame[2]);
    quint8 len = static_cast<quint8>(frame[3]);
    QByteArray data = frame.mid(4, len);
    quint8 crc = static_cast<quint8>(frame[4 + len]);

    if (calculateCRC(frame.left(4 + len)) != crc) return;

    // communication_ids.h ID'leri
    switch (id) {
    case ID_VELOCITY: {
        quint16 raw = qFromLittleEndian<quint16>(data);
        emit speedUpdated(raw / 100.0f);
        break;
    }
    case ID_ACCELERATION: {
        qint16 raw = qFromLittleEndian<qint16>(data);
        emit accelUpdated(raw / 100.0f);
        break;
    }
    case ID_POSITION: {
        float pos;
        memcpy(&pos, data.constData(), sizeof(float));
        emit positionUpdated(pos);
        break;
    }
    case ID_VOLTAGE: {
        quint16 raw = qFromLittleEndian<quint16>(data);
        emit voltageUpdated(raw / 100.0f);
        break;
    }
    case ID_TEMPERATURE_NTC1: {
        qint16 raw = qFromLittleEndian<qint16>(data);
        emit temperatureUpdated(raw / 100.0f);
        break;
    }
    case ID_BRAKE_STATUS: {
        bool brake = static_cast<bool>(data[0]);
        emit brakeStatusChanged(brake);
        break;
    }
    case ID_ERROR_FLAG: {
        quint32 error_flags = qFromLittleEndian<quint32>(data);
        emit errorFlagsUpdated(error_flags);
        break;
    }
    // ... Diğerleri ...
    }
}

void SerialCommunicator::sendCommandPacket(quint8 id, quint8 type, const QByteArray &payload)
{
    QByteArray frame;
    frame.append(static_cast<char>(PKT_START)); 
    frame.append(static_cast<char>(PKT_END));
    frame.append(static_cast<char>(id));
    frame.append(static_cast<char>(payload.size()));
    frame.append(payload);
    frame.append(calculateCRC(frame));

    if(serialPort->isOpen()) serialPort->write(frame);
}

void SerialCommunicator::sendBrakeCommand(quint8 force) {
    QByteArray payload;
    payload.append(force);
    sendCommandPacket(CMD_BRAKE_ACTUATE, TYPE_U8, payload);
}

void SerialCommunicator::sendTargetSpeedCommand(float speed_mps) {
    QByteArray payload;
    payload.resize(sizeof(float));
    memcpy(payload.data(), &speed_mps, sizeof(float));
    sendCommandPacket(CMD_SET_TARGET_SPEED, TYPE_F32, payload);
}

void SerialCommunicator::sendPowerCutCommand() {
    sendCommandPacket(CMD_POWER_CUT_OFF, 0x00, QByteArray());
}

void SerialCommunicator::sendPingRequest() {
    sendCommandPacket(CMD_PING_REQUEST, 0x00, QByteArray());
}

quint8 SerialCommunicator::calculateCRC(const QByteArray &data) {
    quint8 crc = 0;
    for (char byte : data) crc ^= static_cast<quint8>(byte);
    return crc;
}

void SerialCommunicator::handleError(QSerialPort::SerialPortError error) {
    // Hata yönetimi
}
