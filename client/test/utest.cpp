#include "utest.h"

UTest::UTest(QObject *parent) : QObject(parent)
{
    //I did not really understood the QtTest yet
 //   QMetaObject::invokeMethod(parent,"finish", Qt::QueuedConnection);
    exit(0);
}

