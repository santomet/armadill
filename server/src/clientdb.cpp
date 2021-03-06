#include "clientdb.h"
#include "crypto.h"

ClientDb::ClientDb(QString pathToFile, QObject *parent) : QObject(parent)
{
	sqlite3_open_v2(pathToFile.toStdString().c_str(), &mDbFile, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

	sqlite3_stmt * stmt;
	if (sqlite3_prepare_v2(mDbFile, "CREATE TABLE IF NOT EXISTS Clients (username TEXT UNIQUE, password TEXT, ip TEXT, port INTEGER, timeout INTEGER)", -1, &stmt, nullptr) != SQLITE_OK) throw DBException("Can't prepare statement for database creation!");
	if (sqlite3_step(stmt) != SQLITE_DONE)		throw DBException("Can't do statement step for database creation!");
	if (sqlite3_finalize(stmt) != SQLITE_OK)	throw DBException("Can't finalize statement for database creation!");
}

ClientDb::~ClientDb()
{
	sqlite3_close_v2(mDbFile);
}

bool ClientDb::addNewClient(const char *nick, const char *password)
{
	bool ret = true;
	sqlite3_stmt * stmt;
	if (sqlite3_prepare_v2(mDbFile, "INSERT INTO Clients (username, password) VALUES(?, ?)", -1, &stmt, nullptr) != SQLITE_OK) throw DBException("Can't prepare statement in addNewClient!");
	if (sqlite3_bind_text(stmt, 1, nick,			-1, SQLITE_STATIC) != SQLITE_OK) throw DBException("Can't bind statement parameter 1 in addNewClient!");

	std::string hash = pm.hash(password);
	if (sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) throw DBException("Can't bind statement parameter 2 in addNewClient!");
	int e = sqlite3_step(stmt);
	if (e == SQLITE_CONSTRAINT) ret = false;
    else if (e != SQLITE_DONE)	throw DBException("Can't do statement step in addNewClient!");
	
	e = sqlite3_finalize(stmt);
	if (e != SQLITE_OK && e != SQLITE_CONSTRAINT)	throw DBException("Can't finalize statement in addNewClient!");
    return ret;
}

bool ClientDb::verifyClient(const char *nick, const char *password) const
{
	bool valid = false;
	sqlite3_stmt * stmt;
	if(sqlite3_prepare_v2(mDbFile, "SELECT password FROM Clients WHERE username = ?", -1, &stmt, nullptr) != SQLITE_OK) throw DBException("Can't prepare statement in verifyClient!");
	if(sqlite3_bind_text(stmt, 1, nick, -1, SQLITE_STATIC) != SQLITE_OK) throw DBException("Can't bind statement parameter 1 in verifyClient!");
	
	int e = sqlite3_step(stmt);
	if (e != SQLITE_DONE) {
		if (e != SQLITE_ROW) throw DBException("Can't do statement step in verifyClient!");
		if (pm.verify(reinterpret_cast<const char *>(password), reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)))) valid = true;
	}
	if(sqlite3_finalize(stmt) != SQLITE_OK) throw DBException("Can't finalize statement in verifyClient!");
	return valid;
}

bool ClientDb::loginClient(const char * nick, const char * address, int port)
{
	sqlite3_stmt * stmt;
	if (sqlite3_prepare_v2(mDbFile, "UPDATE Clients SET ip = ?, port = ?, timeout = strftime('%s','now','+10 minutes') WHERE username = ?", -1, &stmt, nullptr) != SQLITE_OK) throw DBException("Can't prepare statement in loginClient!");
	if (sqlite3_bind_text(stmt, 1, address, -1, SQLITE_STATIC)	!= SQLITE_OK) throw DBException("Can't bind statement parameter 1 in loginClient!");
	if (sqlite3_bind_int(stmt, 2, port)							!= SQLITE_OK) throw DBException("Can't bind statement parameter 2 in loginClient!");
	if (sqlite3_bind_text(stmt, 3, nick,	-1, SQLITE_STATIC)	!= SQLITE_OK) throw DBException("Can't bind statement parameter 3 in loginClient!");
	if (sqlite3_step(stmt) != SQLITE_DONE)		throw DBException("Can't do statement step in loginClient!");
	if (sqlite3_finalize(stmt) != SQLITE_OK)	throw DBException("Can't finalize statement in loginClient!");

	client * c = new client;
	c->address		= address;
	c->clientNick	= nick;
	c->listeningPort = port;
	c->loggedIn = true;
	c->loginValidUntil = QDateTime::currentDateTime();
    c->loginValidUntil = c->loginValidUntil.addSecs(600);
	mClients.push_back(c);
	return true;
}

bool ClientDb::logoutCient(client * client)
{
	logoutCient(client->clientNick.toStdString().c_str());
	mClients.removeOne(client);
	return true;
}

bool ClientDb::logoutCient(const char * nick)
{
	sqlite3_stmt * stmt;
	if (sqlite3_prepare_v2(mDbFile, "UPDATE Clients SET ip = NULL, port = NULL, timeout = 0 WHERE username = ?", -1, &stmt, nullptr) != SQLITE_OK) throw DBException("Can't prepare statement in logoutCient!");
	if (sqlite3_bind_text(stmt, 1, nick, -1, SQLITE_STATIC) != SQLITE_OK) throw DBException("Can't bind statement parameter 1 in logoutCient!");
	if (sqlite3_step(stmt) != SQLITE_DONE)		throw DBException("Can't do statement step in logoutCient!");
	if (sqlite3_finalize(stmt) != SQLITE_OK)	throw DBException("Can't finalize statement in logoutCient!");
	return true;
}

const QList<client*> & ClientDb::getClientsList() const
{
	return mClients;
}

/*QDateTime ClientDb::getValidUntil(const char * nick)
{

	return QDateTime();
}*/





