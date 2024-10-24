#include "AsioIOServicePool.h"
#include <iostream>
using namespace std;

// io_context 没有拷贝，一定要在初始化列表中初始化
AsioIOServicePool::AsioIOServicePool(std::size_t size) :_ioServices(size),
                                                    _works(size), _nextIOService(0)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        // 为每个 io_context 对象创建一个 Work 对象 unique_ptr绑定裸指针
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
    }

    //遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
    for (std::size_t i = 0; i < _ioServices.size(); ++i)
    {
        _threads.emplace_back([this, i](){ _ioServices[i].run(); });
    }
}

AsioIOServicePool::~AsioIOServicePool()
{
    Stop();
    std::cout << "AsioIOServicePool destruct" << endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
    // 按照 轮询（round-robin） 的方式分配任务
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size())
    {
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop()
{
    //因为仅仅执行work.reset并不能让iocontext从run的状态中退出
    //当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
    for (auto& work : _works)
    {
        //把服务先停止
        work->get_io_context().stop();
        work.reset();
    }

    // 等待每个线程执行完毕，确保所有线程安全退出
    for (auto& t : _threads)
    {
        t.join();
    }
}
