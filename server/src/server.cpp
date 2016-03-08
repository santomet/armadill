#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include "serverconsole.h"

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
//    QCommandLineOption decrypt(QStringList() << "d" << "decrypt", "Decrypt and verify hash");
//    QCommandLineOption encrypt(QStringList() << "e" << "encrypt", "Encrypt and make hash");
//    QCommandLineOption hash(QStringList() << "s" << "hash", "Hash to verify","hash");
    parser.addOption(cert);
    parser.addOption(test);
//    parser.addOption(decrypt);
//    parser.addOption(key);
//    parser.addOption(hash);

    parser.process(a);

    ServerConsole n(&parser, &a);
    //QObject::connect(&n, SIGNAL(exitNormal()), &a, SLOT(exit()));

    QTimer::singleShot(0, &n, SLOT(init()));

    return a.exec();
}

