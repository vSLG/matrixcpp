#include <QtTest/QtTest>

#include <MatrixCpp/Client.hpp>
#include <qdebug.h>

class ClientTest : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        client = new MatrixCpp::Client(QUrl("https://matrix.org"));
    }

    void test() {
        QCOMPARE(true, true);
    }

  private:
    MatrixCpp::Client *client = nullptr;
};

QTEST_MAIN(ClientTest)
#include "ClientTest.moc"