#include "utest.h"
#define CATCH_CONFIG_RUNNER
#include "../../include/catch.hpp"

UTest::UTest()
{

}


TEST_CASE( "Creating message basics", "[message]" )
{
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

    //the key must be the same
	bool same = s.getKey().getSharedKey() == s2.getKey().getSharedKey();
	REQUIRE(same);


    Messages m(nullptr, 0);
    QString sprava("TESTOVACIA SPRAVA");
    QByteArray encrypted = m.createRegularMessage(s, sprava);
//	qDebug() << "PROTECTED:   " << encrypted;
    Messages::ReceivedMessage received;
    bool valid = m.parseMessage(s2, encrypted, received);
	REQUIRE(valid);
//	qDebug() << "Raw unprotected = " << received.messageText;
	QString receivedString = QString::fromUtf8(received.messageText);

    REQUIRE(receivedString == sprava);

    sprava = "Iná správa s diakritikou. # a špeciálnym znakom ooooha";
    encrypted = m.createRegularMessage(s2, sprava);
    valid = m.parseMessage(s, encrypted, received);
    receivedString = QString::fromUtf8(received.messageText);
    REQUIRE(receivedString == sprava);

    //without key change:

    sprava = "Táto správa sa použije niekoľkokrát z jednej session";
    for(int i = 0; i<12; ++i)
    {
        if(i>10)
            CHECK_THROWS(encrypted = m.createRegularMessage(s, sprava));
        else
        {
            encrypted = m.createRegularMessage(s, sprava);
            valid = m.parseMessage(s2, encrypted, received);
            receivedString = QString::fromUtf8(received.messageText);
            REQUIRE(receivedString == sprava);
        }
    }



}

TEST_CASE( "Performance tests", "[message]" )
{
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

    for(int i=1;i<=20;++i)
    {
        sprava = originalMessage.repeated(2);
        encryptStart = clock();
        encrypted = m.createRegularMessage(s, sprava);
        encryptEnd = clock();
        decryptStart = clock();
        valid = m.parseMessage(s2, encrypted, received);
        decryptEnd = clock();
        receivedString = QString::fromUtf8(received.messageText);

        timeSpentEncrypt = (encryptEnd - encryptStart) / ((double)CLOCKS_PER_SEC/1000);
        timeSpentDecrypt = (decryptEnd - decryptStart) / ((double)CLOCKS_PER_SEC/1000);

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



int UTest::makeTests(int argc, char *argv[])
{
    return Catch::Session().run( 1, argv );

//    mbedtls_entropy_context mtls_entropy;

//    Session mSession("keket", "dalsikeket", &mtls_entropy);

//    Messages messages(nullptr, 0);

//    QString daco = messages.createRegularMessage(mSession, "KOKOTINA");

//    qDebug() << daco;


}
