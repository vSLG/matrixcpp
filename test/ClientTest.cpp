#include "MatrixCpp/Responses.hpp"
#include <QtTest/QtTest>

#include <MatrixCpp/Client.hpp>
#include <qdebug.h>
#include <qnumeric.h>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

class ClientTest : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        client = new Client("privacytools.io");
        client->loadDiscovery();
    }

    void serverVersion() {
        VersionsResponse response = client->getServerVersion().result();

        if (response.isBroken())
            QSKIP("Response is broken");

        qInfo() << client->homeserverUrl.host()
                << "versions:" << response.versions;

        QVERIFY(response.versions.size() > 0);
    }

    void loginTypes() {
        LoginTypesResponse response = client->getLoginTypes().result();

        if (response.isBroken())
            QSKIP("Response is broken");

        qInfo() << "Server login types:" << response.types;

        QVERIFY(response.types.size() > 0);
        QVERIFY(response.flows.size() > 0);
    }

    void loginAccount() {
        LoginResponse response =
            client->login("<enter_user_here>", "<insert_pw_here>").result();

        if (response.isBroken())
            QSKIP("Response is broken");

        if (response.isError())
            QSKIP("Response is error");

        qInfo() << "Client user id:" << client->userId();
        qInfo() << "Client device id:" << client->deviceId();
        qInfo() << "Client homeserver:" << client->homeserverUrl;

        QVERIFY(!client->userId().isEmpty());
        QVERIFY(!client->deviceId().isEmpty());
        QVERIFY(!client->accessToken().isEmpty());
    }

    void cleanupTestCase() {
        delete client;
    }

  private:
    Client *client = nullptr;
};

QTEST_MAIN(ClientTest)
#include "ClientTest.moc"