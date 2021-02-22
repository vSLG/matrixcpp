// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file Utils.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Utilities implementation
 * @version 0.1
 * @date 2021-02-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QJsonDocument>
#include <QTemporaryFile>

#include "Utils.hpp"

using namespace MatrixCpp;
using namespace MatrixCpp::Utils;

static bool m_srand = false;

// JsonFile

JsonFile::JsonFile(QString path) {
    this->file.setFileName(path);
}

void JsonFile::save() {
    if (this->file.fileName().isEmpty())
        throw std::runtime_error("Please choose a file name");

    QVariant encoded = this->encode();

    // Do not write if it is null
    if (encoded.isNull())
        return;

    QByteArray  jsonString = QJsonDocument::fromVariant(encoded).toJson();
    std::string errorString =
        ("Failed to write file " + this->file.fileName() + ": ").toStdString();
    QTemporaryFile temp;

    if (!temp.open())
        throw std::runtime_error(errorString +
                                 temp.errorString().toStdString());

    temp.write(jsonString);
    temp.close();

    QFile newFile(this->file.fileName() + ".new");

    // Remove if it exists
    newFile.remove();

    if (!temp.copy(newFile.fileName()))
        throw std::runtime_error(errorString +
                                 temp.errorString().toStdString());

    if (this->file.exists() && !this->file.remove())
        throw std::runtime_error(errorString +
                                 this->file.errorString().toStdString());

    if (!newFile.rename(this->file.fileName()))
        throw std::runtime_error(errorString +
                                 newFile.errorString().toStdString());
}

QVariant JsonFile::read() {
    QJsonDocument   doc;
    QJsonParseError error;
    QByteArray      jsonString;
    std::string     errorString =
        ("Could not read " + this->file.fileName() + ": ").toStdString();

    if (!this->file.open(QFile::ReadOnly))
        throw std::runtime_error(errorString +
                                 this->file.errorString().toStdString());

    jsonString = this->file.readAll();
    this->file.close();

    doc = QJsonDocument::fromJson(jsonString, &error);

    if (error.error)
        throw std::runtime_error(errorString +
                                 error.errorString().toStdString());

    return doc.toVariant();
}

// Namespace Utils

QByteArray Utils::canonicalJson(QVariantMap json) {
    return QJsonDocument::fromVariant(json).toJson(QJsonDocument::Compact);
}

uint8_t *Utils::randomBytes(size_t len) {
    uint8_t *random = (uint8_t *) malloc(len);

    if (!random)
        throw std::runtime_error("Failed to allocate " + std::to_string(len) +
                                 "bytes");

    int i         = 0;
    int randSize  = sizeof(rand());
    int totalInts = len / randSize;
    int remainder = len % randSize;

    if (!m_srand) {
        srand(time(NULL));
        m_srand = true;
    }

    for (i = 0; i < totalInts; i++)
        random[i * randSize] = rand();

    random[i] = rand() >> (remainder * 8);

    return random;
}