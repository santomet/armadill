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
//    QCommandLineOption decrypt(QStringList() << "d" << "decrypt", "Decrypt and verify hash");
//    QCommandLineOption encrypt(QStringList() << "e" << "encrypt", "Encrypt and make hash");
//    QCommandLineOption hash(QStringList() << "s" << "hash", "Hash to verify","hash");
//    parser.addOption(encrypt);
//    parser.addOption(decrypt);
//    parser.addOption(key);
    parser.addOption(test);

    parser.process(a);

    //parse things, run test if test
    if(parser.isSet(test))
    {
        UTest nt(&a);
    }
    else
    {
    ClientConsole n(&a);
    //QObject::connect(&n, SIGNAL(exitNormal()), &a, SLOT(exit()));
    QTimer::singleShot(0, &n, SLOT(init()));
    }

    return a.exec();
}
