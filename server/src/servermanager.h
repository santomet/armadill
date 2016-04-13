#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDateTime>
#include <QByteArray>
#include <QString>
#include "clientdb.h"

class ServerManager : public QObject
{
    Q_OBJECT
private:
    ClientDb clientDatabase;
	const char armaSeparator = '#';
public:
    ClientDb *getClientDb() {return &clientDatabase;}

    ServerManager(QString path) : clientDatabase(path) {}

    /*!
     * \brief newRegistration               Registers a new user
     * \param nickName
     * \param password
     * \return                              True if successfully registered, false if something went wrong/user already exists
     */
    bool newRegistration(QString nickName, QString password);

    /*!
     * \brief login                         Tries to authentify new user and set his state as logged in
     * \param nickname
     * \param password
     * \param address                       IP/Domain of client
     * \param port                          client's port
     * \param cert                          base64-formatted certificate signing request
     * \return                              true if passwords matches and everything goes fine, false if not
     */
    bool login(QString nickname, QString password, QString address, int port, QString cert);

	/*!
	* \brief exportOnlineUsersJson        returns a JSON of online users
	*                              
	* \return
	*/
	QJsonObject exportOnlineUsersJson();

	/*!
	* \brief parseLoginMessage		parses user's login message
	*
	* \return						true if all went right
	*/
	bool parseLoginMessage(QByteArray& message, QString& nickname, QString& password);

};

#endif
