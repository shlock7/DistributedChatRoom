#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include <QRegularExpression>
#include <memory>
#include <iostream>
#include <mutex>
#include "QStyle"

/*
 * 刷新qss
 *
 */
extern std::function<void(QWidget*)> repolish;

#endif // GLOBAL_H
