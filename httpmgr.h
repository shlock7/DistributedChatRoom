#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include "global.h"
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>

// CRTP
class HttpMgr : public QObject, public Singleton<HttpMgr>
{
public:
    HttpMgr();
};

#endif // HTTPMGR_H
