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


// ��װsql���� ���ڳ�ʱ���
class SqlConnection
{
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) :_con(con), _last_oper_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;  // sql������
	int64_t _last_oper_time;	// �ϴβ������ݿ��ʱ��
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
				auto* con = driver->connect(url_, user_, pass_);  // �����ֶ����գ��ŵ�������������ָ�����
				con->setSchema(schema_);
				// ��ȡ��ǰʱ���
				auto currentTime = std::chrono::system_clock::now().time_since_epoch();
				// ��ʱ���ת��Ϊ��
				long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
				pool_.push(std::make_unique<SqlConnection>(con, timestamp));
			}
			
			// ����̣߳��൱��������ÿ60����һ������
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
			// �����쳣
			std::cout << "mysql pool init failed, error is " << e.what() << std::endl;
		}
	}

	// �������е������Ƿ�����
	void checkConnection() {
		std::lock_guard<std::mutex> guard(mutex_);
		int poolsize = pool_.size();   // size��䣬Ҫ��ǰȡ����
		// ��ȡ��ǰʱ���
		auto currentTime = std::chrono::system_clock::now().time_since_epoch();
		// ��ʱ���ת��Ϊ��
		long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
		for (int i = 0; i < poolsize; i++)
		{
			auto con = std::move(pool_.front());
			pool_.pop();
			// RAII˼�룬ʵ��һ������GO�����е�defer
			Defer defer([this, &con]() {
				pool_.push(std::move(con));
				});
			/*
			* ǰ��pop()�ˣ��������ʱ���Ƿ�С��5��ֱ��continue��
			* pop()������ָ�뻹û�Ż�ȥ�����ﶨ��һ��Defer�����
			* ���������������֮ǰһ�������������Ƕ���������������Ĳ���һ����ִ��
			* �����������ڵĳ���ִ����֮��Ż�ִ���ⲿ������
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
				// ���´������Ӳ��滻�ɵ�����
				sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
				auto* newcon = driver->connect(url_, user_, pass_);
				newcon->setSchema(schema_);
				con->_con.reset(newcon);
				con->_last_oper_time = timestamp;
			}
		}
	}

	// �ӳ�����ȡһ������
	std::unique_ptr<SqlConnection> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
				if (b_stop_)
				{
					return true;		// ֱ�ӷ���
				}
				return !pool_.empty();  // ����false�ͻ���𣬵ȴ�������
			});
		if (b_stop_)
		{
			return nullptr;
		}
		std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
		pool_.pop();
		return con;
	}

	// �����ӷŻس���
	void returnConnection(std::unique_ptr<SqlConnection> con) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (b_stop_)
		{
			return;
		}
		pool_.push(std::move(con));
		cond_.notify_one();		// ֪ͨ�����Ϊ�ն�����Ķ���
	}

	// �ر����ӳ�
	void Close()
	{
		b_stop_ = true;
		cond_.notify_all();		// ֪ͨ�����߳�
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
	std::string url_;		// �������ݿ��url
	std::string user_;		// �û���
	std::string pass_;		// ����
	std::string schema_;	// Ҫ���ĸ����ݿ�
	int poolSize_;			// ���ӳش�С
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;				// ��֤���а�ȫ
	std::condition_variable cond_;	// ��������
	std::atomic<bool> b_stop_;		// ֹͣ��־
	std::thread _check_thread;		// ����̣߳�ÿ��һ���Ӽ�����ӣ�������ھͷ�һ����������
};

struct UserInfo
{
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};


// ������װ��MySQL����ɾ�Ĳ鹦��
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



