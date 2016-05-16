#ifndef COMMON
#define COMMON


#include "krypto.h"


/**
* @brief The Session class                        Basically a messaging session, different for every peer. Works with DH
*/
class Session {
	QString myName;
	QString otherName;
	SessionKey key;

	std::function<void(const QByteArray &)> sender;
public:
	Session(QString name, QString otherName, mbedtls_entropy_context * entropy) : myName(name), otherName(otherName), key(entropy) { };

	virtual ~Session() {};

	/*!
	* \brief getMyName
	* \return                          Returns client name
	*/
	const QString & getMyName() const { return myName; };

	/*!
	* \brief getPartnerName
	* \return                          Returns partners name
	*/
	const QString & getPartnerName() const { return otherName; };

	/*!
	* \brief getKey
	* \return                          Returns SessionKey of the session
	*/
	SessionKey & getKey() { return key; };


	void send(const QByteArray & data) { sender(data); };
};




//----------peers
/*!
     * \brief The peer struct
     */
struct peer
{
    QString name;
    QString address;
    qint16 listeningPort;
	Session *session = nullptr;
};

#endif // COMMON

