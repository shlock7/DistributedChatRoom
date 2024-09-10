#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    /* 邮箱地址的正则表达式
     * (\w+):\w至少匹配一个字符 a-z,A-Z,0-9，+表示至少有一个
     * (\.|_): 可选的.或_
     * (\w*): 零个或多个
     */
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");

    bool match = regex.match(email).hasMatch();
    if (match)   // 邮箱匹配成功
    {
        // 发送http验证码
    }
    else
    {
        showTip(tr("邮箱地址不正确！"), false);
    }
}

void RegisterDialog::showTip(QString str, bool b_ok)
{
    if (b_ok)
    {
        ui->err_tip->setProperty("state", "normal");
    }
    else
    {
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}
