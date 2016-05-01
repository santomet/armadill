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

		class Worker : public std::function<void(QString, qint64, qint64)> {
			Session & session;

		public:
			Worker(Session & session) : session(session) {};

			void operator()(QString path, qint64 start, qint64 len) {
				QFile file(path);
				file.open(QIODevice::ReadOnly);

				file.seek(start);
				QByteArray data = file.read(len);

				ArmaMessage ret;
				SessionKey & key = session.getKey();

				ret.append(session.getMyName().toUtf8().toBase64());
				ret.append(Messages::armaSeparator);
				ret.append(session.getPartnerName().toUtf8().toBase64());
				ret.append(Messages::armaSeparator);
				ret.append(QString::number(QDateTime::currentMSecsSinceEpoch()));
				ret.append(Messages::armaSeparator);

				if (!key.isMyDHCreated()) {
					ret.append('A' + Messages::FileMessageDH);
					ret.append(Messages::armaSeparator);
					ret.append(key.getDH().toBase64());
				}
				else {
					ret.append('A' + Messages::FileMessage);
				}

				ret.append(Messages::armaSeparator);
				ret.append(QString::number(start));
				ret.append(Messages::armaSeparator);
				ret.append(QString::number(len));
				ret.append(Messages::armaSeparator);

				ret.append(key.protect(data, ret));
				key.generateKey();

				//session.send(ret);
			};
		};

		Worker worker;

	public:
		FileSendingContext(Session & session, QString path) : session(session), worker(session) {

		};

		bool startSending() {
			QFuture<void> future = QtConcurrent::run(std::bind(worker, path, 10, 10));
		};
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
