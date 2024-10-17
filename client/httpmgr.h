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
class HttpMgr : public QObject, public Singleton<HttpMgr>, public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();  // 单例类析构的时候会调用T的析构，这里T就是HttpMgr，所以析构函数需要设置为公有类型
    // 发送请求
    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);
private:
    // 将基类声明为友元，才能正确构造派生类对象
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager _manager;

public slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod); // http发送完就发送这个信号，通知其他模块
    //注册模块http相关请求完成发送此信号
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
