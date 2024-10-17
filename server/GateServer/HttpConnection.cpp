#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(tcp::socket socket)
    : _socket(std::move(socket))
{
}

void HttpConnection::Start()
{
    auto self = shared_from_this();
    http::async_read(_socket, _buffer, _request, 
        [self](beast::error_code ec, std::size_t bytes_transferred)
        {
            try {
                if (ec)
                {
                    std::cout << "http read err is " << ec.what() << std::endl;
                    return;
                }

                //�������������
                boost::ignore_unused(bytes_transferred); // http���ô���ճ��
                self->HandleReq();      // ����http����
                self->CheckDeadline();  // ��ʱ���
            }
            catch (std::exception& exp)
            {
                std::cout << "exception is " << exp.what() << std::endl;
            }
        }
    );
}

unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //��ԭ+Ϊ��
        if (str[i] == '+') strTemp += ' ';
        //����%������������ַ���16����תΪchar��ƴ��
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}

void HttpConnection::PreParseGetParam() {
    // ��ȡ URI : ex: get_test?key1=value1&key2=value2
    auto uri = _request.target();
    // ���Ҳ�ѯ�ַ����Ŀ�ʼλ�ã��� '?' ��λ�ã�  
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos) {
        _get_url = uri;
        return;
    }

    // �ָ��ַ�����query_pos ָ��?��λ��
    _get_url = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    // ����ͨ�� & �ָ���ͨ������ & ���м�ֵ����ȡ
    while ((pos = query_string.find('&')) != std::string::npos) {
        auto pair = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, eq_pos)); // ������ url_decode ����������URL����  
            value = UrlDecode(pair.substr(eq_pos + 1)); // ����ȥ����%�������ַ�
            _get_params[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // �������һ�������ԣ����û�� & �ָ�����  
    if (!query_string.empty()) {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, eq_pos));
            value = UrlDecode(query_string.substr(eq_pos + 1));
            _get_params[key] = value;
        }
    }
}

void HttpConnection::HandleReq()
{
    _response.version(_request.version());  // ���ð汾
    _response.keep_alive(false);            // ����Ϊ������ http����ά�ֳ�����

    if (_request.method() == http::verb::get) // ����http��get����
    {
        PreParseGetParam(); 
        bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
        if (!success)
        {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";  // ����Ϣ����д����
            WriteResponse();
            return;
        }

        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }

    if (_request.method() == http::verb::post)
    {
        bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
        if (!success)
        {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }

        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }
}



void HttpConnection::WriteResponse()
{
    auto self = shared_from_this();
    // ���ð��峤��
    _response.content_length(_response.body().size());
    http::async_write(
        _socket,
        _response,
        [self](beast::error_code ec, std::size_t)
        {
            self->_socket.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline_.cancel();
        });
}

void HttpConnection::CheckDeadline()
{
    auto self = shared_from_this();

    deadline_.async_wait(
        [self](beast::error_code ec)
        {
            if (!ec)
            {
                // Close socket to cancel any outstanding operation.
                self->_socket.close(ec);
            }
        });
}