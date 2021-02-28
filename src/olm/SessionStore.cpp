// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file SessionStore.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implements SessionStore
 * @version 0.1
 * @date 2021-02-28
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QJsonDocument>
#include <olm/olm.h>

#include "SessionStore.hpp"
#include "src/Utils.hpp"

using namespace MatrixCpp::Crypto;

SessionStore::SessionStore(QString path, OlmAccount *olm, QString key)
    : key(key.toStdString()), olm(olm) {
    this->file.setFileName(path);
}

SessionStore::SessionStore() {
}

SessionStore::~SessionStore() {
    for (auto sessions : this->m_devices)
        for (OlmSession *session : sessions)
            olm_clear_session(session);
}

QMap<QString, OlmSession *> SessionStore::operator[](QString deviceKey) {
    if (this->file.fileName().isEmpty())
        throw std::runtime_error("SESSION please set a file name");

    // Try to find cached
    if (!this->m_devices[deviceKey].isEmpty())
        return this->m_devices[deviceKey];

    // Else try to load from file
    if (!this->file.open(QFile::ReadOnly)) {
        if (!this->file.exists()) {
            // We cannot throw error when file does not exist, so cache empty
            // map and return it
            this->m_devices[deviceKey] = {};
            return {};
        } else
            throw std::runtime_error("SESSION could not open store file: " +
                                     this->file.errorString().toStdString());
    }

    int        lineNumber = 0;
    QByteArray line;

    while ((line = this->file.readLine()) != "") {
        lineNumber++;

        QJsonParseError error;
        QVariantMap     parsed =
            QJsonDocument::fromJson(line, &error).toVariant().toMap();

        if (error.error != QJsonParseError::NoError) {
            qCritical() << "SESSION failed to parse line" << lineNumber << "("
                        << error.errorString() << ")";
            continue;
        }

        if (!parsed[deviceKey].isNull()) {
            // Then we found the stored session. Unpickle it, cache and
            // return
            this->file.close();
            return this->unpickleAndCache(deviceKey, parsed[deviceKey].toMap());
        }
    }
    this->file.close();

    // Then we have not found any sessions. Return empty map
    // Also cache empty map so we do not read the file again
    this->m_devices[deviceKey] = {};
    return {};
}

void SessionStore::save() {
    if (this->file.fileName().isEmpty())
        throw std::runtime_error("SESSION please set a file name");

    qDebug() << "SESSION saving";

    QFile newFile(this->file.fileName() + ".new");

    if (!(this->file.open(QFile::ReadOnly) && newFile.open(QFile::WriteOnly))) {
        this->file.close(); // Can be opened, so close just to make sure
        throw std::runtime_error("SESSION could not open store file");
    }

    int        lineNumber = 0;
    QByteArray line;

    while ((line = this->file.readLine()) != "") {
        lineNumber++;

        QJsonParseError error;
        QVariantMap     parsed =
            QJsonDocument::fromJson(line, &error).toVariant().toMap();

        if (error.error != QJsonParseError::NoError) {
            qCritical() << "SESSION failed to parse line" << lineNumber << "("
                        << error.errorString() << ")";
            continue;
        }

        QString deviceKey(parsed.keys().first());

        if (this->m_devices.contains(deviceKey) &&
            !this->m_devices[deviceKey].isEmpty())
            // Then update record
            line =
                this->serializeSessions(deviceKey, this->m_devices[deviceKey]) +
                "\n";

        if (newFile.write(line) < 0) {
            this->file.close();
            newFile.close();
            throw std::runtime_error("SESSION failed to update store file");
        }
    }
    this->file.close();
    newFile.close();

    if (!this->file.remove())
        throw std::runtime_error("SESSION could not update store file");

    newFile.rename(this->file.fileName());
}

OlmSession *SessionStore::createInbound(QString message, QString deviceKey) {
    if (this->file.fileName().isEmpty() || this->olm == nullptr)
        throw std::runtime_error(
            "SESSION please set a file name and/or an olm account");

    std::string stdMessage(message.toStdString());
    std::string stdDeviceKey(deviceKey.toStdString());
    OlmSession *session = olm_session(malloc(olm_session_size()));

    // olm_create_inbound_session_from *might* destoy message buffer.
    // Documentation is obscure about this, so just in case allocate memory
    char *msg = (char *) malloc(stdMessage.size());
    memcpy(msg, stdMessage.c_str(), stdMessage.length());

    if (olm_create_inbound_session_from(session,
                                        this->olm,
                                        stdDeviceKey.c_str(),
                                        stdDeviceKey.length(),
                                        msg,
                                        stdMessage.length()) == olm_error())
        throw std::runtime_error(
            "SESSION could not create inbound session for " + stdDeviceKey +
            " (" + olm_session_last_error(session) + ")");

    if (msg)
        free(msg);

    int   idSize    = olm_session_id_length(session);
    char *sessionId = (char *) malloc(idSize);

    if (olm_session_id(session, sessionId, idSize) == olm_error()) {
        throw std::runtime_error("SESSION failed to get session ID for " +
                                 stdDeviceKey + " (" +
                                 olm_session_last_error(session) + ")");
    }

    QString id = sessionId;
    free(sessionId);

    // Load and store session
    (*this)[deviceKey][id] = session;
    this->save();
    return session;
}

QMap<QString, OlmSession *>
SessionStore::unpickleAndCache(QString deviceKey, QVariantMap pickledSessions) {
    QMap<QString, OlmSession *> sessions;
    QVariantMap::const_iterator it = pickledSessions.constBegin();

    for (; it != pickledSessions.constEnd(); ++it) {
        OlmSession *session    = olm_session(malloc(olm_session_size()));
        std::string pickledStr = it.value().toString().toStdString();

        // Because olm_unpickle_session destroys pickled buffer, we need to
        // manually allocate the buffer. We cannot modify std::string
        // internals.
        char *pickled = (char *) malloc(pickledStr.length());
        memcpy(pickled, pickledStr.c_str(), pickledStr.length());

        if (olm_unpickle_session(session,
                                 this->key.c_str(),
                                 this->key.length(),
                                 pickled,
                                 pickledStr.length()) == olm_error()) {
            qCritical() << "SESSION failed to unpickle session for" << deviceKey
                        << "(" << olm_session_last_error(session) << ")";
            this->m_devices[deviceKey] = {};
            return {};
        }

        int   idSize    = olm_session_id_length(session);
        char *sessionId = (char *) malloc(idSize);

        if (olm_session_id(session, sessionId, idSize) == olm_error()) {
            qCritical() << "SESSION failed to get session ID for" << deviceKey
                        << "(" << olm_session_last_error(session) << ")";
            this->m_devices[deviceKey] = {};
            return {};
        }

        QString id = sessionId;
        free(sessionId);

        // Check if stored ID matches the calculated ID
        if (it.key() != id) {
            qWarning() << "SESSION stored ID does not match calculated, "
                          "replacing for"
                       << deviceKey;
        }

        sessions[id] = session;
    }

    this->m_devices[deviceKey] = sessions;
    return sessions;
}

QByteArray
SessionStore::serializeSessions(QString                     deviceKey,
                                QMap<QString, OlmSession *> sessions) {
    QVariantMap record;
    QVariantMap pickledSessions;

    auto sessionsIt = sessions.constBegin();
    for (; sessionsIt != sessions.constEnd(); ++sessionsIt) {
        OlmSession *session = sessionsIt.value();

        int   pickledSize = olm_pickle_session_length(session);
        char *pickled     = (char *) malloc(pickledSize);

        if (olm_pickle_session(session,
                               this->key.c_str(),
                               this->key.length(),
                               pickled,
                               pickledSize) == olm_error())
            throw std::runtime_error("SESSION could not pickle session for " +
                                     deviceKey.toStdString());

        pickledSessions[sessionsIt.key()] = pickled;
        free(pickled);
    }

    record[deviceKey] = pickledSessions;
    return Utils::canonicalJson(record);
}