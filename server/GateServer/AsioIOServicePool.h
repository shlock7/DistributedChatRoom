#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work; // ����ȷ�� io_context ���󲻻�ֹͣ����
    using WorkPtr = std::unique_ptr<Work>;      // �����Զ����� Work �������������
    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
    // ʹ�� round-robin �ķ�ʽ����һ�� io_service
    boost::asio::io_context& GetIOService();  
    void Stop(); // ֹͣ���е� io_context���������е��첽����
private:
    // std::thread::hardware_concurrency()���Ի�ȡcpu������
    AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;  // ÿ��������Ҫ����һ���߳���
    std::size_t _nextIOService;         // ��¼��һ����Ҫ���ص� io_context ������ʵ����ѯ����
};

