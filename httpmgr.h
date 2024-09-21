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

// CRTP, T实例化为派生类
class HttpMgr : public QObject, public Singleton<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();
private:
    // 将基类声明为友元，才能正确构造派生类对象
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager _manager;

signals:
    void sig_http_finish(); // http发送完就发送这个信号，通知其他模块
};

#endif // HTTPMGR_H
