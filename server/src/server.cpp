#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include "serverconsole.h"
#include "../test/utest.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("armadill-server");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;

    parser.setApplicationDescription("armadill - super secure server");

    parser.addPositionalArgument("port", "Port to listen to");
    parser.addHelpOption();

    QCommandLineOption cert(QStringList() << "c" << "cert", "File for used HTTPS certificate", "file");
    QCommandLineOption database(QStringList() << "d" << "database", "Database file. If does not exist, a new one is created", "file");
    QCommandLineOption test(QStringList() << "t" << "test"); //Run unit tests
    parser.addOption(cert);
    parser.addOption(test);
    parser.addOption(database);

    parser.process(a);

    if(parser.isSet(test))
    {
        UTest test;
        return test.makeTests(argc, argv);
    }

    ServerConsole n(&parser, &a);
    QMetaObject::invokeMethod(&n, "init", Qt::QueuedConnection);

    return a.exec();

}

