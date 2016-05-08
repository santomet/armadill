#include "userinputhelper.h"

UserInputHelper::UserInputHelper(QObject *parent) : QObject(parent)
{
    mThread = new QThread();
    connect(mThread, SIGNAL(started()), this, SLOT(userInput()));
    this->moveToThread(mThread);
    mThread->start();
}

void UserInputHelper::userInput()
{
    forever
    {
        std::string line;
        std::getline(std::cin, line);
        QString Qline = QString::fromStdString(line);
        emit inputReady(Qline);
    }
}
