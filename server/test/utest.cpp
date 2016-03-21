#include "utest.h"
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <QDebug>
#include <QFile>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>


UTest::UTest()
{

}

TEST_CASE( "User registration and verification", "[user]" )
{
    QFile testfile("testdb.sql");
    if(testfile.exists())
        testfile.remove();
    ServerManager mngr("testdb.sql");
    REQUIRE(mngr.newRegistration("keket", "heslo"));
    REQUIRE(!mngr.newRegistration("keket", "heslo"));
    REQUIRE(mngr.newRegistration("keket2", "superheslo"));
    REQUIRE(mngr.getClientDb()->verifyClient("keket", "heslo"));
    REQUIRE(!mngr.getClientDb()->verifyClient("keket", "zleheslo"));
    REQUIRE(mngr.getClientDb()->verifyClient("keket2", "superheslo"));
}

TEST_CASE("Log in user, getting JSON", "[user]")
{
    ServerManager mngr("testdb.sql"); //this should go after first TEST_CASE

    REQUIRE(!mngr.login("keket", "ZLEheslo", "127.0.0.1", 666, "NO_CERT_YET"));
    REQUIRE(mngr.login("keket", "heslo", "127.0.0.1", 666, "NO_CERT_YET"));

    QJsonObject json = mngr.exportOnlineUsersJson();
    QJsonObject test;
    QJsonArray testarray;
    QJsonObject usertest
    {
        { "nick", "keket" },
        { "address", "127.0.0.1" },
        { "port", 666}
    };

    testarray << usertest;
    test.insert("users", testarray);

    REQUIRE((json == test));

    REQUIRE(mngr.login("keket2", "superheslo", "192.168.1.10", 1234, "NO_CERT_YET"));

    json = mngr.exportOnlineUsersJson();
    QJsonObject usertest2
    {
        { "nick", "keket2" },
        { "address", "192.168.1.10" },
        { "port", 1234}
    };
    testarray << usertest2;

    test.remove("users");
    test.insert("users", testarray);

    REQUIRE((json == test));


}


int UTest::makeTests(int argc, char *argv[])
{
    return Catch::Session().run( 1, argv );
}



