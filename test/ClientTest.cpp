#include <QFile>
#include <QUrlQuery>
#include <QtTest/QtTest>

#include <MatrixCpp/Client.hpp>
#include <MatrixCpp/Responses.hpp>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

class ClientTest : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        QFile accFile("account_info");

        if (!accFile.exists())
            qFatal(
                "%s",
                (accFile.fileName() + " does not exist").toStdString().c_str());

        accFile.open(QFile::ReadOnly);
        QByteArray content = accFile.readAll();
        accFile.close();

        QList<QByteArray> parts = content.split(' ');

        client = new Client("matrix.org");
        client->restore(parts[0], parts[1], parts[2]);
        client->loadDiscovery();
    }

    void serverVersion() {
        VersionsResponse response = client->getServerVersion().result();

        if (response.isBroken())
            QSKIP("Response is broken");

        QVERIFY(response.versions.size() > 0);

        qInfo() << client->homeserverUrl.host()
                << "versions:" << response.versions;
    }

    void loginTypes() {
        LoginTypesResponse response = client->getLoginTypes().result();

        if (response.isBroken())
            QSKIP("Response is broken");

        QVERIFY(response.types.size() > 0);
        QVERIFY(response.flows.size() > 0);

        qInfo() << "Server login types:" << response.types;
    }

    void passwordLogin() {
        if (!client->accessToken().isEmpty())
            QSKIP("Already have access token");

        LoginResponse response = client->login("<secret>").result();

        if (response.isError() || response.isBroken())
            qFatal("Failed to log in");

        QVERIFY(!client->userId().isEmpty());
        QVERIFY(!client->deviceId().isEmpty());
        QVERIFY(!client->accessToken().isEmpty());

        qInfo() << "Client user id:" << client->userId();
        qInfo() << "Client device id:" << client->deviceId();
        qInfo() << "Client homeserver:" << client->homeserverUrl;
    }

    void sync() {
        SyncResponse response = client->sync().result();

        if (response.isBroken())
            QSKIP("Response is broken");

        if (response.isError())
            QSKIP("Response is error");

        QVERIFY(!response.nextBatch.isEmpty());
    }

    void cleanupTestCase() {
        delete client;
    }

  private:
    Client *client = nullptr;
};

QTEST_MAIN(ClientTest)
#include "ClientTest.moc"