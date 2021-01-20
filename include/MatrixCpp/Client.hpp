/**
 * @file Client.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Declares Client class
 * @version 0.1
 * @date 2021-01-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QObject>
#include <QUrl>
#include <qobject.h>

#include "export.hpp"

namespace MatrixCpp {
class PUBLIC Client : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new Client object
     *
     * @param homeserver Homeserver URL for this client
     * @param parent QObject parent, if any
     */
    explicit Client(const QUrl &homeserverUrl, QObject *parent = nullptr);

    ~Client();

    // Enums and structs

    /**
     * @brief Available presence states for a client
     *
     */
    enum Presence { PRESENCE_ONLINE, PRESENCE_BUSY, PRESENCE_OFFLINE };

    // Public variables

    QUrl homeserverUrl;

    static const inline QString apiEndpoints[] = {"/_matrix/client/versions"};

  signals:
    void abortRequests();

  private:
    // To avoid public header pollution, use plmpl idiom
    // (https://en.wikipedia.org/wiki/Opaque_pointer)

    /**
     * @brief Stores MatrixCpp private functions and variables
     *
     */
    struct Privates;
    Privates *m_private;
};
} // namespace MatrixCpp