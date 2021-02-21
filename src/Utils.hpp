/**
 * @file Utils.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Utilities
 * @version 0.1
 * @date 2021-02-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QFile>

namespace MatrixCpp {
/**
 * @brief A file writtable class, in JSON format
 *
 */
class JsonFile {
  public:
    /**
     * @brief Construct a new Json File object
     *
     * @param path Path to the class file
     */
    explicit JsonFile(QString path);

    /**
     * @brief Saves this class to file (encoded by encode())
     *
     */
    virtual void save();

    /**
     * @brief Read class contents in file
     *
     * @return QVariant
     */
    virtual QVariant read();

    QFile file; ///< This file this class is saved to

  protected:
    /**
     * @brief Encode current object to JSON
     *
     * @return QVariant serialized data
     */
    virtual QVariant encode() = 0;
};
} // namespace MatrixCpp