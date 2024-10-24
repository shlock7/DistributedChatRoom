#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work; // 用于确保 io_context 对象不会停止运行
    using WorkPtr = std::unique_ptr<Work>;      // 用于自动管理 Work 对象的生命周期
    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
    // 使用 round-robin 的方式返回一个 io_service
    boost::asio::io_context& GetIOService();  
    void Stop(); // 停止所有的 io_context，结束所有的异步任务
private:
    // std::thread::hardware_concurrency()可以获取cpu核心数
    AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;  // 每个上下文要跑在一个线程里
    std::size_t _nextIOService;         // 记录下一个将要返回的 io_context 索引，实现轮询调度
};

