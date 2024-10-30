#include "timerbtn.h"
#include <QMouseEvent>
#include <QDebug>

TimerBtn::TimerBtn(QWidget *parent) : QPushButton(parent), _counter(10)
{
    // 父指针是this TimerBtn被回收，timer也会被回收
    _timer = new QTimer(this);

    // 定时器到超时时间会触发这个回调
    connect(_timer, &QTimer::timeout, [this](){
        _counter--;         // 定时器开始递减
        if(_counter <= 0)   // 减到0就重置
        {
            _timer->stop();
            _counter = 10;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });
}

TimerBtn::~TimerBtn()
{
    _timer->stop();
}

// 鼠标释放事件
void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) // 判断点击的是鼠标左键
    {
        // 在这里处理鼠标左键释放事件
        qDebug() << "MyButton was released!";
        this->setEnabled(false);
        this->setText(QString::number(_counter));
        _timer->start(1000);    // 每隔一秒触发一次
        emit clicked();         // 发出点击的信号
    }
    // 调用基类的mouseReleaseEvent以确保正常的事件处理（如点击效果）
    QPushButton::mouseReleaseEvent(e);
}
