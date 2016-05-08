#include "utest.h"
#define CATCH_CONFIG_RUNNER
#include "../../include/catch.hpp"

UTest::UTest()
{

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
