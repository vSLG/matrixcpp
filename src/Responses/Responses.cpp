// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file Responses.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implements various response types
 * @version 0.1
 * @date 2021-01-21
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <MatrixCpp/Responses.hpp>

using namespace MatrixCpp::Responses;
using namespace MatrixCpp::Types;

/*
 * Response
 */

bool Response::isError() const {
    return !this->data.toMap()["errcode"].isNull();
}

void Response::parseData() {
    // Noop
}

/*
 * VersionsResponse
 */

void VersionsResponse::parseData() {
    CHECK_MAP()
    BROKEN(dataMap["versions"].isNull())

    this->versions = dataMap["versions"].value<QStringList>();

    QVariantMap unstableFeaturesMap = dataMap["unstable_features"].toMap();

    if (!unstableFeaturesMap.isEmpty()) {
        QVariantMap::iterator it = unstableFeaturesMap.begin();
        for (; it != unstableFeaturesMap.end(); ++it)
            this->unstableFeatures[it.key()] = it.value().toBool();
    }
}

/*
 * LoginTypesResponse
 */

void LoginTypesResponse::parseData() {
    CHECK_MAP()
    BROKEN(dataMap["flows"].isNull())

    QVariantList flows = dataMap["flows"].toList();

    for (QVariant item : flows) {
        QVariantMap entry = item.toMap();

        if (entry.isEmpty() || entry["type"].isNull())
            continue;

        QMap<QString, QString> newEntry;
        newEntry["type"] = entry["type"].toString();

        this->types.append(entry["type"].toString());
        this->flows.append(newEntry);
    }
}

/*
 * LoginResponse
 */

void LoginResponse::parseData() {
    CHECK_MAP()

    for (QString key : {"user_id", "access_token", "device_id"})
        BROKEN(dataMap[key].isNull())

    this->userId      = dataMap["user_id"].toString();
    this->accessToken = dataMap["access_token"].toString();
    this->deviceId    = dataMap["device_id"].toString();

    if (dataMap["well_known"].isNull())
        return; // End it here

    this->homeserver = dataMap["well_known"]
                           .toMap()["m.homeserver"]
                           .toMap()["base_url"]
                           .toUrl();

    this->identityServer = dataMap["well_known"]
                               .toMap()["m.identity_server"]
                               .toMap()["base_url"]
                               .toUrl();
}

/*
 * WellKnownResponse
 */

void WellKnownResponse::parseData() {
    CHECK_MAP()

    this->homeserver = dataMap["m.homeserver"].toMap()["base_url"].toUrl();
    this->identityServer =
        dataMap["m.identity_server"].toMap()["base_url"].toUrl();
}

/*
 * SyncResponse
 */

void SyncResponse::parseData() {
    CHECK_MAP()
    BROKEN(dataMap["next_batch"].isNull())

    this->nextBatch = dataMap["next_batch"].toString();
    this->rooms     = dataMap["rooms"];

    for (QVariant event : dataMap["presence"].toMap()["events"].toList())
        this->presence.append(event);

    for (QVariant event : dataMap["account_data"].toMap()["events"].toList())
        this->accountData.append(event);

    for (QVariant event : dataMap["to_device"].toMap()["events"].toList())
        this->toDevice.append(event);

    this->deviceLists = dataMap["device_lists"].toMap();
    this->deviceOneTimeKeysCount =
        dataMap["device_one_time_keys_count"].toMap();
}