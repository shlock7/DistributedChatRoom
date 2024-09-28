#ifndef SINGLETON_H
#define SINGLETON_H

#include "global.h"

template <typename T>
class Singleton
{
protected:
    Singleton() = default;
    // 单例类禁用拷贝构造和拷贝赋值
    Singleton(const Singleton<T>&) = delete;
    Singleton &operator = (const Singleton<T>& st) = delete;

    static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag s_flag;  // 如果被多次调用，只会被初始化一次
        // 下面这个函数第一次被调用时，s_flag内部标记为 false，被调用一次之后就是true
        // 第二次调用时，这个函数就不会执行了
        std::call_once(s_flag, [&]()
        {
            // 思考一下为什么不用make_shared()
            _instance = std::shared_ptr<T>(new T);
        });
        return _instance;
    }

    void PrintAddress()
    {
        std::cout << _instance.get() << std::endl;
    }

    ~Singleton()
    {
        std::cout << "singleton destruct" << std::endl;
    }
};

/* 防止编译能过，链接出错
 * 问题出在instance是个静态变量，他一定要被初始化，非模板类我们都把他放在cpp里
 * 如果是模板类，我们就在头文件中初始化成空指针
 */
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;


#endif // SINGLETON_H
