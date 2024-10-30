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
			// 将Http请求的消息体转化为字符串，便于后续解析json
			auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
			std::cout << "receive body is " << body_str << std::endl;
			// 设置返回类型
			connection->_response.set(http::field::content_type, "text/json");
			Json::Value root;		// 要返回给对方的
			Json::Reader reader;	// json解析器
			Json::Value src_root;	// 存储json解析结果

			bool parse_success = reader.parse(body_str, src_root); // body解析给src_root
			if (!parse_success)
			{
				std::cout << "Failed to parse JSON data!" << std::endl;
				root["error"] = ErrorCodes::Error_Json;		 // 记录错误码，json解析错误
				std::string jsonstr = root.toStyledString(); // root转化为格式化的json字符串
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			// 判断json中是否有key，没有也算解析失败
			if (!src_root.isMember("email"))
			{
				std::cout << "Failed to parse JSON data(no email in json)!" << std::endl;
				root["error"] = ErrorCodes::Error_Json;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			// 如果解析成功，从 src_root 中提取邮箱地址，并将其添加到响应中
			auto email = src_root["email"].asString();
			// 发送grpc请求并获得响应  这里会调用js的服务
			GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
			std::cout << "email is " << email << std::endl;
			root["error"] = rsp.error();
			root["email"] = src_root["email"];
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;

		});

	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		// 取出body里的数据，转成string
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		// 设置回包，类型也是json
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		// 解析json请求
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

		// 判断注册界面两次密码是否相同
		//if (pwd != confirm) 
		if (std::strcmp(pwd.c_str(), confirm.c_str()))
		{
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 先查找redis中email对应的验证码是否合理
		std::string  varify_code;
		// 验证码写数据库的时候加了个前缀，这里get的时候也要加上
		// 先判断验证码是否过期
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+src_root["email"].asString(), varify_code);
		if (!b_get_varify)
		{
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		// 验证码未过期，判断填写的验证码与redis中的验证码是否相同
		if (varify_code != src_root["varifycode"].asString())
		{
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 在redis中查找用户是否已经存在
		//bool b_usr_exist = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
		//if (b_usr_exist)
		//{
		//	std::cout << " user exist" << std::endl;
		//	root["error"] = ErrorCodes::UserExist;
		//	std::string jsonstr = root.toStyledString();
		//	beast::ostream(connection->_response.body()) << jsonstr;
		//	return true;
		//}

		//查找数据库判断用户是否存在 这部分放到MySQL中做
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
		if (uid == 0 || uid == -1)
		{
			std::cout << " user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}


		// 注册成功，返回相关信息
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

	_get_handlers[path](con); //con是传给handler的参数
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
