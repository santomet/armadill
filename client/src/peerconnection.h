#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>

class PeerConnection : public QObject
{
    Q_OBJECT
public:
    explicit PeerConnection(QObject *parent = 0);

signals:

public slots:
};

#endif // PEERCONNECTION_H
