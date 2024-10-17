#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    setCentralWidget(_login_dlg); // 将登录对话框设置为主窗口的中心组件。这样主窗口会显示登录对话框，且不会有其他控件与其并存
    // _login_dlg->show();

    // 创建和注册连接
    // _login_dlg 发出 switchRegister信号时，MainWindow进行捕获并调用SlotSwitchReg
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    _reg_dlg = new RegisterDialog(this);

    // Qt::CustomizeWindowHint：允许自定义窗口
    // Qt::FramelessWindowHint：使窗口没有标准的标题栏和边框
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _reg_dlg->hide();  // 隐藏注册对话框

}

MainWindow::~MainWindow()
{
    delete ui;
//    if (_login_dlg)
//    {
//        delete _login_dlg;
//        _login_dlg = nullptr;
//    }

//    if (_reg_dlg)
//    {
//        delete _reg_dlg;
//        _reg_dlg = nullptr;
//    }
}

void MainWindow::SlotSwitchReg()
{
    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
}

