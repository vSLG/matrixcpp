// SPDX-License-Identifier: LGPL-3.0-or-later

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

#include <QDir>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QUrlQuery>

#include <MatrixCpp/Responses.hpp>
#include <MatrixCpp/Types.hpp>
#include <MatrixCpp/export.hpp>

namespace MatrixCpp {
// Fast forward private types
namespace Crypto {
class Olm;
}

/**
 * @brief A Matrix Client
 *
 */
class PUBLIC Client : public QObject {
    Q_OBJECT

  public:
    //**
    // * @brief Construct a new Client object
    // *
    // * @param host Homeserver host
    // * @param user Fully qualified or local user_id for this Client
    // * @param deviceId device_id to use with this Client
    // * @param parent QObject parent, if any
    // */
    /* explicit Client(const QString &host,
                    const QString &user     = "",
                    const QString &deviceId = "",
                    QObject *      parent   = nullptr); */

    /**
     * @brief Construct a new Client object
     *
     * @param homeserverUrl Homeserver URL for this client
     * @param encryption Whether to enable or not encryption for this client
     * @param parent QObject parent, if any
     */
    explicit Client(const QUrl &homeserverUrl,
                    bool        encryption = true,
                    QObject *   parent     = nullptr);

    /**
     * @brief (sync) Request for well_known and update the client
     *
     */
    void loadDiscovery();

    /**
     * @brief Load given info to the Client
     *
     * @param userId *Fully qualified* user id
     * @param deviceId A valid and existing device id
     * @param accessToken The access token for this account
     */
    void restore(const QString &userId,
                 const QString &deviceId,
                 const QString &accessToken);

    // Enums and structs

    /**
     * @brief Available presence states for a client
     *
     */
    enum Presence { PRESENCE_ONLINE, PRESENCE_UNAVAILABLE, PRESENCE_OFFLINE };

    // API calls

    /**
     * @brief (async) Get server discovery information. This will also update
       the client
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *getDiscovery();

    /**
     * @brief (async) Get the Server Version and unstable features
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *getServerVersion() const;

    /**
     * @brief (async) Get server supported login types
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *getLoginTypes() const;

    /**
     * @brief (async) Logs in to the specified account with password OR
     * token
     *
     * @param userId Local or fully qualified user ID
     * @param password Password for the user
     * @param token Token for the user
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *
    login(QString userId = "", QString password = "", QString token = "");

    /**
     * @brief (async) Performs a sync request. This will also update the Client
     *
     * @param filter Filter id or plain JSON for the filter
     * @param since A point in time to continue a sync from
     * @param fullState Controls whether to include the full state for all rooms
       the user is a member of
     * @param presence Desired presence to set on sync request
     * @param timeout The maximum time to wait, in milliseconds, before
       returning this request
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *sync(const QString &filter    = "",
                                    const QString &since     = "",
                                    bool           fullState = false,
                                    Presence       presence  = PRESENCE_ONLINE,
                                    int            timeout   = 0);

    /**
     * @brief HTTP get request to specified path on homeserver
     *
     * @param path
     * @param query Query data for request
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *get(QString   path,
                                   QUrlQuery query = QUrlQuery()) const;

    /**
     * @brief Sends POST JSON to specified path
     *
     * @param path
     * @param data The data to be sent. Will be JSON encoded
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *send(QString path, QVariantMap data) const;

    // Getters & setters

    /**
     * @brief Gets the user_id
     *
     * @return QString
     */
    QString userId() const;

    /**
     * @brief Gets the access_token
     *
     * @return QString
     */
    QString accessToken() const;

    // Public variables

    QUrl homeserverUrl; ///< Current homeserver URL this Client is associated
    QMap<QString, Types::Room *> rooms;    ///< Rooms this Client is associated
    QString                      deviceId; ///< Device ID
    QDir storeDir; ///< Store directory for encryption keys

  signals:
    /**
     * @brief When fired, will stop all ongoing requests
     *
     */
    void abortRequests();

  protected slots:
    /**
     * @brief Sets Client properties properly from login response
     *
     * @param response
     */
    void onLoginResponse(Responses::LoginResponse response);

    /**
     * @brief Sets Client properties properly from well known response
     *
     * @param response
     */
    void onDiscoveryResponse(Responses::WellKnownResponse response);

    /**
     * @brief Sets Client properties properly from sync response
     *
     * @param response
     */
    void onSyncResponse(Responses::SyncResponse response);

    /**
     * @brief Updates our rooms based on roomUpdate
     *
     * @param roomsUpdates
     */
    void onRoomJoinUpdate(const QMap<QString, Types::RoomUpdate> &roomsUpdates);

  private:
    /**
     * @brief HTTP get request to specified URL
     *
     * @param url
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *get(QUrl url) const;

    /**
     * @brief Sends POST JSON to specified URL
     *
     * @param url
     * @param data The data to be sent. Will be JSON encoded
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture *send(QUrl url, QVariantMap data) const;

    QNetworkAccessManager *m_nam;

    QString      m_userId;
    QString      m_accessToken;
    QString      m_nextBatch;
    bool         m_encryption;
    Crypto::Olm *m_olm = nullptr;
};
} // namespace MatrixCpp