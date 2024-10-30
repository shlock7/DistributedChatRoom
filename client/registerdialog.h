#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
    void solt_reg_mod_finish(ReqId id, QString res, ErrorCodes err);

    void on_sure_btn_clicked();

    void on_pushButton_clicked();

private:
    QMap<TipErr, QString> _tip_errs; // 存储错误码
    void AddTipErr(TipErr te,QString tips);
    void DelTipErr(TipErr te);
    bool checkUserValid();      // 检查输入的用户名是否合法
    bool checkEmailValid();     // 检查输入的邮箱是否合法
    bool checkPassValid();      // 检查输入的密码是否合法
    bool checkVarifyValid();    // 检查输入的确认密码是否合法
    bool checkConfirmValid();   // 检查输入的验证码是否合法
    void showTip(QString str, bool b_ok);  // 邮箱错误提示

    Ui::RegisterDialog *ui;
    void initHttpHandlers();
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;

    QTimer * _countdown_timer;  // 注册成功界面的定时器
    int _countdown;             // 注册成功界面倒计时的数字
    void ChangeTipPage();       //

signals:
    void sigSwitchLogin();

};

#endif // REGISTERDIALOG_H
