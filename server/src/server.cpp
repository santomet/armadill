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
    QCommandLineOption test(QStringList() << "t" << "test"); //Run unit tests
    parser.addOption(cert);
    parser.addOption(test);


    parser.process(a);

    if(parser.isSet(test))
    {
        UTest test;
        return test.makeTests(argc, argv);
    }
    else
    {
        ServerConsole n(&parser, &a);
        //QObject::connect(&n, SIGNAL(exitNormal()), &a, SLOT(exit()));

        QTimer::singleShot(0, &n, SLOT(init()));

        return a.exec();
    }
    return 1;
}

