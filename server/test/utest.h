#ifndef UTEST_H
#define UTEST_H

#include <QObject>
#include "../src/serverconsole.h"

class UTest
{
public:
    UTest();

    int makeTests(int argc, char *argv[]);
};

#endif // UTEST_H
