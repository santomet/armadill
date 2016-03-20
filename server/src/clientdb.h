#ifndef CLIENTDB_H
#define CLIENTDB_H

#include "../../include/sqlite3.h"

#include <QObject>
#include <QList>
#include <QFile>
#include <QDateTime>
#include <QString>

struct client
{
    QString clientNick;
    bool loggedIn;
    QString address;
    int listeningPort;
    QDateTime loginValidUntil;
};


class DBException : public std::exception {
	int id;
public:
	DBException(const char *msg, int id = -1) : std::exception(msg), id(id) {};
	int getID() const { return id; };
};


class ClientDb : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief ClientDb          Creates a Db object
     * \param pathToFile        path to SQLite file
     * \param parent            0
     */
    explicit ClientDb(QString pathToFile, QObject *parent = 0);

    /*!
      Saves the db!!
    */
    ~ClientDb();

    /*!
     * \brief addNewClient      Adds new client to the db
     * \param nick              nick of new client
     * \param passwordHash      pwd hash
     * \return                  true if successfully added, false if nick is already used
     */
    bool addNewClient(const char *nick, const char *passwordHash);

    /*!
     * \brief verifyClient       Verifies if loginClient and passwordHash mathes the db DO NOT ADD LOGGED IN FLAG
     * \param nick
     * \param passwordHash
     * \return                  true if everything's allright, false if wrong password or non-existing user
     */
    bool verifyClient(const char *nick, const char *passwordHash) const;

    /*!
     * \brief loginClient       adds loggedIn flag to client, his address and listening port
     *                          and automatically add loginValidUntil 24h from now
     * \param nick
     * \param address
     * \param port
     * \return                  true, false only on error
     */
    bool loginClient(const char *nick, const char *address, int port);

	/*!
	* \brief logoutCient       logout user from database and remove from online client list
	* \param nick
	* \return
	*/
	bool logoutCient(client *client);

    /*!
     * \brief logoutCient       logout user from database
     * \param nick
     * \return
     */
    bool logoutCient(const char *nick);

    /*!
     * \brief getValidUnti         returns the validity of actual certificate (loginValidUntilFor value)
     * \param nick
     */
    // QDateTime getValidUntil(const char *nick);
	//Useless?? :D

    /*!
     * \brief getClientsList        returns a QList of clients, so the parent class can make JSON and send it over
     *                              to clients
     * \return
     */
    const QList<client*> &getClientsList() const;


private:
	sqlite3 *mDbFile;  //or better using third-party SQLite-access?
    QList<client*> mClients;

signals:

public slots:
};

#endif // CLIENTDB_H
