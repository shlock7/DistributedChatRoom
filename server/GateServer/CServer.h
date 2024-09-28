#pragma once

#include "const.h"

class CServer:public std::enable_shared_from_this<CServer>
{
public:
    // 通信的上下文，端口
    CServer(boost::asio::io_context& ioc, unsigned short& port);
    void Start();   // 服务器启动
private:
    tcp::acceptor  _acceptor;
    net::io_context& _ioc;      // 上下文
    tcp::socket   _socket;      // 用来复用的socket
};

