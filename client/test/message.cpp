







#include "../src/clientconsole.h"
#include "../../include/catch.hpp"
#include <QFile>

class TestSession {
	Session *s1;
	Session *s2;
public:

	TestSession() {
		mbedtls_entropy_context mtls_entropy;
		mbedtls_entropy_init(&mtls_entropy);
		mbedtls_entropy_gather(&mtls_entropy);

		s1 = new Session("keket", "druhykeket", &mtls_entropy);
		s2 = new Session("druhykeket#@1431", "ke@##$VFSDBket", &mtls_entropy);

		exchangeDH();
	};

	void exchangeDH() {
		//get each other's Diffie Hellman
		s1->getKey().setDH(s2->getKey().getDH());
		s2->getKey().setDH(s1->getKey().getDH());

		//generate private key
		s1->getKey().generateKey();
		s2->getKey().generateKey();
	};

	~TestSession() {
		delete s1;
		delete s2;
	};

	Session & getS1() { return *s1; };
	Session & getS2() { return *s2; };
};


TEST_CASE("Key exchange", "[Krypto]") {
	mbedtls_entropy_context mtls_entropy;
	mbedtls_entropy_init(&mtls_entropy);
	mbedtls_entropy_gather(&mtls_entropy);

	//Create "virtual" sessions for both clients
	Session s("keket", "druhykeket", &mtls_entropy);
	Session s2("druhykeket#@1431", "ke@##$VFSDBket", &mtls_entropy);

	QSet<QString> usedKeys;

	for (int i = 0; i < 20; ++i) {
		//get each other's Diffie Hellman
		s.getKey().setDH(s2.getKey().getDH());
		s2.getKey().setDH(s.getKey().conditionalGetDH());

		//generate private key
		s.getKey().generateKey();
		s2.getKey().generateKey();

		//the key must be the same
		REQUIRE_FALSE(usedKeys.contains(s.getKey().getSharedKey()));
		usedKeys.insert(s.getKey().getSharedKey());
		REQUIRE(usedKeys.contains(s.getKey().getSharedKey()));
		bool same = s.getKey().getSharedKey() == s2.getKey().getSharedKey();
		REQUIRE(same);
		REQUIRE(s.getKey().getSharedKey().length() == 256);
	}

	REQUIRE(usedKeys.size() == 20);
}

TEST_CASE("Sending simple message", "[message]") {
	TestSession s;


	// Send simple message
	Messages m(nullptr, 0);
	QString sprava("TESTOVACIA SPRAVA # wlivywouihfwicdcoywgv aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaas");
	QByteArray encrypted = m.createRegularMessage(s.getS1(), sprava);

	// Recieve simple message
	Messages::ReceivedMessage received;
	bool valid = m.parseMessage(s.getS2(), encrypted, received);
	REQUIRE(valid);

	QString receivedString = QString::fromUtf8(received.messageText);
	REQUIRE(receivedString == sprava);
}

TEST_CASE("Sending complex messages", "[message]") {
	TestSession s;

	Messages m(nullptr, 0);
	QString sprava("TESTOVACIA SPRAVA");
	QByteArray encrypted = m.createRegularMessage(s.getS1(), sprava);
	//	qDebug() << "PROTECTED:   " << encrypted;
	Messages::ReceivedMessage received;
	bool valid = m.parseMessage(s.getS2(), encrypted, received);
	REQUIRE(valid);
	//	qDebug() << "Raw unprotected = " << received.messageText;
	QString receivedString = QString::fromUtf8(received.messageText);

	REQUIRE(receivedString == sprava);

	sprava = "Iná správa s diakritikou. # a špeciálnym znakom ooooha";
	encrypted = m.createRegularMessage(s.getS2(), sprava);
	valid = m.parseMessage(s.getS1(), encrypted, received);
	receivedString = QString::fromUtf8(received.messageText);
	REQUIRE(receivedString == sprava);

	//without key change:

	sprava = "Táto správa sa použije nieko¾kokrát z jednej session";
	for (int i = 0; i<10; ++i) {
		encrypted = m.createRegularMessage(s.getS1(), sprava);
		valid = m.parseMessage(s.getS2(), encrypted, received);
		receivedString = QString::fromUtf8(received.messageText);
		REQUIRE(receivedString == sprava);
	}
	REQUIRE_THROWS_AS(encrypted = m.createRegularMessage(s.getS1(), sprava), KryptoOveruseException);
	REQUIRE_THROWS_AS(encrypted = m.createRegularMessage(s.getS1(), sprava), KryptoOveruseException);

	REQUIRE_FALSE(m.parseMessage(s.getS2(), encrypted, received));
	REQUIRE_FALSE(m.parseMessage(s.getS2(), encrypted, received));
}

TEST_CASE("Sending out of order message", "[message]") {
	TestSession s;

	// Send simple message
	Messages m(nullptr, 0);
	QString sprava("TESTOVACIA SPRAVA # wlivywouihfwicdcoywgv aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaas");
	QByteArray encrypted = m.createRegularMessage(s.getS1(), sprava);

	s.exchangeDH();

	Messages::ReceivedMessage received;
	bool valid;
	for (int i = 0; i < 10; ++i) {
		// Recieve simple message
		REQUIRE_NOTHROW(valid = m.parseMessage(s.getS2(), encrypted, received));
		REQUIRE(valid);

		QString receivedString = QString::fromUtf8(received.messageText);
		REQUIRE(receivedString == sprava);
	}
	REQUIRE_FALSE(m.parseMessage(s.getS2(), encrypted, received));
}

TEST_CASE("Performance tests", "[message]") {
	mbedtls_entropy_context mtls_entropy;
	mbedtls_entropy_init(&mtls_entropy);
	mbedtls_entropy_gather(&mtls_entropy);

	//Create "virtual" sessions for both clients
	Session s("keket", "druhykeket", &mtls_entropy);
	Session s2("druhykeket", "keket", &mtls_entropy);

	//get each other's Diffie Hellman
	s.getKey().setDH(s2.getKey().getDH());
	s2.getKey().setDH(s.getKey().getDH());
	//generate private key
	s.getKey().generateKey();
	s2.getKey().generateKey();

	Messages m(nullptr, 0);

	QString originalMessage("ORIGINAL MESSAGE WITH SIZE = 32B");
	QString sprava;
	QByteArray encrypted;
	bool valid;
	Messages::ReceivedMessage received;
	QString receivedString;

	clock_t encryptStart, encryptEnd, decryptStart, decryptEnd;
	double timeSpentEncrypt, timeSpentDecrypt;

	for (int i = 1; i <= 0; ++i) {
		sprava = originalMessage.repeated(2);
		encryptStart = clock();
		encrypted = m.createRegularMessage(s, sprava);
		encryptEnd = clock();
		decryptStart = clock();
		valid = m.parseMessage(s2, encrypted, received);
		decryptEnd = clock();
		receivedString = QString::fromUtf8(received.messageText);

		timeSpentEncrypt = (encryptEnd - encryptStart) / ((double)CLOCKS_PER_SEC / 1000);
		timeSpentDecrypt = (decryptEnd - decryptStart) / ((double)CLOCKS_PER_SEC / 1000);

		qDebug() << "Message Size: " << sprava.size() << "B, encrypt time: " << qSetRealNumberPrecision(10) << timeSpentEncrypt
			<< "ms, decrypt time: " << timeSpentDecrypt << "ms";

		REQUIRE(receivedString == sprava);

		originalMessage = sprava;

		s.getKey().setDH(s2.getKey().getDH());
		s2.getKey().setDH(s.getKey().getDH());
		//generate private key
		s.getKey().generateKey();
		s2.getKey().generateKey();
	}
}

TEST_CASE("Sending file", "[File sending]") {
	static const char * path = "test.t";
	QFile file(path);
	file.open(QIODevice::WriteOnly);

	QString testData = "ORIGINAL MESSAGE WITH SIZE = 32B\nORIGINAL MESSAGE WITH SIZE = 32B";
	file.write(testData.toUtf8());
	file.close();

	TestSession s;

	Messages::FileSendingContext test(s.getS1(), path);
	REQUIRE_NOTHROW(test.startSending());

	QThread::sleep(3);
}

TEST_CASE("Sending long files", "[File sending]") {
	static const char * path = "test.t";
	QFile file(path);

	TestSession s;

	QString testData = "My little test\nTesting\n";
	for (int i = 1; i < 32; i *= 2) {
		file.open(QIODevice::WriteOnly);
		do {
			testData += testData;
		} while (testData.length() <= i * 2048);
		file.write(testData.toUtf8());
		file.close();

		s.exchangeDH();

		Messages::FileSendingContext test(s.getS1(), path);
		REQUIRE_NOTHROW(test.startSending());
	}
	QThread::sleep(20);
}


