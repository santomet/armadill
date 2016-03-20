#include "servermanager.h"
#include <qjsonarray>
#include <qjsonobject>
#include <qjsonvalue>
#include <qdatetime>


QJsonObject ServerManager::exportOnlineUsersJson() {
	const QList<client*> & clientList = clientDatabase.getClientsList();
	
	QJsonObject clientsJson;
	QJsonArray clientsArray;
	for each (client* client in clientList)
	{
		if (client->loggedIn && 
			(client->loginValidUntil.toUTC() > QDateTime::currentDateTimeUtc())) {
			
			QJsonObject singleClient
			{
				{ "nick", QJsonValue(client->clientNick) },
				{ "address", QJsonValue(client->address) },
				{ "port", QJsonValue(client->listeningPort)}
			};
			clientsArray << singleClient;
		}
	}
	clientsJson.insert("users", clientsArray);
	return clientsJson;
}