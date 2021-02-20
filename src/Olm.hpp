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

namespace MatrixCpp::Crypto {
/**
 * @brief An abstraction to OlmAccount operations
 *
 * This class offers:
 *   - Saving OlmAccount to a JSON file
 *   - Encrypting/decrypting events
 */
class Olm : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new Olm object
     *
     * @param client
     * @param storeDir Directory for storing keys
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
     * @brief Save OlmAccount in the store dir
     *
     */
    void save();

  signals:
    void olmError(QString error);

  private:
    OlmAccount *m_account = nullptr;
    QString     m_deviceKeys;
};
} // namespace MatrixCpp::Crypto