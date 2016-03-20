#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <qjsonobject>
#include "clientdb.h"

class ServerManager
{
private:
	ClientDb & clientDatabase;
public:
	ServerManager(ClientDb & clientDb) : clientDatabase(clientDb) {};
	/*!
	* \brief exportOnlineUsersJson        returns a JSON of online users
	*                              
	* \return
	*/
	QJsonObject exportOnlineUsersJson();
};

#endif