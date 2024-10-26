#pragma once
#include <queue>
#include <atomic>
#include <mutex>
#include "Singleton.h"
#include "hiredis.h"


class RedisConPool
{
public:
    RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
        : poolSize_(poolSize)
        , host_(host)
        , port_(port)
        , b_stop_(false)
    {
        // Ϊÿ���̴߳���һ������
        for (size_t i = 0; i < poolSize_; ++i)
        {
            auto* context = redisConnect(host, port);
            if (context == nullptr || context->err != 0)
            {
                if (context != nullptr)
                {
                    redisFree(context);
                }
                continue;
            }

            auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
            if (reply->type == REDIS_REPLY_ERROR)
            {
                std::cout << "��֤ʧ��" << std::endl;
                //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
                freeReplyObject(reply);
                redisFree(context);
                continue;
            }

            //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
            freeReplyObject(reply);
            std::cout << "��֤�ɹ�" << std::endl;
            connections_.push(context);
        }

    }

    ~RedisConPool()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!connections_.empty())
        {
            connections_.pop();
        }
    }

    redisContext* getConnection()
    {
        // ����Ҫ��������֤�̵߳ķ����ǻ����
        std::unique_lock<std::mutex> lock(mutex_);
        // �ȴ���ֱ������ص�������������ж��ܲ�����
        cond_.wait(lock, [this] {
                if (b_stop_)        // 1. ���ӳعر�
                {
                    return true;
                }
                return !connections_.empty(); // 2. ���ӳطǿ�
            });
        //���ֹͣ��ֱ�ӷ��ؿ�ָ��
        if (b_stop_)
        {
            return  nullptr;
        }
        // ��������ӿ�����ȡ�������е����Ӳ�����
        auto* context = connections_.front();
        connections_.pop();
        return context;
    }
    // �߳���ʹ�������Ӻ󣬽����� context �Ż� connections_ ����
    void returnConnection(redisContext* context)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_)
        {
            return;
        }
        connections_.push(context);
        cond_.notify_one();  // ����һ���ȴ��߳�
    }

    void Close()
    {
        b_stop_ = true;
        cond_.notify_all(); // �������еȴ����̣߳������߳����޵ȴ�
    }

private:
    std::atomic<bool> b_stop_;              // ���ӳ��Ƿ�ֹͣ
    size_t poolSize_;                       // ���Ӵ�С
    const char* host_;                      // ��ַ
    int port_;                              // �˿�
    std::queue<redisContext*> connections_; // ���Ӷ���
    std::mutex mutex_;                      // �����������ƶ��г������
    std::condition_variable cond_;          // ���ڵȴ���֪ͨ����
};


class RedisMgr : public Singleton<RedisMgr>,
    public std::enable_shared_from_this<RedisMgr>
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    // ��ʼ�����ӵ�ʱ���Ѿ����Ӻ��ˣ����ﲻ��Ҫ���ӷ�����
    //bool Connect(const std::string& host, int port);
    bool Get(const std::string& key, std::string& value);
    bool Set(const std::string& key, const std::string& value);
    bool Auth(const std::string& password);
    bool LPush(const std::string& key, const std::string& value);
    bool LPop(const std::string& key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string& key, const std::string& hkey);
    bool Del(const std::string& key);
    bool ExistsKey(const std::string& key);
    void Close();
private:
    RedisMgr();

    //redisContext* _connect;
    //redisReply* _reply;
    std::unique_ptr<RedisConPool>  _con_pool;

};

