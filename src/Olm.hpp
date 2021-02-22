// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file Olm.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Olm related classes and structs declarations
 * @version 0.1
 * @date 2021-02-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QDir>
#include <olm/olm.h>

#include <MatrixCpp/Client.hpp>

#include "Utils.hpp"

#define olm_check_error(function, msg) \
    if (function == olm_error())       \
        throw std::runtime_error(      \
            "OLM " msg ": " +          \
            std::string(olm_account_last_error(this->m_account)));

namespace MatrixCpp::Crypto {
/**
 * @brief An abstraction to OlmAccount operations
 *
 * This class offers:
 *   - Saving OlmAccount to a JSON file
 *   - Encrypting/decrypting events
 */
class Olm : public QObject, public JsonFile {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new Olm object
     *
     * @param client
     */
    explicit Olm(Client *client);

    /**
     * @brief Destroy the Olm object
     *
     */
    ~Olm();

    /**
     * @brief Create or get deivce keys
     *
     * @return QString JSON-formatted device keys
     */
    QString deviceKeys();

    /**
     * @brief Send identity and one time keys to the server, as needed
     *
     * @return Responses::ResponseFuture*
     */
    Responses::ResponseFuture *sendKeys();

    /**
     * @brief Max one time keys this account can handle
     *
     * @return int
     */
    int maxOneTimeKeys();

  signals:
    void olmError(QString error);

  protected:
    QVariant encode() override;

    /**
     * @brief Creates an Olm account related to this user
     *
     */
    void create();

    /**
     * @brief Loads Olm account from disk
     *
     */
    void load();

    /**
     * @brief Signs a message with the ed25519 key for this account
     *
     * @param message
     * @return QString The signature
     */
    QString sign(QString message);

    bool deviceKeysUploaded = false; ///< Whether keys have been uploaded or not
    int  uploadedOneTimeKeys =
        -1; ///< Total uploaded one time keys we have track of

  private:
    QVariantMap serializeDeviceKeys();
    QVariantMap serializeOneTimeKeys(int count);
    int         oneTimeKeysToUploadCount();

    OlmAccount *m_account = nullptr;
    QString     m_deviceKeys;
    Client *    m_client         = nullptr;
    int         m_maxOneTimeKeys = -1;
};
} // namespace MatrixCpp::Crypto