#include <QtTest/QtTest>

class ClientTest : public QObject {
    Q_OBJECT

  private slots:
    void test() {
        QCOMPARE(true, true);
    }
};

QTEST_MAIN(ClientTest)
#include "ClientTest.moc"