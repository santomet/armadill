#ifndef MESSAGES_H
#define MESSAGES_H

#include <functional>
#include <memory>

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


/**
 * @brief The Messages class                    Originally intended to be used as session (separate object for every peer) but somehow diverted into a namespace...
 */
class Messages : public QObject
{
    Q_OBJECT
public:
    static const char armaSeparator = '#';
	static const qint64 maxThreads = 8;
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



	// Odd numbers are with DH, even are without DH
    enum MsgType
    {
		None = 0,
		PureDH = 1,
        RegularMessage = 2,
        RegularMessageDH = 3,
        FileMessage = 4,
        FileMessageDH = 5,
		FileContext = 6,
		FileContextDH = 7,
		FileResponse = 8,
		FileResponseDH = 9
    };

    struct FileContext
    {
        unsigned char id[16];
        bool sending;
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

		ReceivedMessage() {};
		ReceivedMessage(const ReceivedMessage & o) : messageText(o.messageText), timestamp(o.timestamp) {};

	};
//-----------------------Public Methods--------------------------------------------------

    Messages(peer *peerToConnect, QObject *parent = 0);
    ~Messages();


	static void callbackHandler(Session & session, MsgType type, const ReceivedMessage & payload) {
		switch (type) {
		case RegularMessage, RegularMessageDH:

			break;
		case FileMessage:
		case FileMessageDH:
			FileReceivingContext::receiveChunk(session, payload.messageText);
			break;
		case FileContext:
		case FileContextDH:
			// FileReceivingContext::receiveFile(session, payload, );
			break;
		}
	};



	Session & getSessionFromName(QString & name);

    /*!
     * \brief parseMessage              Parses message and makes proper actions
     *                                  Distinguishes between different types and sends the message
     *                                  part of ArmaMessage to another method
     * \param message                   message
     * \return                          true if everything goes allright
     */
    static bool parseMessage(std::function<Session &(QString & name)> sessions, const ArmaMessage & message, std::function<void(Session &, MsgType, const ReceivedMessage &)> callback);


    /*!
     * \brief addMessageHeader
     * \param session
     * \param payload
     * \param type                      We don't need DH
     * \param typeDH                    we neeeeed!
     * \return
     */

	static QByteArray addMessageHeader(Session & session, const QByteArray & payload, Messages::MsgType type, Messages::MsgType typeDH);

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
    ArmaMessage createLoginMessage(QString & name, const QString & password, int port, bool reg = false);


//---------------------------Files---------------------------------------------------
	class FileSendingContext {
		static qint64  transferID;
		static QMap<qint64, std::shared_ptr<FileSendingContext>> transfers;
	public:
		static void sendFile(Session & s, const QString & path, std::function<void(const QByteArray &)>);
		static void confirmFile(Session & s, const QByteArray & response);
	private:
		Session & session;
		QString path;
		qint64 fileSize;
		qint64 destID;
		std::function<void(const QByteArray &)> dataSender;

		class Worker {
			Session & session;
			QString path;
			qint64 destID;
			std::function<void(QByteArray &)> dataSender;
		public:
			Worker(Session & session, qint64 destID, QString path, std::function<void(const QByteArray &)> dataSender) : session(session), destID(destID), path(path), dataSender(dataSender) { };
			Worker(const Worker & w) : session(w.session), destID(w.destID), path(w.path), dataSender(w.dataSender) {};

			void operator()(qint64 gstart, qint64 glen);
		};

		std::vector<Worker> workers;
		std::vector<QFuture<void>> futures;

	public:
		FileSendingContext(Session & session, QString path, std::function<void(const QByteArray &)> dataSender);

		bool startSending();
	};


	class FileReceivingContext {
		static qint64  transferID;
		static QMap<qint64, std::shared_ptr<FileReceivingContext>> transfers;
	public:
		static void receiveFile(Session & s, const QByteArray & payload, std::function<void(const QByteArray &)> sender = std::function<void(const QByteArray &)>());
		static void receiveChunk(Session & s, const QByteArray & payload);
	private:
		Session & session;
		QString path;
		qint64 fileSize;
		qint64 originID;

		class Worker {
			Session & session;
			QString path;
			qint64 fileSize;
		public:
			Worker(Session & session, QString path, qint64 fileSize) : session(session), path(path), fileSize(fileSize) { };
			Worker(const Worker & w) : session(w.session), path(w.path), fileSize(fileSize) {};

			void operator()(qint64 start, qint64 len, QByteArray data);
		};

		std::vector<Worker> workers;
		std::vector<QFuture<void>> futures;

	public:
		FileReceivingContext(Session & session, QString path);

		void parseChunk(const QByteArray & data);
		void parseChunk(qint64 start, qint64 len, const QByteArray & data);
		void operator()(qint64 start, qint64 len, const QByteArray & data) { parseChunk(start, len, data); };
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

};

class MessageException : public std::runtime_error {
public:
	MessageException(const char * msg) : std::runtime_error(msg) {};
};

#endif // MESSAGES_H
