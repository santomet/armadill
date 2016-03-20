#include "servermanager.h"
#include <qjsonarray>
#include <qjsonobject>
#include <qjsonvalue>

ServerManager::ServerManager(ClientDb clientDb)
	: clientDatabase(clientDb)
	{

	}

QJsonObject ServerManager::exportOnlineUsersJson() {
	QList<client*> *clientList = clientDatabase.getClientsList();
	
	QJsonObject clientsJson;
	QJsonArray clientsArray;
	for each (client* client in *clientList)
	{
		if (client->loggedIn) {
			
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