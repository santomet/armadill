#ifndef UTEST_H
#define UTEST_H

#include <QObject>
#include <QtTest/QTest>

class UTest
{
public:
    UTest();
    int makeTests(int argc, char *argv[]);
};

#endif // UTEST_H
