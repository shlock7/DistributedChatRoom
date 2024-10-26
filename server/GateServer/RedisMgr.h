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
        // 为每个线程创建一个连接
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
                std::cout << "认证失败" << std::endl;
                //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
                freeReplyObject(reply);
                redisFree(context);
                continue;
            }

            //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
            freeReplyObject(reply);
            std::cout << "认证成功" << std::endl;
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
        // 这里要加锁，保证线程的访问是互斥的
        std::unique_lock<std::mutex> lock(mutex_);
        // 等待，直到满足回调里的条件（先判断能不能用
        cond_.wait(lock, [this] {
                if (b_stop_)        // 1. 连接池关闭
                {
                    return true;
                }
                return !connections_.empty(); // 2. 连接池非空
            });
        //如果停止则直接返回空指针
        if (b_stop_)
        {
            return  nullptr;
        }
        // 如果有连接可用则取出队列中的连接并返回
        auto* context = connections_.front();
        connections_.pop();
        return context;
    }
    // 线程在使用完连接后，将连接 context 放回 connections_ 队列
    void returnConnection(redisContext* context)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_)
        {
            return;
        }
        connections_.push(context);
        cond_.notify_one();  // 唤醒一个等待线程
    }

    void Close()
    {
        b_stop_ = true;
        cond_.notify_all(); // 唤醒所有等待的线程，避免线程无限等待
    }

private:
    std::atomic<bool> b_stop_;              // 连接池是否停止
    size_t poolSize_;                       // 池子大小
    const char* host_;                      // 地址
    int port_;                              // 端口
    std::queue<redisContext*> connections_; // 连接队列
    std::mutex mutex_;                      // 互斥量，控制队列出队入队
    std::condition_variable cond_;          // 用于等待和通知操作
};


class RedisMgr : public Singleton<RedisMgr>,
    public std::enable_shared_from_this<RedisMgr>
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    // 初始化池子的时候已经连接好了，这里不需要连接方法了
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

