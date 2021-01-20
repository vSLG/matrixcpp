#pragma once

#include <QNetworkAccessManager>
#include <QObject>

#include "export.hpp"

namespace MatrixCpp {
class PUBLIC Client {
    Q_OBJECT

  private:
    QNetworkAccessManager *m_nam;
};
} // namespace MatrixCpp