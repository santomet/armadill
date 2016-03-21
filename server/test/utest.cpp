#include "utest.h"
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <QDebug>

UTest::UTest()
{

}

TEST_CASE( "New user registration", "[user]" )
{
    REQUIRE(true);
    ServerManager mngr("testdb.sql");
    mngr.newRegistration("keket", "heslo");
    REQUIRE(mngr.getClientDb()->verifyClient("keket", "heslo"));
    REQUIRE(!mngr.getClientDb()->verifyClient("keket", "zleheslo"));
}


int UTest::makeTests(int argc, char *argv[])
{
    return Catch::Session().run( 1, argv );
}



