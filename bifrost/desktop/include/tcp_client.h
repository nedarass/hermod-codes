#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>

class TCPClient : public QObject {
    Q_OBJECT
    // QML tarafında bağlantı durumunu görmek için property
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)

public:
    explicit TCPClient(QObject *parent = nullptr);
    ~TCPClient();

    // Polaris'e bağlanmak için (IP ve Port Config'den veya UDP'den gelir)
    Q_INVOKABLE void connectToServer(QString host, int port);
    
    // Polaris'e veri göndermek için (Örn: Komutlar)
    Q_INVOKABLE void sendMessage(const QString &message);
    
    // Bağlantıyı kes
    Q_INVOKABLE void disconnectFromServer();

    bool isConnected() const;

signals:
    void connectionChanged(bool connected);
    // Polaris'ten gelen veriyi (JSON/String) QML'e iletir
    void messageReceived(const QString &message); 
    void errorOccurred(QString errorMsg);
    // --- TELEMETRİ SİNYALLERİ (QML Arayüzü İçin) ---
    // Bu sinyaller parse edilen verileri taşır
    void speedUpdated(float speed);       // ID 1
    void positionUpdated(float position); // ID 3
    void voltageUpdated(float voltage);   // ID 4
    void temperatureUpdated(float temp);  // ID 7
    void brakeStatusChanged(bool engaged);// ID 8
    void errorFlagsUpdated(int flags);    // ID 243 (0xF3)

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead(); // Yeni veri geldiğinde çalışır
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket;
    // Gelen metni ("FLOAT:1:25.5") parçalayan fonksiyon
    void processIncomingData(const QString &data);
};

#endif // TCPCLIENT_H
