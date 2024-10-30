#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
	_get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection)
		{
			beast::ostream(connection->_response.body()) << "receive get_test req";
			int i = 0;
			for (auto& elem : connection->_get_params)
			{
				i++;
				beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
				beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
			}
		});

	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection)
		{	
			// ��Http�������Ϣ��ת��Ϊ�ַ��������ں�������json
			auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
			std::cout << "receive body is " << body_str << std::endl;
			// ���÷�������
			connection->_response.set(http::field::content_type, "text/json");
			Json::Value root;		// Ҫ���ظ��Է���
			Json::Reader reader;	// json������
			Json::Value src_root;	// �洢json�������

			bool parse_success = reader.parse(body_str, src_root); // body������src_root
			if (!parse_success)
			{
				std::cout << "Failed to parse JSON data!" << std::endl;
				root["error"] = ErrorCodes::Error_Json;		 // ��¼�����룬json��������
				std::string jsonstr = root.toStyledString(); // rootת��Ϊ��ʽ����json�ַ���
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			// �ж�json���Ƿ���key��û��Ҳ�����ʧ��
			if (!src_root.isMember("email"))
			{
				std::cout << "Failed to parse JSON data(no email in json)!" << std::endl;
				root["error"] = ErrorCodes::Error_Json;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			// ��������ɹ����� src_root ����ȡ�����ַ����������ӵ���Ӧ��
			auto email = src_root["email"].asString();
			// ����grpc���󲢻����Ӧ  ��������js�ķ���
			GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
			std::cout << "email is " << email << std::endl;
			root["error"] = rsp.error();
			root["email"] = src_root["email"];
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;

		});

	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		// ȡ��body������ݣ�ת��string
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		// ���ûذ�������Ҳ��json
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		// ����json����
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success)
		{
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		auto confirm = src_root["confirm"].asString();
		auto icon = src_root["icon"].asString();

		// �ж�ע��������������Ƿ���ͬ
		//if (pwd != confirm) 
		if (std::strcmp(pwd.c_str(), confirm.c_str()))
		{
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// �Ȳ���redis��email��Ӧ����֤���Ƿ����
		std::string  varify_code;
		// ��֤��д���ݿ��ʱ����˸�ǰ׺������get��ʱ��ҲҪ����
		// ���ж���֤���Ƿ����
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+src_root["email"].asString(), varify_code);
		if (!b_get_varify)
		{
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		// ��֤��δ���ڣ��ж���д����֤����redis�е���֤���Ƿ���ͬ
		if (varify_code != src_root["varifycode"].asString())
		{
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// ��redis�в����û��Ƿ��Ѿ�����
		//bool b_usr_exist = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
		//if (b_usr_exist)
		//{
		//	std::cout << " user exist" << std::endl;
		//	root["error"] = ErrorCodes::UserExist;
		//	std::string jsonstr = root.toStyledString();
		//	beast::ostream(connection->_response.body()) << jsonstr;
		//	return true;
		//}

		//�������ݿ��ж��û��Ƿ���� �ⲿ�ַŵ�MySQL����
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
		if (uid == 0 || uid == -1)
		{
			std::cout << " user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}


		// ע��ɹ������������Ϣ
		root["error"] = 0;
		root["email"] = email;
		root["uid"] = uid;
		root["user"] = name;
		root["passwd"] = pwd;
		root["confirm"] = confirm;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});

}

LogicSystem::~LogicSystem()
{

}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con) {
	if (_get_handlers.find(path) == _get_handlers.end())
	{
		return false;
	}

	_get_handlers[path](con); //con�Ǵ���handler�Ĳ���
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_post_handlers.find(path) == _post_handlers.end())
	{
		return false;
	}

	_post_handlers[path](con);
	return true;
}
