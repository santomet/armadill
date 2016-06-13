#include "messages.h"


QSslCertificate Messages::localCert;
QSslKey Messages::localKey;

const char Messages::armaSeparator;
const qint64 Messages::maxThreads;
const qint64 Messages::maxChunkSize;



qint64  Messages::FileSendingContext::transferID;
QMap<qint64, std::shared_ptr<Messages::FileSendingContext>> Messages::FileSendingContext::transfers;

qint64  Messages::FileReceivingContext::transferID;
QMap<qint64, std::shared_ptr<Messages::FileReceivingContext>> Messages::FileReceivingContext::transfers;




Messages::Messages(peer *peerToConnect = nullptr, QObject *parent) : QObject(parent), mPeer(peerToConnect)
{
    //here we create peerconnection session
}

Messages::~Messages()
{

}


Messages::ArmaMessage Messages::createRegularMessage(Session & session, const QString & message) {
	return addMessageHeader(session, message.toUtf8(), Messages::MsgType::RegularMessage, Messages::MsgType::RegularMessageDH);
}

Messages::ArmaMessage Messages::createLoginMessage(QString & name, const QString & password, int port, bool reg) {
	QByteArray req, key;
	Krypto::createCert(key, req, "C=SK,O=Armadill,CN=" + name);
	localKey = QSslKey(key, QSsl::KeyAlgorithm::Rsa);

	ArmaMessage ret;
    ret.append(reg ? "r" : "l");
    ret.append(Messages::armaSeparator);
	ret.append(name.toUtf8().toBase64());
	ret.append(Messages::armaSeparator);
	ret.append(password.toUtf8().toBase64());
    ret.append(Messages::armaSeparator);
    ret.append(QString::number(port).toUtf8().toBase64());
	
	if (!reg) {
		ret.append(Messages::armaSeparator);
		ret.append(req);
	}
	return ret;
}

bool Messages::isJsonMessage(const ArmaMessage& message) {
	return !QJsonDocument::fromJson(message).isNull();
}

bool Messages::parseJsonUsers(ArmaMessage &message, QList<peer>& usersList) {
	usersList.clear();
	QJsonDocument jsonDoc = QJsonDocument::fromJson(message);
	if (jsonDoc.isNull()) {
		return false;
	}
	QJsonObject jsonObject = jsonDoc.object();
	if (jsonObject["users"].isArray()) {
		QJsonArray users = jsonObject["users"].toArray();
		foreach (QJsonValue user, users)
		{
			QJsonObject userObject = user.toObject();
			peer p;
			p.address = userObject["address"].toString();
			p.name = userObject["nick"].toString();
			p.listeningPort = userObject["port"].toInt();
			usersList.append(p);
		}
	}
	else {
		return false;
	}
	return true;
}

bool Messages::parseMessage(std::function<Session &(QString & name)> sessions, const ArmaMessage & message, std::function<void(Session &, MsgType, const ReceivedMessage &)> callback) {
	QString senderNick, receiverNick;
	QByteArray dh;
    QDateTime timestamp;
    MsgType type;

    QList<QByteArray> list = message.split(armaSeparator);

    if (list.size() < 5) throw MessageException("incomplete message");

    senderNick = QString::fromUtf8(QByteArray::fromBase64(list[0]));
    receiverNick = QString::fromUtf8(QByteArray::fromBase64(list[1]));

	Session & session = sessions(senderNick);
	if (session.getMyName() != receiverNick) throw MessageException("Message not for this client.");

	ReceivedMessage parsedMessage;
    timestamp.setMSecsSinceEpoch(list[2].toLongLong());
    parsedMessage.timestamp = timestamp;

    type = static_cast<MsgType>(list[3][0] - 'A');


    QByteArray messageText;

    if(type == Messages::MsgType::PureDH)
    {
        dh = QByteArray::fromBase64(list[4]);

        SessionKey& sk = session.getKey();

        int contextDataLength = list[0].size() + list[1].size() + list[2].size() + list[3].size() + list[4].size() + 5;
        QByteArray messageText;
        try {
            QByteArray a1 = message.left(contextDataLength);
            QByteArray a2 = message.right(message.length() - contextDataLength);
            messageText = sk.unprotect(a2, a1);
        }
        catch (KryptoException e) {
            return false;
        }
        sk.setDH(dh);
        sk.generateKey();

        parsedMessage.messageText = messageText;
		return true;
    }
    else if (type % 2) {
		if (list.size() < 6) throw MessageException("incomplete message");
		dh = QByteArray::fromBase64(list[4]);

		SessionKey& sk = session.getKey();

		int contextDataLength = list[0].size() + list[1].size() + list[2].size() + list[3].size() + list[4].size() + 5;
		QByteArray messageText;
		try {
			QByteArray a1 = message.left(contextDataLength);
			QByteArray a2 = message.right(message.length() - contextDataLength);
			messageText = sk.unprotect(a2, a1);
		}
		catch (KryptoException e) {
			return false;
		}
		sk.setDH(dh);
		sk.generateKey();

		parsedMessage.messageText = messageText;
	}
    //regularMessage
	else {

        SessionKey& sk = session.getKey();
        int contextDataLength = list[0].size() + list[1].size() + list[2].size() + list[3].size() + 4; //number of separators
        try{
            QByteArray a1 = message.left(contextDataLength);
            QByteArray a2 = message.right(message.length() - contextDataLength);
            messageText = sk.unprotect(a2, a1);
        }
        catch (KryptoException e) {
            return false;
        }
        parsedMessage.messageText = messageText;
    }

	callback(session, type, parsedMessage);
    return true;
}

QByteArray Messages::addMessageHeader(Session & session, const QByteArray & payload, Messages::MsgType type, Messages::MsgType typeDH) {
	QByteArray ret;
	SessionKey & key = session.getKey();

	ret.append(session.getMyName().toUtf8().toBase64());
	ret.append(Messages::armaSeparator);
	ret.append(session.getPartnerName().toUtf8().toBase64());
	ret.append(Messages::armaSeparator);
	ret.append(QString::number(QDateTime::currentMSecsSinceEpoch()));
	ret.append(Messages::armaSeparator);

	QByteArray dh = key.conditionalGetDH();
	if (dh.length() > 0) {
		ret.append('A' + typeDH);
		ret.append(Messages::armaSeparator);
		ret.append(dh.toBase64());
	}
	else {
		ret.append('A' + type);
	}
	ret.append(Messages::armaSeparator);

	ret.append(key.protect(payload, ret));

	if (dh.length() > 0) key.generateKey();
	return ret;
}
















void Messages::FileSendingContext::sendFile(Session & s, const QString & path, std::function<void(const QByteArray &)> sender) {
	QByteArray payload;
	auto it = transfers.insert(++transferID, std::shared_ptr<FileSendingContext>(new FileSendingContext(s, path, sender)));

	payload.append(QByteArray::number(transferID));
	payload.append(Messages::armaSeparator);
	payload.append(QByteArray::number((*it)->fileSize));

	sender(Messages::addMessageHeader(s, payload, MsgType::FileContext, MsgType::FileContextDH));
}

void Messages::FileSendingContext::confirmFile(Session & s, const QByteArray & response) {
	QList<QByteArray> list = response.split(Messages::armaSeparator);

	qint64 id = list[0].toLongLong();
	auto it = transfers.find(id);
	(*it)->destID = list[1].toLongLong();
	(*it)->startSending();
}

void Messages::FileReceivingContext::receiveFile(Session & s, const QByteArray & payload, std::function<void(const QByteArray &)> sender) {
	QList<QByteArray> list = payload.split(Messages::armaSeparator);
	qint64 id = list[0].toLongLong();
	qint64 size = list[1].toLongLong();

	QString path = "downloads/";
	path.append(QString::number(++transferID));
	path.append(".rec");

	auto it = transfers.insert(transferID, std::shared_ptr<FileReceivingContext>(new FileReceivingContext(s, path)));

	QByteArray response;
	response.append(QByteArray::number(id));
	response.append(Messages::armaSeparator);
	response.append(QByteArray::number(transferID));

	if(sender) sender(Messages::addMessageHeader(s, response, MsgType::FileResponse, MsgType::FileResponseDH));
	else s.send(Messages::addMessageHeader(s, response, MsgType::FileResponse, MsgType::FileResponseDH));
}

void Messages::FileReceivingContext::receiveChunk(Session & s, const QByteArray & payload) {
	QList<QByteArray> list = payload.split(Messages::armaSeparator);

	qint64 id = list[0].toLongLong();
	qint64 start = list[1].toLongLong();
	qint64 len = list[2].toLongLong();

	auto it = transfers.find(id);
	(*it)->parseChunk(start, len, payload.right(payload.length() - (list[0].length() + list[1].length() + list[2].length() + 3)));
}


Messages::FileSendingContext::FileSendingContext(Session & session, QString path, std::function<void(const QByteArray &)> dataSender) : session(session), path(path), dataSender(dataSender) {
	QFile file(path);
	if(!file.open(QIODevice::ReadWrite)) throw MessageException("Unable to open file!");
	fileSize = file.size();
	file.close();
}


bool Messages::FileSendingContext::startSending() {
	qint64 chunks = (fileSize - 1) / Messages::maxChunkSize + 1;
	qint64 threads = std::min(chunks, maxThreads);
	qint64 done = 0;

	for (qint64 i = 0; i < threads; ++i) {
		workers.push_back(Worker(session, destID, path, dataSender));

		// Load balancer
		qint64 cChunks = ((chunks - 1) / threads + (((chunks - 1) % threads < i) ? 0 : 1));
		qint64 start = done * Messages::maxChunkSize;
		done += cChunks;
		qint64 len = (i == threads - 1) ? (fileSize - start) : (cChunks * Messages::maxChunkSize);

		futures.push_back(QtConcurrent::run(workers.back(), start, len));
	}
	return true;
}

void Messages::FileSendingContext::Worker::operator()(qint64 gstart, qint64 glen) {
	QFile file(path);
	file.open(QIODevice::ReadOnly);
	file.seek(gstart);

	do {
		qint64 len = std::min(glen, maxChunkSize);
		qint64 start = file.pos();
		glen -= len;

		QByteArray payload;
		payload.append(QString::number(destID));
		payload.append(Messages::armaSeparator);
		payload.append(QString::number(start));
		payload.append(Messages::armaSeparator);
		payload.append(QString::number(len));
		payload.append(Messages::armaSeparator);
		payload.append(file.read(len));

		QByteArray ret = addMessageHeader(session, payload, Messages::MsgType::FileMessage, Messages::MsgType::FileMessageDH);

		dataSender(ret);
	} while (glen > 0);
	file.close();
}

Messages::FileReceivingContext::FileReceivingContext(Session & session, QString path) : session(session), path(path) {
	QFile file(path);
	if (!file.open(QIODevice::ReadWrite)) throw MessageException("Unable to open file!");
	fileSize = file.size();
	file.close();
}


void Messages::FileReceivingContext::parseChunk(qint64 start, qint64 len, const QByteArray & data) {
	workers.push_back(Worker(session, path, fileSize));
	futures.push_back(QtConcurrent::run(workers.back(), start, len, data));
}

void Messages::FileReceivingContext::parseChunk(const QByteArray & data) {
	QList<QByteArray> list = data.split(Messages::armaSeparator);

	qint64 start = list[0].toLongLong();
	qint64 len = list[1].toLongLong();

	parseChunk(start, len, data.right(data.length() - (list[0].length() + list[1].length() + 2)));
}

void Messages::FileReceivingContext::Worker::operator()(qint64 start, qint64 len, QByteArray data) {
	

	//if (start + len > fileSize) throw MessageException("File chunk out of range.");

	QFile file(path);
	file.open(QIODevice::ReadWrite);
	file.seek(start);
	file.write(data);
	file.close();
}
