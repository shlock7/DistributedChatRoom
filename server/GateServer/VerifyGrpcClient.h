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
            // ������Զ�̷��������� 127.0.0.1:50051 �ϣ���ͨ��ͨ����grpc�����ͨ��ͨ��
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
                grpc::InsecureChannelCredentials());  // ��ʹ���κμ��ܻ������֤
            connections_.push(VarifyService::NewStub(channel));  // ��channel�����stub
            // �� ����push���ƶ�����
        }
    }

    ~RPConPool()
    {
        std::lock_guard<std::mutex> lock(mutex_); // b_stop����
        Close();
        while (!connections_.empty())
        {
            connections_.pop();
        }
    }

    // ֪ͨ������̣߳�����Ҫ�����˲�������
    void Close()
    {
        b_stop_ = true;
        cond_.notify_all(); // �������еȴ��е��̣߳�֪ͨ�������ӳ��ѹر�
    }

    std::unique_ptr<VarifyService::Stub> getConnection()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // ʹ��ǰ�̵߳ȴ���ֱ��������������
        cond_.wait(lock, [this]
            {
                if (b_stop_) // 1. ���ӻ�����
                {
                    return true;
                }
                return !connections_.empty(); // 2. ���ӷǿգ����п�������
            });
        
        //������ӳعر���ֱ�ӷ��ؿ�ָ��
        if (b_stop_)
        {
            return  nullptr;
        }
        auto context = std::move(connections_.front()); // û�п�����Ҫ���ƶ�
        connections_.pop();
        return context;
    }

    // �߳���ʹ�������Ӻ󣬽����� context �Ż� connections_ ����
    void returnConnection(std::unique_ptr<VarifyService::Stub> context)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_)
        {
            return;
        }
        connections_.push(std::move(context));
        cond_.notify_one();  // �Żغ��ѵȴ��߳�
    }

private:
    std::atomic<bool> b_stop_;  // ��ʶ���ӳ�����״̬��true��ʾֹͣ
    size_t poolSize_;           // ���Ӵ�С
    std::string host_;          // ��������ַ
    std::string port_;          // �������˿�
    std::queue<std::unique_ptr<VarifyService::Stub>> connections_; // �洢�ɸ��õ�stub����
    std::condition_variable cond_; // �������ӵȴ���֪ͨ
    std::mutex mutex_;

};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    // ��Singleton����Ϊ��Ԫ��Singleton���ܷ���˽�й���
    friend class Singleton<VerifyGrpcClient>;
public:
    // ����grpc���󣬻�ȡ��֤��
    GetVarifyRsp GetVarifyCode(std::string email) {
        ClientContext context;      // gRPC �ͻ��������� context�����ڹ��������Ԫ���ݺ�ѡ�
        GetVarifyRsp reply;         // ��Ӧ�����������շ��������ص�����
        GetVarifyReq request;       // ������󣬰��������͵���������
        request.set_email(email);   // ���������еĵ����ʼ�����

        // ���� gRPC ���ã����� stub_ �� gRPC �ͻ��˴����Stub��������װ�˿ͻ�����Զ�̷����ͨ���߼�
        // ������ûὫ context��request �� reply ��Ϊ�������ݸ�Զ�� gRPC ����
        // ������һ�� status ���󣬱�ʾ�����Ƿ�ɹ�
        auto stub = pool_->getConnection(); // �ĳɴӳ�����ȡһ��
        Status status = stub->GetVarifyCode(&context, request, &reply);

        if (status.ok())
        {
            pool_->returnConnection(std::move(stub)); // ����Ҫ�ͷ�������Ӹ������߳���
            return reply;
        }
        else
        {
            pool_->returnConnection(std::move(stub)); // ʧ��ҲҪ�ͷ��������
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
        }
    }

private:
    //VerifyGrpcClient() {
    //    // ������Զ�̷��������� 127.0.0.1:50051 �ϣ���ͨ��ͨ����grpc�����ͨ��ͨ��
    //    std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", 
    //        grpc::InsecureChannelCredentials());  // ��ʹ���κμ��ܻ������֤
    //    stub_ = VarifyService::NewStub(channel);  // ��channel�����stub
    //}
    
    //std::unique_ptr<VarifyService::Stub> stub_;  // ��ʹ��ͨ�������ܸ�����ͨ��

    VerifyGrpcClient();
    std::unique_ptr<RPConPool> pool_;
};

