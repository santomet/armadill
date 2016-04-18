#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QObject>
#include "servermanager.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>

class ClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit ClientConnection(qintptr socDescriptor, ServerManager *ser, bool newThread, QObject *parent = 0);
    ~ClientConnection();

signals:
    void done(ClientConnection *c);
public slots:

protected slots:
    void init();
    void doneSlot() {emit done(this);}

    void readDataFromClient();
    void sendDataToClient(QByteArray a) {mSoc->write(a);}

private:
    /*!
    * \brief parseLoginMessage		parses user's login message
    *
    * \return						true if all went right
    */
    bool parseLoginMessage(QByteArray& message);

    QThread *mThread{nullptr};
    ServerManager *mServerManager;
    QTcpSocket *mSoc;
    qintptr mSocDescriptor;
    QString mPeerAddress;
    QString mNickName{""};

};

#endif // CLIENTCONNECTION_H
