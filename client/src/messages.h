#ifndef MESSAGES_H
#define MESSAGES_H

#include <functional>
#include <QObject>
#include "krypto.h"
#include "peerconnection.h"
#include "common.h"
#include <QFile>
#include <QQueue>
#include <QVector>
#include <QList>
#include <QTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QtConcurrent>

class Session {
	QString myName;
	QString otherName;
    SessionKey key;
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
};


class Messages : public QObject
{
    Q_OBJECT
public:
    static const char armaSeparator = '#';
	static const qint64 maxChunkSize = 2048;

//-----------------------------Structures and types-------------------------------------
    /*!
     * \brief ArmaMessage               ArmaMessage should looks like this:
     *                                  Sender's nick||Receiver's nick||Timestamp||TYPE||DH||Used Key identificator||HMAC||Message
     *                                  where || concat is non-base64 char defined in armaSeparator
     *                                  Message is binary, HMAC and DH are base64
     */
    typedef QByteArray ArmaMessage;

    /*!
     * \brief FileChunk                 NOT EXACTLY DEFINED YET
     */
    typedef QByteArray FileChunkEncrypted;
    typedef QByteArray FileChunkDecrypted;


    enum MsgType
    {
        RegularMessage = 0,
        RegularMessageDH = 1,
        FileMessage = 2,
        FileMessageDH = 3,
        PureDH = 4
    };

    struct FileContext
    {
        unsigned char id[16]; //shortened hash, maybe? TODO
        bool sending; //true if file is being sent, false if received
        QFile *file = nullptr;
        size_t wholeSize = 0;
        size_t processedSize = 0;
        int allParts = 0;
        int processedParts = 0;
    };

	struct ReceivedMessage
	{
		QByteArray messageText;
		QDateTime timestamp;

	};
//-----------------------Public Methods--------------------------------------------------

    Messages(peer *peerToConnect, QObject *parent = 0);
    ~Messages();

    /*!
     * \brief parseMessage              Parses message and makes proper actions
     *                                  Distinguishes between different types and sends the message
     *                                  part of ArmaMessage to another method
     * \param message                   message
     * \return                          true if everything goes allright
     */
    bool parseMessage(Session & session, ArmaMessage &message, ReceivedMessage & receivedMessage);

    /*!
     * \brief createRegularMessage      Creates regular ArmaMessage that will be ready to send
	 * \param session					Session for which the message is intended for
     * \param message                   message
     * \return                          ArmaMessage which can be sent to peer
     */
    ArmaMessage createRegularMessage(Session & session, const QString & message);

	/*!
	* \brief createLoginMessage			Creates login ArmaMessage that will be ready to send to server
	* \param name						client name
	* \param password					password for client name
    * \param reg                        If it is going to be registration attempt
	* \return							ArmaMessage which can be sent to server
	*/
    ArmaMessage createLoginMessage(QString & name, const QString & password, bool reg = false);


//---------------------------Files---------------------------------------------------
	class FileSendingContext {
		Session & session;
		QString path;
		qint64 fileSize;

		class Worker {
			Session & session;
			QString path;
		public:
			Worker(Session & session, QString path) : session(session), path(path) {};
			Worker(const Worker & w) : session(w.session), path(w.path) {};

			void operator()(qint64 gstart, qint64 glen);
		};

		std::vector<Worker> workers;
		std::vector<QFuture<void>> futures;

	public:
		static const qint64 maxThreads = 8;
		FileSendingContext(Session & session, QString path);

		bool startSending();
	};


	class FileReceivingContext {
		Session & session;
		QString path;

	public:
		FileReceivingContext(Session & session, QString path) : session(session) {

		};
	};
    /*!
     * \brief createFileSendingContext  Prepares File for sending to peger
     *                                  - creates a context which getFileMessage() will be working with
     * \param path                      path to file
     * \param receiver                  intended receiver of a file
     * \return                          true if everything goes well
     */
	// FileContext * createFileSendingContext(Session & session, QString path);

    /*!
     * \brief getFileMessage
     * \return                          Returns an ArmaMessage*
     */
    ArmaMessage* getFileMessage();
	/*!
	 * \brief parseJsonUsers	parses received list of logged in users
	 *
	 * \param message			received Json
	 * \param users				parsed list of users
	 * \return					true if everything goes well
	 */
	bool parseJsonUsers(ArmaMessage &message, QList<peer>& users);
	/*!
	 * \brief isJsonMessage		checks if message contains parseable json
	 * \return					true if json
	 */
	bool isJsonMessage(const ArmaMessage &message);
//---------------------------Others---------------------------------------------------
    Krypto mKrypto;
    peer *mPeer;    //actual peer we are communicating with

private:
    /*!
     * \brief parseFileMessage          Parses File data block (creates file for first one)
     * \param encryptedData
     * \return
     */
    bool parseFileMessage(const FileChunkEncrypted &fileChunk);

	
    ArmaMessage* createFileMessage(FileContext *context, QString eceiver);

    QList<FileContext*>  mFileContexts;
    QList<FileChunkDecrypted*> mUnorderedFileBlocks;


};

class MessageException : public std::runtime_error {
public:
	MessageException(const char * msg) : std::runtime_error(msg) {};
};

#endif // MESSAGES_H
