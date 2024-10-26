#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class RPConPool
{
public:
    RPConPool(size_t poolsize, std::string host, std::string port)
        : poolSize_(poolsize)
        , host_(host)
        , port_(port)
        , b_stop_(false)
    {
        for (size_t i = 0; i < poolSize_; ++i)
        {
            // 创建与远程服务（运行在 127.0.0.1:50051 上）的通信通道，grpc用这个通道通信
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
                grpc::InsecureChannelCredentials());  // 不使用任何加密或身份验证
            connections_.push(VarifyService::NewStub(channel));  // 将channel分配给stub
            // ↑ 这里push是移动构造
        }
    }

    ~RPConPool()
    {
        std::lock_guard<std::mutex> lock(mutex_); // b_stop互斥
        Close();
        while (!connections_.empty())
        {
            connections_.pop();
        }
    }

    // 通知挂起的线程，池子要回收了不能用了
    void Close()
    {
        b_stop_ = true;
        cond_.notify_all(); // 唤醒所有等待中的线程，通知它们连接池已关闭
    }

    std::unique_ptr<VarifyService::Stub> getConnection()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 使当前线程等待，直到满足以下条件
        cond_.wait(lock, [this]
            {
                if (b_stop_) // 1. 池子回收了
                {
                    return true;
                }
                return !connections_.empty(); // 2. 池子非空，即有可用连接
            });
        
        //如果连接池关闭则直接返回空指针
        if (b_stop_)
        {
            return  nullptr;
        }
        auto context = std::move(connections_.front()); // 没有拷贝，要用移动
        connections_.pop();
        return context;
    }

    // 线程在使用完连接后，将连接 context 放回 connections_ 队列
    void returnConnection(std::unique_ptr<VarifyService::Stub> context)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_)
        {
            return;
        }
        connections_.push(std::move(context));
        cond_.notify_one();  // 放回后唤醒等待线程
    }

private:
    std::atomic<bool> b_stop_;  // 标识连接池运行状态，true表示停止
    size_t poolSize_;           // 池子大小
    std::string host_;          // 服务器地址
    std::string port_;          // 服务器端口
    std::queue<std::unique_ptr<VarifyService::Stub>> connections_; // 存储可复用的stub对象
    std::condition_variable cond_; // 控制连接等待和通知
    std::mutex mutex_;

};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    // 将Singleton声明为友元，Singleton才能访问私有构造
    friend class Singleton<VerifyGrpcClient>;
public:
    // 发起grpc请求，获取验证码
    GetVarifyRsp GetVarifyCode(std::string email) {
        ClientContext context;      // gRPC 客户端上下文 context，用于管理请求的元数据和选项。
        GetVarifyRsp reply;         // 响应对象，用来接收服务器返回的数据
        GetVarifyReq request;       // 请求对象，包含待发送的请求数据
        request.set_email(email);   // 设置请求中的电子邮件参数

        // 发起 gRPC 调用，其中 stub_ 是 gRPC 客户端存根（Stub），它封装了客户端与远程服务的通信逻辑
        // 这个调用会将 context、request 和 reply 作为参数传递给远程 gRPC 服务
        // 并返回一个 status 对象，表示调用是否成功
        auto stub = pool_->getConnection(); // 改成从池子里取一个
        Status status = stub->GetVarifyCode(&context, request, &reply);

        if (status.ok())
        {
            pool_->returnConnection(std::move(stub)); // 用完要释放这个连接给其他线程用
            return reply;
        }
        else
        {
            pool_->returnConnection(std::move(stub)); // 失败也要释放这个连接
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
        }
    }

private:
    //VerifyGrpcClient() {
    //    // 创建与远程服务（运行在 127.0.0.1:50051 上）的通信通道，grpc用这个通道通信
    //    std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", 
    //        grpc::InsecureChannelCredentials());  // 不使用任何加密或身份验证
    //    stub_ = VarifyService::NewStub(channel);  // 将channel分配给stub
    //}
    
    //std::unique_ptr<VarifyService::Stub> stub_;  // 信使，通过它才能跟别人通信

    VerifyGrpcClient();
    std::unique_ptr<RPConPool> pool_;
};

