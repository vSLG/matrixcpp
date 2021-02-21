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

  private:
    OlmAccount *m_account = nullptr;
    QString     m_deviceKeys;
    Client *    m_client = nullptr;
};
} // namespace MatrixCpp::Crypto