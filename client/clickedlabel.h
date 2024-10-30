#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H
#include <QLabel>
#include "global.h"

class ClickedLabel:public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget* parent);
    virtual void mousePressEvent(QMouseEvent *ev) override; // 点击
    virtual void enterEvent(QEvent* event) override;        // 进入
    virtual void leaveEvent(QEvent* event) override;        // 离开
    // normal 为闭眼状态，select 睁眼，
    void SetState(QString normal="", QString hover="", QString press="",
                  QString select="", QString select_hover="", QString select_press="");

    // 返回此时label的状态，控制是否显示密码
    ClickLbState GetCurState();

protected:

private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curstate;
signals:
    void clicked(void);  //发一个clicked信号

};

#endif // CLICKEDLABEL_H
