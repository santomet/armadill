#ifndef CLIENTDB_H
#define CLIENTDB_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QDateTime>
#include <QString>

struct client
{
    QString clientNick;
    QString passwordHash;
    bool loggedIn;
    QString address;
    int listeningPort;
    QDateTime loginValidUntil;      //NULL if no certificate yet?
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
    bool addNewClient(QString nick, QString passwordHash);

    /*!
     * \brief verifyClient       Verifies if loginClient and passwordHash mathes the db DO NOT ADD LOGGED IN FLAG
     * \param nick
     * \param passwordHash
     * \return                  true if everything's allright, false if wrong password or non-existing user
     */
    bool verifyClient(QString nick, QString passwordHash);

    /*!
     * \brief loginClient       adds loggedIn flag to client, his address and listening port
     *                          and automatically add loginValidUntil 24h from now
     * \param nick
     * \param address
     * \param port
     * \return                  true, false only on error
     */
    bool loginClient(QString nick, QString address, int port);

    /*!
     * \brief logoutCient       set loggedIn flag to false
     * \param nick
     * \return
     */
    bool logoutCient(QString nick);

    /*!
     * \brief getValidUnti         returns the validity of actual certificate (loginValidUntilFor value)
     * \param nick
     */
    QDateTime getValidUntil(QString nick);


    /*!
     * \brief getClientsList        returns a QList of clients, so the parent class can make JSON and send it over
     *                              to clients
     * \return
     */
    QList<client*> *getClientsList();


private:
    QFile mDbFile;  //or better using third-party SQLite-access?
    QList<client*> mClients;

    //TODO Peťo

signals:

public slots:
};

#endif // CLIENTDB_H
