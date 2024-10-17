#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "global.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);  // Qt 应用程序的核心对象 只有在实例化了 QApplication 之后，Qt 的窗口部件才能显示。

    QFile qss(":/style/stylesheet.qss");
    if( qss.open(QFile::ReadOnly))
    {
        qDebug("Open qss Success!");
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style); // 将读取的 QSS 样式应用到整个应用程序
        qss.close();
    }
    else
    {
         qDebug("Open qss failed!");
    }

    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    // 拼接文件名
    QString fileName = "config.ini";
    QString config_path = QDir::toNativeSeparators(app_path +
                                                   QDir::separator() + fileName);

    // 以ini格式读取config_path配置文件
    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://"+gate_host+":"+gate_port;

    MainWindow w;
    w.show();
    return a.exec();
}
