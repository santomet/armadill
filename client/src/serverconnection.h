#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include <QtNetwork/QtNetwork>
#include <QtNetwork/QTcpSocket>
#include <QThread>
#include "krypto.h"
#include "common.h"

class ServerConnection : public QObject
{
    Q_OBJECT
public:
    explicit ServerConnection(QString host, int port, QObject *parent = 0);

    QList<peer> mPeers; //list of peers from server

    //----------certs
    QString mCert; //actual certificate in base64
    QString mCertKey; //actual certificate private key (base64)
    QString mSignRequest; //actual signing request for CA (base64)

    QString mServerCert; //should be loaded from file

    //---------Connection
    int mPort; //my port for listening
    QString mServerAddress; //server address
    int mServerPort;


signals:
    void connectSuccess();

public slots:
    void sendDataToServer(QByteArray array);

protected slots:
    void init();
    void connectionError(QAbstractSocket::SocketError error);
    void connectionSuccess() {this->mConnected = true; qDebug() << "successfully connected, please log in(l) or register(r)";}
    void dataFromServerReady();
    void serverDisconnected() {this->mConnected = false; qDebug() << "disconnected :(";}

private:
    bool mConnected{false};
    QTcpSocket *mSoc;
    QThread *mThread;
};

#endif // SERVERCONNECTION_H
