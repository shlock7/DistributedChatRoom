#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "resetdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg); // 将登录对话框设置为主窗口的中心组件。这样主窗口会显示登录对话框，且不会有其他控件与其并存
    // _login_dlg->show();

    // 创建和注册连接
    // _login_dlg 发出 switchRegister信号时，MainWindow进行捕获并调用SlotSwitchReg
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);

    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);

#if 0
    _reg_dlg = new RegisterDialog(this);

    // Qt::CustomizeWindowHint：允许自定义窗口
    // Qt::FramelessWindowHint：使窗口没有标准的标题栏和边框
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _reg_dlg->hide();  // 隐藏注册对话框
#endif

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
    // 点击注册才会跳转到注册页面，所以可以在这里动态的初始化
    // 好处就是从注册界面切换到其他界面会自动回收
    _reg_dlg = new RegisterDialog(this);

    // Qt::CustomizeWindowHint：允许自定义窗口
    // Qt::FramelessWindowHint：使窗口没有标准的标题栏和边框
    _reg_dlg->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    // _reg_dlg->hide();  // 隐藏注册对话框

    //连接注册界面返回登录信号
    connect(_reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);

    setCentralWidget(_reg_dlg);
    _login_dlg->hide();
    _reg_dlg->show();
}

// 注册切换到登录界面
void MainWindow::SlotSwitchLogin()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reg_dlg->hide();
    _login_dlg->show();
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //连接登录界面忘记密码信号
    //connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _reset_dlg = new ResetDialog(this);
    _reset_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_reset_dlg);

    _login_dlg->hide();  // 这里会回收这个界面
    _reset_dlg->show();
    //注册返回登录信号和槽函数
    connect(_reset_dlg, &ResetDialog::switchLogin, this, &MainWindow::SlotSwitchLogin2);
}

// 从重置界面返回登录界面
void MainWindow::SlotSwitchLogin2()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    _login_dlg = new LoginDialog(this);
    _login_dlg->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(_login_dlg);

    _reset_dlg->hide();
    _login_dlg->show();
    // 因为登录界面是新建的，所以需要再连接一遍信号
    //连接登录界面忘记密码信号
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    //连接登录界面注册信号
    connect(_login_dlg, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
}

