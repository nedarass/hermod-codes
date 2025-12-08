#include "../include/tcp_client.h"
#include <QDataStream>
#include <QTimer>
#include <QDebug>
#include <QtEndian> 

TCPClient::TCPClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &TCPClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TCPClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TCPClient::onReadyRead);

    // SADECE onError KULLAN - onSocketError YOK
    #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        connect(socket, &QAbstractSocket::errorOccurred, this, &TCPClient::onError);
    #else
        connect(socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                this, &TCPClient::onError);
    #endif
}

TCPClient::~TCPClient() {
    if (socket->isOpen()) socket->close();
}

void TCPClient::connectToServer(QString host, int port) {
    if (socket->state() == QAbstractSocket::ConnectedState) return;
    socket->abort();
    socket->connectToHost(host, port);
}

void TCPClient::disconnectFromServer() {
    socket->disconnectFromHost();
}

bool TCPClient::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

// --- FİNAL BINARY OKUMA MANTIĞI ---
void TCPClient::onReadyRead() {
    buffer.append(socket->readAll());

    // Paket: [AA][55][ID][LEN][...DATA...][CRC]
    while (buffer.size() >= 6) {
        
        // Header (AA 55) kontrolü
        if ((quint8)buffer[0] != PKT_START || (quint8)buffer[1] != PKT_END) {
             // Header bozuksa 1 byte kaydırıp tekrar dene (Basit senkronizasyon)
             buffer.remove(0, 1);
             continue;
        }

        quint8 len = static_cast<quint8>(buffer[3]); 
        int totalFrameSize = 4 + len + 1; // Header(2)+ID(1)+Len(1)+Data+CRC(1)

        if (buffer.size() < totalFrameSize) return; // Verinin devamı bekleniyor

        QByteArray frame = buffer.left(totalFrameSize);
        buffer.remove(0, totalFrameSize);
        
        parseFrame(frame);
    }
}

void TCPClient::parseFrame(const QByteArray &frame) {
    quint8 id = static_cast<quint8>(frame[2]);
    quint8 len = static_cast<quint8>(frame[3]);
    QByteArray data = frame.mid(4, len);
    quint8 receivedCrc = static_cast<quint8>(frame[4 + len]);

    if (calculateCRC(frame.left(4 + len)) != receivedCrc) {
        return; // CRC Hatası
    }

    // communication_ids.h içindeki ID'leri kullanıyoruz
    switch (id) {
        case ID_VELOCITY: {
            quint16 raw = qFromLittleEndian<quint16>(data);
            emit speedUpdated(raw / 100.0f);
            break;
        }
        case ID_POSITION: {
            if (data.size() >= sizeof(float)) {
                float pos;
                memcpy(&pos, data.constData(), sizeof(float));
                emit positionUpdated(pos);
            }
            break;
        }
        case ID_VOLTAGE: {
            if (data.size() >= 2) {
                quint16 raw = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data.constData()));
                emit voltageUpdated(raw / 100.0f);
            }
            break;
        }
        case ID_TEMPERATURE_NTC1: {
            if (data.size() >= 2) {
                qint16 raw = qFromLittleEndian<qint16>(reinterpret_cast<const uchar*>(data.constData()));
                emit temperatureUpdated(raw / 100.0f);
            }
            break;
        }
        case ID_BRAKE_STATUS: {
            if (data.size() >= 1) {
                bool engaged = static_cast<bool>(data[0]);
                emit brakeStatusChanged(engaged);
            }
            break;
        }
        case ID_ERROR_FLAG: {
            if (data.size() >= 4) {
                quint32 flags = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(data.constData()));
                emit errorFlagsUpdated(flags);
            }
            break;
        }
        case ID_ACCELERATION: { // 0x02
            if (data.size() >= 2) {
                quint16 raw = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data.constData()));
                emit accelerationUpdated(raw / 100.0f);
            }
            break;
       }
        case ID_CURRENT: { // 0x05
           if (data.size() >= 2) {
               quint16 raw = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data.constData()));
               emit currentUpdated(raw / 100.0f);
           }
           break;
        }
        case ID_POWER: { // 0x06
            if (data.size() >= 2) {
                quint16 pwr = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data.constData()));
                emit powerUpdated(pwr / 100.0f); // pwr değişkenini tanımladık
            }
            break;
        }
        default:
            qDebug() << "Unknown ID:" << QString::number(id, 16);
            break;
    }
}

void TCPClient::sendCommandPacket(quint8 id, quint8 type, const QByteArray &payload) {
    if (!isConnected()) return;

    QByteArray frame;
    frame.append(static_cast<char>(PKT_START)); 
    frame.append(static_cast<char>(PKT_END));   
    frame.append(static_cast<char>(id));
    frame.append(static_cast<char>(payload.size()));
    frame.append(payload);
    frame.append(calculateCRC(frame));

    socket->write(frame);
    socket->flush();
}

void TCPClient::sendBrakeCommand(quint8 force) {
    QByteArray payload;
    payload.append(static_cast<char>(force));
    sendCommandPacket(CMD_BRAKE_ACTUATE, TYPE_U8, payload);
}

void TCPClient::sendTargetSpeedCommand(float speed) {
    QByteArray payload;
    payload.resize(sizeof(float));
    memcpy(payload.data(), &speed, sizeof(float));
    sendCommandPacket(CMD_SET_TARGET_SPEED, TYPE_F32, payload);
}

void TCPClient::sendPowerCutCommand() {
    sendCommandPacket(CMD_POWER_CUT_OFF, 0x00, QByteArray());
}

void TCPClient::sendPingRequest() {
    sendCommandPacket(CMD_PING_REQUEST, 0x00, QByteArray());
}

quint8 TCPClient::calculateCRC(const QByteArray &data) {
    quint8 crc = 0;
    for (char byte : data) crc ^= static_cast<quint8>(byte);
    return crc;
}

void TCPClient::onConnected() {
    emit connectionChanged(true);
    buffer.clear();
}

void TCPClient::onDisconnected() {
    emit connectionChanged(false);
}

void TCPClient::onError(QAbstractSocket::SocketError) {
    emit errorOccurred(socket->errorString());
}
