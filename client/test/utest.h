#ifndef UTEST_H
#define UTEST_H

#include <QObject>
#include <QtTest/QTest>
#include "../src/clientconsole.h"

class UTest : public QObject
{
    Q_OBJECT
public:
    explicit UTest(QObject *parent = 0);

signals:

public slots:
};

#endif // UTEST_H
