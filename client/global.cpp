#include "global.h"

QString gate_url_prefix = "";

// 刷新QWidget的样式
std::function<void(QWidget*)> repolish = [](QWidget* w)
{
    w->style()->unpolish(w);  // 去掉QWidget的旧样式
    w->style()->polish(w);    // 应用新样式
};
