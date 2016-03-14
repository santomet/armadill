#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include "krypto.h"

class ServerConnection : public QObject
{
    Q_OBJECT
public:
    explicit ServerConnection(QObject *parent = 0);

    //----------peers
    /*!
         * \brief The peer struct
         */
    struct peer
    {
        QString name;
        QString address;
        int listeningPort;
        QString cert;
        bool trustedByServer = false;
        bool active = false;
    };


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

public slots:
};

#endif // SERVERCONNECTION_H
