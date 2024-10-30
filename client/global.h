#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include <QRegularExpression>
#include <memory>
#include <iostream>
#include <mutex>
#include <QByteArray>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QSettings>
#include "QStyle"

/*
 * 刷新qss
 */
extern std::function<void(QWidget*)> repolish;
\
// 异或字符串，用于加密
extern std::function<QString(QString)> xorString;

enum ReqId
{
    ID_GET_VARIFY_CODE = 1001,  // 获取验证码
    ID_REG_USER = 1002,         // 注册用户
};

enum Modules
{
    REGISTERMOD = 0,
};


enum ErrorCodes
{
    SUCCESS = 0,
    ERR_JSON = 1,       // json解析失败
    ERR_NETWORK = 2,    // 网络错误
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

// 显示密码按钮的状态
enum ClickLbState
{
    Normal = 0,
    Selected = 1
};

extern QString gate_url_prefix;

#endif // GLOBAL_H
