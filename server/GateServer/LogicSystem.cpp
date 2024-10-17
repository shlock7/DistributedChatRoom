#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"

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
			// 发送grpc请求并获得响应
			GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
			std::cout << "email is " << email << std::endl;
			root["error"] = rsp.error();
			root["email"] = src_root["email"];
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
