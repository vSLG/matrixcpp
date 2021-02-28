// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file SessionStore.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief
 * @version 0.1
 * @date 2021-02-27
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QFile>
#include <QMap>
#include <olm/olm.h>

namespace MatrixCpp::Crypto {
/**
 * @brief Manages storage of OLM sessions
 *
 */
class SessionStore {
  public:
    /**
     * @brief Construct a new Session Store object
     *
     * @param path
     * @param encryptionKey key used to encrypt pickled sessions
     */
    explicit SessionStore(QString path, QString encryptionKey = "");

    ~SessionStore();

    /**
     * @brief Saves currently loaded sessions to file
     *
     */
    void save();

    /**
     * @brief Get sessions for specified device
     *
     * @param deviceKey
     * @return QMap<QString, OlmSession *> A mapping from session ID to
       session
     */
    QMap<QString, OlmSession *> operator[](QString deviceKey);

    QFile file; ///< This is the file the store is saved to

  private:
    QMap<QString, OlmSession *> unpickleAndCache(QString     deviceKey,
                                                 QVariantMap sessions);
    QByteArray                  serializeSessions(QString                     deviceKey,
                                                  QMap<QString, OlmSession *> sessions);

    QMap<QString, QMap<QString, OlmSession *>> m_devices;
    std::string                                m_key;
};
} // namespace MatrixCpp::Crypto