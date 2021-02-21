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