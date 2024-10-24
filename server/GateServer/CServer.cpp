#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	:_ioc(ioc)
	, _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{

}

void CServer::Start()
{
    auto self = shared_from_this();

    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    // �첽���տͻ������ӣ��������׽��ִ浽_socket���������Ӻ����lambda�ص�
    _acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
        try {
            //���������������ӣ���������������
            if (ec)
            {
                self->Start();
                return;
            }

            //���������ӣ�����HpptConnection�����������
            //std::make_shared<HttpConnection>(std::move(self->_socket))->Start();
            new_con->Start();
            //��������
            self->Start();
        }
        catch (std::exception& exp)
        {
            std::cout << "exception is " << exp.what() << std::endl;
            self->Start();
        }
    });
}
