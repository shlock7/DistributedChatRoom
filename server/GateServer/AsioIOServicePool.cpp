#include "AsioIOServicePool.h"
#include <iostream>
using namespace std;

// io_context û�п�����һ��Ҫ�ڳ�ʼ���б��г�ʼ��
AsioIOServicePool::AsioIOServicePool(std::size_t size) :_ioServices(size),
                                                    _works(size), _nextIOService(0)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        // Ϊÿ�� io_context ���󴴽�һ�� Work ���� unique_ptr����ָ��
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
    }

    //�������ioservice����������̣߳�ÿ���߳��ڲ�����ioservice
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
    // ���� ��ѯ��round-robin�� �ķ�ʽ��������
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size())
    {
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop()
{
    //��Ϊ����ִ��work.reset��������iocontext��run��״̬���˳�
    //��iocontext�Ѿ����˶���д�ļ����¼��󣬻���Ҫ�ֶ�stop�÷���
    for (auto& work : _works)
    {
        //�ѷ�����ֹͣ
        work->get_io_context().stop();
        work.reset();
    }

    // �ȴ�ÿ���߳�ִ����ϣ�ȷ�������̰߳�ȫ�˳�
    for (auto& t : _threads)
    {
        t.join();
    }
}
