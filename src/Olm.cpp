/**
 * @file Olm.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implements Olm
 * @version 0.1
 * @date 2021-02-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#include "Olm.hpp"
#include <cstring>
#include <olm/olm.h>
#include <stdexcept>

using namespace MatrixCpp;
using namespace MatrixCpp::Crypto;

Olm::Olm(Client *client)
    : QObject(client),
      JsonFile(client->storeDir.filePath(
          QUrl::toPercentEncoding(client->userId() + ".json"))),
      m_client(client) {
    this->m_account = olm_account(malloc(olm_account_size()));

    connect(this, &Olm::olmError, [=](QString error) {
        qCritical() << error.toStdString().c_str();
    });

    if (this->file.exists())
        this->load();
    else {
        this->create();
        this->save();
    }
}

Olm::~Olm() {
    olm_clear_account(this->m_account);
}

QVariant Olm::encode() {
    QVariantHash json;

    int         pickledLenght = olm_pickle_account_length(this->m_account);
    char *      pickled       = (char *) malloc(pickledLenght);
    std::string key           = this->m_client->accessToken().toStdString();

    if (olm_pickle_account(this->m_account,
                           key.c_str(),
                           key.length(),
                           pickled,
                           pickledLenght) == olm_error()) {
        emit this->olmError("Failed to pikcle Olm account: " +
                            QString(olm_account_last_error(this->m_account)));
        return QVariant();
    }

    json["olm"] = pickled;

    free(pickled);

    return json;
}

void Olm::create() {
    qDebug() << "OLM Creating account for" << this->m_client->userId();

    int            randSize    = sizeof(rand());
    unsigned char *randomBytes = (unsigned char *) malloc(randSize * 64);

    srand(time(NULL));

    for (int i = 0; i < 64; i++)
        randomBytes[i * randSize] = rand();

    int ret = olm_create_account(this->m_account, randomBytes, randSize * 64);

    free(randomBytes);

    if (ret == olm_error())
        emit this->olmError("OLM Could not create account: " +
                            QString(olm_account_last_error(this->m_account)));
}

void Olm::load() {
    qDebug() << "OLM Loading account for" << this->m_client->userId()
             << "from store";

    QVariantHash json       = this->read().toHash();
    std::string  key        = this->m_client->accessToken().toStdString();
    std::string  pickledStr = json["olm"].toString().toStdString();

    // Because olm_unpickle_account destroys pickled buffer, we need to manually
    // allocate the buffer. We cannot modify std::string internals.
    char *pickled = (char *) malloc(pickledStr.length());
    memcpy(pickled, pickledStr.c_str(), pickledStr.length());

    if (olm_unpickle_account(this->m_account,
                             key.c_str(),
                             key.length(),
                             pickled,
                             pickledStr.length()) == olm_error())
        emit this->olmError(QString("OLM Could not load account from disk: %1")
                                .arg(olm_account_last_error(this->m_account)));
}

QString Olm::deviceKeys() {
    if (!this->m_deviceKeys.isEmpty())
        return this->m_deviceKeys;

    int   keysSize = olm_account_identity_keys_length(this->m_account);
    char *keys     = (char *) malloc(keysSize);

    if (olm_account_identity_keys(this->m_account, keys, keysSize) ==
        olm_error()) {
        this->m_deviceKeys = "";
        emit this->olmError(QString("OLM Could not get identity keys: %1")
                                .arg(olm_account_last_error(this->m_account)));
    } else
        this->m_deviceKeys = keys;

    free(keys);
    return this->m_deviceKeys;
}