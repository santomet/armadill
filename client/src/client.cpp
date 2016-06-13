#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include "clientconsole.h"
#include "../test/utest.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("armadill");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;

    parser.setApplicationDescription("armadill- super secure IM");

    parser.addPositionalArgument("server", "Server ip to connect to");
    parser.addPositionalArgument("port", "port of that server");
    parser.addHelpOption();

    QCommandLineOption test(QStringList() << "t" << "test", "Run tests");
    parser.addOption(test);

    parser.process(a);

    if(parser.isSet(test))
    {
        UTest test;
        return test.makeTests(argc, argv);
    }

    //parse things, run test if test
	QTextStream out(stdout);
    ClientConsole n(parser.positionalArguments(), out, &a);
    QMetaObject::invokeMethod(&n, "init", Qt::QueuedConnection);

    return a.exec();
}
