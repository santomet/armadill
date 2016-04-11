#include "utest.h"
#define CATCH_CONFIG_RUNNER
#include "../../include/catch.hpp"
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
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
    REQUIRE(!mngr.login("neexistujucikeket", "heslo", "127.0.0.1", 666, "NO_CERT_YET"));

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



TEST_CASE("Performance tests", "[perf]")
{
    ServerManager mngr("testdb.sql");

    REQUIRE(mngr.login("keket", "heslo", "127.0.0.1", 666, "NO_CERT_YET"));

    QJsonObject json;



    clock_t start, end;
    double spent;
    QElapsedTimer timer;

    timer.restart();
    start = clock();
    json = mngr.exportOnlineUsersJson();
    end = clock();
    spent = (end-start) / (double)(CLOCKS_PER_SEC/1000);
    qDebug() << "getting list of users with three entries. CPU: " << spent << "ms, FULL: " << timer.elapsed() << "ms";

    for(int i = 3;i<103;++i)
    {
        timer.restart();
        start = clock();
        REQUIRE(mngr.newRegistration(QString("keket" + QString::number(i)), "heslo"));
        end = clock();
        spent = (end-start) / (double)(CLOCKS_PER_SEC/1000);
        qDebug() << "writing " << i << "th user. CPU: " << spent << "ms, FULL:" << timer.elapsed() << "ms";
    }

    for(int i = 3;i<103;++i)
    {
        timer.restart();
        start = clock();
        REQUIRE(mngr.login(QString("keket" + QString::number(i)), "heslo", "127.0.0.1", 666, "NO_CERT_YET"));
        end = clock();
        spent = (end-start) / (double)(CLOCKS_PER_SEC/1000);

        qDebug() << "Logging in " << i << "th user. CPU: " << spent << "ms, FULL:" << timer.elapsed() << "ms";

        json = mngr.exportOnlineUsersJson();
        end = clock();
        spent = (end-start) / (double)(CLOCKS_PER_SEC/1000);
        qDebug() << "getting list of users. CPU: " << spent << "ms, FULL: " << timer.elapsed() << "ms";
    }




}


int UTest::makeTests(int argc, char *argv[])
{
    return Catch::Session().run( 1, argv );
}



