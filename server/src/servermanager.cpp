#include "servermanager.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>


QJsonObject ServerManager::exportOnlineUsersJson() {
	const QList<client*> & clientList = clientDatabase.getClientsList();
	
	QJsonObject clientsJson;
	QJsonArray clientsArray;
    foreach (client* c, clientList)
	{
        if (c->loggedIn &&
            (c->loginValidUntil.toUTC() > QDateTime::currentDateTimeUtc())) {
			
			QJsonObject singleClient
			{
                { "nick", QJsonValue(c->clientNick) },
                { "address", QJsonValue(c->address) },
                { "port", QJsonValue(c->listeningPort)}
			};
			clientsArray << singleClient;
		}
	}
	clientsJson.insert("users", clientsArray);
	return clientsJson;
}
