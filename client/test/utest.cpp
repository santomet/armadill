#include "utest.h"
#define CATCH_CONFIG_RUNNER
#include "../../include/catch.hpp"

UTest::UTest()
{

}


TEST_CASE( "Creating message", "[message]" )
{
    mbedtls_entropy_context mtls_entropy;
    mbedtls_entropy_init(&mtls_entropy);
    mbedtls_entropy_gather(&mtls_entropy);

    Session s("keket", "druhykeket", &mtls_entropy);

    Session s2("druhykeket", "keket", &mtls_entropy);

    s.getKey().setDH(s2.getKey().getDH());
    s2.getKey().setDH(s.getKey().getDH());
	s.getKey().generateKey();
	s2.getKey().generateKey();

	bool same = s.getKey().getSharedKey() == s2.getKey().getSharedKey();
	REQUIRE(same);

    Messages m(nullptr, 0);
    QString sprava("TESTOVACIA SPRAVA");

    QByteArray encrypted = m.createRegularMessage(s, sprava);

	qDebug() << "PROTECTED:   " << encrypted;

    Messages::ReceivedMessage received;
    bool valid = m.parseMessage(s2, encrypted, received);

	REQUIRE(valid);

	QString receivedString;
	receivedString.fromUtf8(received.messageText);

	qDebug() << "UNPROTECTED: " << receivedString;

    REQUIRE(receivedString == sprava);


}



int UTest::makeTests(int argc, char *argv[])
{
    return Catch::Session().run( 1, argv );

//    mbedtls_entropy_context mtls_entropy;

//    Session mSession("keket", "dalsikeket", &mtls_entropy);

//    Messages messages(nullptr, 0);

//    QString daco = messages.createRegularMessage(mSession, "KOKOTINA");

//    qDebug() << daco;


}
