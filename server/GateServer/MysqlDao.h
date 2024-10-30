#pragma once
#include "const.h"
#include <thread>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>
#include <queue>


// 封装sql连接 用于超时检测
class SqlConnection
{
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) :_con(con), _last_oper_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;  // sql的连接
	int64_t _last_oper_time;	// 上次操作数据库的时间
};

class MySqlPool {
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
		: url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize), b_stop_(false)
	{
		try {
			for (int i = 0; i < poolSize_; ++i)
			{
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* con = driver->connect(url_, user_, pass_);  // 避免手动回收，放到池子里用智能指针管理
				con->setSchema(schema_);
				// 获取当前时间戳
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				// 将时间戳转换为秒
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				pool_.push(std::make_unique<SqlConnection>(con, timestamp));
			}
			
			// 检测线程，相当于心跳，每60秒检测一次连接
			_check_thread = std::thread([this](){
					while (!b_stop_)
					{
						checkConnection();
						std::this_thread::sleep_for(std::chrono::seconds(60));
					}
				});

			_check_thread.detach();
		}
		catch (sql::SQLException& e)
		{
			// 处理异常
			std::cout << "mysql pool init failed, error is " << e.what() << std::endl;
		}
	}

	// 检查池子中的连接是否正常
	void checkConnection() {
		std::lock_guard<std::mutex> guard(mutex_);
		int poolsize = pool_.size();   // size会变，要提前取出来
		// 获取当前时间戳
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		// 将时间戳转换为秒
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		for (int i = 0; i < poolsize; i++)
		{
			auto con = std::move(pool_.front());
			pool_.pop();
			// RAII思想，实现一个类似GO语言中的defer
			Defer defer([this, &con]() {
				pool_.push(std::move(con));
				});
			/*
			* 前面pop()了，如果连接时间是否小于5秒直接continue了
			* pop()出来的指针还没放回去，这里定义一个Defer类对象
			* 这个对象在右括号之前一定会析构，我们定义在析构函数里的操作一定会执行
			* 这样大括号内的程序执行完之后才会执行这部分内容
			*/

			if (timestamp - con->_last_oper_time < 5)
			{
				continue;
			}

			try
			{
				std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
				stmt->executeQuery("SELECT 1");
				con->_last_oper_time = timestamp;
				//std::cout << "execute timer alive query , cur is " << timestamp << std::endl;
			}
			catch (sql::SQLException& e)
			{
				std::cout << "Error keeping connection alive: " << e.what() << std::endl;
				// 重新创建连接并替换旧的连接
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* newcon = driver->connect(url_, user_, pass_);
				newcon->setSchema(schema_);
				con->_con.reset(newcon);
				con->_last_oper_time = timestamp;
			}
		}
	}

	// 从池子中取一个连接
	std::unique_ptr<SqlConnection> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
				if (b_stop_)
				{
					return true;		// 直接返回
				}
				return !pool_.empty();  // 返回false就会挂起，等待被唤醒
			});
		if (b_stop_)
		{
			return nullptr;
		}
		std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
		pool_.pop();
		return con;
	}

	// 将连接放回池子
	void returnConnection(std::unique_ptr<SqlConnection> con) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (b_stop_)
		{
			return;
		}
		pool_.push(std::move(con));
		cond_.notify_one();		// 通知因池子为空而挂起的队列
	}

	// 关闭连接池
	void Close()
	{
		b_stop_ = true;
		cond_.notify_all();		// 通知所有线程
	}

	~MySqlPool()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (!pool_.empty())
		{
			pool_.pop();
		}
	}

private:
	std::string url_;		// 连接数据库的url
	std::string user_;		// 用户名
	std::string pass_;		// 密码
	std::string schema_;	// 要用哪个数据库
	int poolSize_;			// 连接池大小
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;				// 保证队列安全
	std::condition_variable cond_;	// 条件变量
	std::atomic<bool> b_stop_;		// 停止标志
	std::thread _check_thread;		// 检测线程，每隔一分钟检测连接，如果还在就发一个请求，心跳
};

struct UserInfo
{
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};


// 这个类封装了MySQL的增删改查功能
class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	//int RegUserTransaction(const std::string& name, const std::string& email, const std::string& pwd, const std::string& icon);
	//bool CheckEmail(const std::string& name, const std::string& email);
	//bool UpdatePwd(const std::string& name, const std::string& newpwd);
	//bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
	//bool TestProcedure(const std::string& email, int& uid, std::string& name);
private:
	std::unique_ptr<MySqlPool> pool_;
};



