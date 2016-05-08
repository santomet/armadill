#ifndef USERINPUTHELPER_H
#define USERINPUTHELPER_H

#include <QObject>
#include <QThread>
#include <iostream>
#include <QDebug>
#include "messages.h"


class UserInputHelper : public QObject
{
    Q_OBJECT
public:
    explicit UserInputHelper(QObject *parent = 0);



signals:
    void inputReady(QString i);

public slots:

protected slots:
    void userInput(); //loop
private:
    QThread * mThread;

};

#endif // USERINPUTHELPER_H
