#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>

#include <qdebug.h>
#include <qlist.h>
#include <qtestcase.h>

#include "carousel/circle-list.h"
#include "carousel/carousel.h"

class TestCarousel : public QObject
{
    Q_OBJECT

private slots:

    void testCircleList()
    {
        CircleList<int> lst;
        for (size_t i = 0; i < 5; i++) {
            lst.push(i);
        }

        const auto srcList{QList<int>{0, 1, 2, 3, 4}};

        QCOMPARE(lst.list(), srcList);
        lst.rotateL(2);
        QCOMPARE(lst.list(), (QList<int>{2, 3, 4, 0, 1}));
        lst.rotateR(2);
        QCOMPARE(lst.list(), srcList);

        lst.rotateR(2);
        QCOMPARE(lst.list(), (QList<int>{3, 4, 0, 1, 2}));

        lst.reset();
        QCOMPARE(lst.list(), srcList);

        lst.rotateL(2);
        lst.reset();
        QCOMPARE(lst.list(), srcList);
        lst.rotate();
        QCOMPARE(lst.list(), (QList<int>{4, 0, 1, 2, 3}));
        lst.rotate(-2);
        QCOMPARE(lst.list(), (QList<int>{1, 2, 3, 4, 0}));
    }

    void testCarousel()
    {
        Carousel car;
        car.setActive(0);
    }
};

QTEST_MAIN(TestCarousel)

#include "main.moc"
