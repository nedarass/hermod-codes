#include "../include/tcp_client.h"
#include <QDebug>

TCPClient::TCPClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &TCPClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TCPClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TCPClient::onReadyRead);

    // Hata yakalama (Qt5/Qt6 uyumlu syntax)
    connect(socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this, &TCPClient::onError);
}

TCPClient::~TCPClient() {
    if (socket->isOpen()) socket->close();
}

// --- BAĞLANTI ---
void TCPClient::connectToServer(QString host, int port) {
    if (socket->state() == QAbstractSocket::ConnectedState) return;

    qDebug() << "Baglaniyor:" << host << ":" << port;
    socket->abort(); // Varsa eski işlemi iptal et
    socket->connectToHost(host, port);
}

void TCPClient::disconnectFromServer() {
    socket->disconnectFromHost();
}

bool TCPClient::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

// --- VERİ GÖNDERME ---
void TCPClient::sendMessage(const QString &message) {
    if (!isConnected()) return;
    socket->write(message.toUtf8());
    socket->flush();
}

// --- KOMUT FONKSİYONLARI ---
void TCPClient::sendBrakeCommand(int force) {
    // Polaris formatı: CMD:161:50
    sendMessage(QString("CMD:161:%1").arg(force));
}

void TCPClient::sendTargetSpeedCommand(float speed) {
    sendMessage(QString("CMD:162:%1").arg(speed));
}

void TCPClient::sendPowerCutCommand() {
    sendMessage("CMD:163:0");
}

// --- SLOTLAR ---
void TCPClient::onConnected() {
    qDebug() << "Sunucuya baglandi!";
    emit connectionChanged(true);
}

void TCPClient::onDisconnected() {
    qDebug() << "Baglanti kesildi!";
    emit connectionChanged(false);
}

void TCPClient::onError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit errorOccurred(socket->errorString());
}

// --- VERİ ALMA VE PARSE ETME ---
void TCPClient::onReadyRead() {
    // Polaris satır sonu (\n) ile veri gönderir.
    // canReadLine() tam bir satır gelip gelmediğini kontrol eder.
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed(); // \n temizle
        if (!line.isEmpty()) {
            processIncomingData(line);
        }
    }
}

void TCPClient::processIncomingData(const QString &data) {
    // 1. Ping/Pong Kontrolü
    if (data == "PING") { sendMessage("PONG"); return; }
    if (data == "PONG") return;

    // 2. Veriyi Parçala (Format: TYPE:ID:VALUE)
    QStringList parts = data.split(":");
    if (parts.size() < 3) return; // Hatalı paket

    // parts[0] = TYPE (FLOAT, INT, UINT) - Şu an kullanmıyoruz, ID yeterli
    bool ok;
    int id = parts[1].toInt(&ok);
    if (!ok) return;
    
    QString valueStr = parts[2];

    // 3. ID'ye Göre Sinyal Yay
    // Bu ID'ler communication_ids.h ile aynı olmalı!
    switch (id) {
        case 1: // Hız (ID_VELOCITY)
            emit speedUpdated(valueStr.toFloat());
            break;
        case 3: // Konum (ID_POSITION)
            emit positionUpdated(valueStr.toFloat());
            break;
        case 4: // Voltaj (ID_VOLTAGE)
            emit voltageUpdated(valueStr.toFloat());
            break;
        case 7: // Sıcaklık (ID_TEMPERATURE_NTC1)
            emit temperatureUpdated(valueStr.toFloat());
            break;
        case 8: // Fren Durumu (ID_BRAKE_STATUS)
            emit brakeStatusChanged(valueStr.toInt() > 0);
            break;
        case 243: // Hata Bayrakları (ID_ERROR_FLAG = 0xF3)
            emit errorFlagsUpdated(valueStr.toInt());
            break;
        default:
            // qDebug() << "Bilinmeyen ID:" << id;
            break;
    }
    
    // Ham veriyi de log için gönder
    emit messageReceived(data);
}
