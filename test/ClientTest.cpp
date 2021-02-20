// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QFile>
#include <QUrlQuery>
#include <QtTest/QtTest>

#include <MatrixCpp/Client.hpp>
#include <MatrixCpp/Responses.hpp>
#include <cstdlib>
#include <olm/olm.h>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

class ClientTest : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
        // Use path provided by CMake
        QFile accFile("@ACCOUNT_INFO@");

        OlmAccount *account = olm_account(malloc(olm_account_size()));
        qDebug() << olm_account_last_error(account);

        int            randSize    = sizeof(rand());
        unsigned char *randomBytes = (unsigned char *) malloc(randSize * 64);

        srand(time(NULL));

        for (int i = 0; i < 64; i++)
            randomBytes[i * randSize] = rand();

        olm_create_account(account, randomBytes, randSize * 64);
        qDebug() << olm_account_last_error(account);

        free(randomBytes);

        int   size = olm_account_identity_keys_length(account);
        char *keys = (char *) malloc(size);

        olm_account_identity_keys(account, keys, size);
        qDebug() << olm_account_last_error(account);
        qDebug() << keys;
        free(keys);

        if (!accFile.exists())
            qFatal(
                "%s",
                (accFile.fileName() +
                 " does not exist. Please create a plain text file with the "
                 "following format: '<homeserver_host> <user_id> <device_id> "
                 "<access_token>'")
                    .toStdString()
                    .c_str());

        accFile.open(QFile::ReadOnly);
        QByteArray content = accFile.readAll();
        accFile.close();

        QList<QByteArray> parts = content.split(' ');

        client = new Client(QUrl(parts[0]));
        client->restore(parts[1], parts[2], parts[3]);
        client->loadDiscovery();
    }

    void serverVersion() {
        VersionsResponse response = client->getServerVersion()->result();

        if (response.isBroken())
            QSKIP("Response is broken");

        QVERIFY(response.versions.size() > 0);

        qInfo() << client->homeserverUrl.host()
                << "versions:" << response.versions;
    }

    void loginTypes() {
        LoginTypesResponse response = client->getLoginTypes()->result();

        if (response.isBroken())
            QSKIP("Response is broken");

        QVERIFY(response.types.size() > 0);
        QVERIFY(response.flows.size() > 0);

        qInfo() << "Server login types:" << response.types;
    }

    void passwordLogin() {
        if (!client->accessToken().isEmpty())
            QSKIP("Already have access token");

        LoginResponse response = client->login("<secret>")->result();

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
        SyncResponse response = client->sync()->result();

        if (response.isBroken())
            QSKIP("Response is broken");

        if (response.isError())
            QSKIP("Response is error");

        QVERIFY(!response.nextBatch.isEmpty());

        do {
            SyncResponse response =
                client
                    ->sync(
                        "", "", false, MatrixCpp::Client::PRESENCE_ONLINE, 7000)
                    ->result();
        } while (0);
    }

    void cleanupTestCase() {
        delete client;
    }

  private:
    Client *client = nullptr;
};

QTEST_MAIN(ClientTest)
#include "ClientTest.moc"