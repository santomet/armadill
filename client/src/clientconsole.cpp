#include "clientconsole.h"

ClientConsole::ClientConsole(QObject *parent) : QObject(parent)
{
    qDebug() << "HELLOOOOOOOOOOOOOOO WORLDDDDDDDDDD";
    exit(0);
}

