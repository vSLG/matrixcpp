// SPDX-License-Identifier: LGPL-3.0-or-later

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

#include <QJsonDocument>
#include <olm/olm.h>
#include <qjsondocument.h>

#include "MatrixCpp/Responses.hpp"
#include "Olm.hpp"
#include "src/Utils.hpp"

using namespace MatrixCpp;
using namespace MatrixCpp::Crypto;
using namespace MatrixCpp::Responses;
using namespace MatrixCpp::Utils;

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

    json["device_keys_uploaded"] = this->deviceKeysUploaded;

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

    int      randomSize  = olm_create_account_random_length(this->m_account);
    uint8_t *randomBytes = Utils::randomBytes(randomSize);
    int      ret = olm_create_account(this->m_account, randomBytes, randomSize);

    free(randomBytes);

    if (ret == olm_error())
        emit this->olmError("OLM Could not create account: " +
                            QString(olm_account_last_error(this->m_account)));
}

void Olm::load() {
    qDebug() << "OLM Loading account for" << this->m_client->userId()
             << "from store";

    QVariantMap json = this->read().toMap();

    this->deviceKeysUploaded = json["device_keys_uploaded"].toBool();

    std::string key        = this->m_client->accessToken().toStdString();
    std::string pickledStr = json["olm"].toString().toStdString();

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

QString Olm::sign(QString message) {
    int         signSize = olm_account_signature_length(this->m_account);
    char *      sign     = (char *) malloc(signSize);
    std::string msg      = message.toStdString();

    if (olm_account_sign(
            this->m_account, msg.c_str(), msg.length(), sign, signSize) ==
        olm_error()) {
        emit this->olmError(QString("OLM Could not get identity keys: %1")
                                .arg(olm_account_last_error(this->m_account)));
        return "";
    }

    QString signature = sign;
    free(sign);
    return signature;
}

ResponseFuture *Olm::sendKeys() {
    QVariantMap data;
    bool        uploadingDeviceKeys = false;

    if (!this->deviceKeysUploaded) {
        data["device_keys"] = this->serializeDeviceKeys();
        qDebug("OLM uploading device keys for the first time");
        uploadingDeviceKeys = true;
    } else if (this->oneTimeKeysToUploadCount() > 0)
        data["one_time_keys"] =
            this->serializeOneTimeKeys(this->oneTimeKeysToUploadCount());
    else
        throw std::runtime_error(
            "Trying to upload keys when there is none to upload");

    ResponseFuture *future =
        this->m_client->send("/_matrix/client/r0/keys/upload", data);

    connect(future, &ResponseFuture::responseComplete, [=](Response response) {
        qDebug() << response.getJson().toStdString().c_str();

        if (response.isError() || response.isBroken())
            return;

        if (uploadingDeviceKeys)
            // Fisrt time sending device keys
            this->deviceKeysUploaded = true;
        else
            // Else we are uploading one time keys. Mark them as published,
            // because upload was successful
            olm_account_mark_keys_as_published(this->m_account);

        this->save();
    });

    return future;
}

int Olm::maxOneTimeKeys() {
    if (this->m_maxOneTimeKeys > 0)
        return this->m_maxOneTimeKeys;

    this->m_maxOneTimeKeys =
        olm_account_max_number_of_one_time_keys(this->m_account);

    return this->m_maxOneTimeKeys;
}

QVariantMap Olm::serializeDeviceKeys() {
    QVariantMap deviceKeys, keys, signatures, selfSignature;
    QString     keysStr = this->deviceKeys();

    QString userId   = this->m_client->userId();
    QString deviceId = this->m_client->deviceId;

    keys = QJsonDocument::fromJson(keysStr.toUtf8()).toVariant().toMap();
    keys["curve25519:" + deviceId] = keys.take("curve25519");
    keys["ed25519:" + deviceId]    = keys.take("ed25519");

    deviceKeys["user_id"]   = userId;
    deviceKeys["device_id"] = deviceId;
    deviceKeys["algorithms"] =
        QStringList({"m.olm.curve25519-aes-sha256", "m.megolm.v1.aes-sha"});
    deviceKeys["keys"] = keys;

    selfSignature["ed25519:" + deviceId] =
        this->sign(canonicalJson(deviceKeys));

    signatures[userId]       = selfSignature;
    deviceKeys["signatures"] = signatures;

    return deviceKeys;
}

QVariantMap Olm::serializeOneTimeKeys(int count) {
    assert(count > 0);

    int randomSize = olm_account_generate_one_time_keys_random_length(
        this->m_account, count);
    uint8_t *randomBytes = Utils::randomBytes(randomSize);

    olm_check_error(olm_account_generate_one_time_keys(
                        this->m_account, count, randomBytes, randomSize),
                    "could not generate one time keys");

    free(randomBytes);

    int   keysSize = olm_account_one_time_keys_length(this->m_account);
    char *keysStr  = (char *) malloc(keysSize);

    olm_check_error(
        olm_account_one_time_keys(this->m_account, keysStr, keysSize),
        "could not get the one time keys");

    QVariantMap keys = QJsonDocument::fromJson(keysStr)
                           .toVariant()
                           .toMap()["curve25519"]
                           .toMap();

    free(keysStr);

    QVariantMap data;

    QVariantMap::const_iterator it = keys.constBegin();
    for (; it != keys.constEnd(); ++it) {
        QVariantMap key, signatures, selfSignature;
        key["key"] = it.value();

        selfSignature["ed25519:" + this->m_client->deviceId] =
            this->sign(canonicalJson(key));

        signatures[this->m_client->userId()]  = selfSignature;
        key["signatures"]                     = signatures;
        data["signed_curve25519:" + it.key()] = key;
    }

    qDebug() << QJsonDocument::fromVariant(data).toJson().toStdString().c_str();

    return data;
}

int Olm::oneTimeKeysToUploadCount() {
    if (this->uploadedOneTimeKeys < 0)
        return -1;

    return this->maxOneTimeKeys() / 2 - this->uploadedOneTimeKeys;
}