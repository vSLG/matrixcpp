// SPDX-License-Identifier: LGPL-3.0-or-later

#include "MatrixCpp/Responses.hpp"
#include <QtTest/QtTest>
#include <iostream>

#include <MatrixCpp/Client.hpp>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

class LoginTest : public QObject {
    Q_OBJECT

  private slots:

    void login() {
        QUrl homeserver;

        do {
            std::cout << "Enter homeserver url (with schema): " << std::flush;
            std::string line;
            std::getline(std::cin, line);
            homeserver.setUrl(line.c_str());
        } while (!homeserver.isValid());

        Client *client = new Client(homeserver, this);
        client->loadDiscovery();

        std::string userId, password;

        std::cout << "Enter local or fully qualified user ID: " << std::flush;
        std::getline(std::cin, userId);
        std::cout << "Enter password for this user ID: " << std::flush;
        std::getline(std::cin, password);

        qInfo("Trying to log in...");

        LoginResponse response(
            client->login(userId.c_str(), password.c_str())->result());

        QVERIFY(!response.isError() && !response.isBroken());

        qInfo("Login successful");
        qInfo("Saving credentials to file");

        QString fileContent(homeserver.toString() + " " + client->userId() +
                            " " + client->deviceId + " " +
                            client->accessToken());

        QFile file("@ACCOUNT_INFO@");
        file.remove();
        file.open(QFile::WriteOnly);
        file.write(fileContent.toUtf8());
        file.close();
    }
};

QTEST_MAIN(LoginTest)
#include "LoginTest.moc"