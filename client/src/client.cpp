#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include "clientconsole.h"

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

//    QCommandLineOption key(QStringList() << "k" << "key", "Password for encryption/decryption. It will be hashed with SHA256 and the digest then used as a key", "password");
//    QCommandLineOption decrypt(QStringList() << "d" << "decrypt", "Decrypt and verify hash");
//    QCommandLineOption encrypt(QStringList() << "e" << "encrypt", "Encrypt and make hash");
//    QCommandLineOption hash(QStringList() << "s" << "hash", "Hash to verify","hash");
//    parser.addOption(encrypt);
//    parser.addOption(decrypt);
//    parser.addOption(key);
//    parser.addOption(hash);

    parser.process(a);

    ClientConsole n(&parser, &a);
    //QObject::connect(&n, SIGNAL(exitNormal()), &a, SLOT(exit()));

    QTimer::singleShot(0, &n, SLOT(init()));

    return a.exec();
}
