#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>
#include <QJsonObject>
#include "clientdb.h"

class ServerManager : public QObject
{
    Q_OBJECT
private:
    ClientDb clientDatabase;
public:
    ServerManager(QString path) : clientDatabase(path) {}

    bool newRegistration(QString nickName, QString passwordHash);

    bool login(QString nickname, QString password, QString address, int port, QString cert);



	/*!
	* \brief exportOnlineUsersJson        returns a JSON of online users
	*                              
	* \return
	*/
	QJsonObject exportOnlineUsersJson();
};

#endif
