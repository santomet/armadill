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

    /*!
     * \brief newRegistration               Registers a new user
     * \param nickName
     * \param passwordHash
     * \return                              True if successfully registered, false if something went wrong/user already exists
     */
    bool newRegistration(QString nickName, QString passwordHash);

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
};

#endif
