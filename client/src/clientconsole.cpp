#include "clientconsole.h"

ClientConsole::ClientConsole(QCommandLineParser *parser, QObject *parent) : QObject(parent)
{
    qDebug() << "HELLOOOOOOOOOOOOOOO WORLDDDDDDDDDD";
    exit(0);
}

