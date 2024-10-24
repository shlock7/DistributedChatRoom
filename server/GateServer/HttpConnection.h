#pragma once
#include "const.h"
#include "LogicSystem.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
    friend class LogicSystem;
public:
    //HttpConnection(tcp::socket socket);
    // ���ӳذ汾
    HttpConnection(boost::asio::io_context& ioc);
    void Start();
    tcp::socket& GetSocket()
    {
        return _socket;
    }

private:
    void CheckDeadline();   // ��ⳬʱ
    void WriteResponse();   // Ӧ��
    void HandleReq();       // ��������
    tcp::socket  _socket;   //

    void PreParseGetParam();

    // The buffer for performing reads.
    beast::flat_buffer  _buffer{ 8192 };  // 8k�Ļ�����

    // The request message.
    http::request<http::dynamic_body> _request;

    // The response message.
    http::response<http::dynamic_body> _response;

    // The timer for putting a deadline on connection processing.
    net::steady_timer deadline_{
        _socket.get_executor(), std::chrono::seconds(60)
    };

    std::string _get_url;
    std::unordered_map<std::string, std::string> _get_params;
};

