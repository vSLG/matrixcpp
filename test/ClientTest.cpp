#include "MatrixCpp/Responses.hpp"
#include <QtTest/QtTest>

#include <MatrixCpp/Client.hpp>
#include <qdebug.h>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

class ClientTest : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        client = new Client("matrix.org");
    }

    void serverVersion() {
        VersionsResponse response = client->getServerVersion().result();

        qInfo() << client->homeserverUrl.host()
                << "versions:" << response.versions;

        if (response.isBroken())
            QSKIP("Response is broken");

        QVERIFY(response.versions.size() > 0);
    }

    void cleanupTestCase() {
        delete client;
    }

  private:
    Client *client = nullptr;
};

QTEST_MAIN(ClientTest)
#include "ClientTest.moc"