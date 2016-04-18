#include "servermanager.h"

bool ServerManager::newRegistration(QString nickName, QString password)
{
    return clientDatabase.addNewClient(nickName.toLatin1().constData(), password.toLatin1().constData());
}

bool ServerManager::login(QString nickname, QString password, QString address, int port, QString cert)
{
    bool ret = false;
    if(clientDatabase.verifyClient(nickname.toLatin1().constData(), password.toLatin1().constData()))
    {
        //TODO: SIGN client's certificate
        //CRITICAL PART!
        ret = clientDatabase.loginClient(nickname.toLatin1().constData(), address.toLatin1().constData(), port);
    }
    return ret;
}

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

QByteArray ServerManager::JsonToByteArray(QJsonObject json) {
	return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

