#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

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
        Status status = stub_->GetVarifyCode(&context, request, &reply);

        if (status.ok()) {

            return reply;
        }
        else {
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
        }
    }

private:
    VerifyGrpcClient() {
        // 创建与远程服务（运行在 127.0.0.1:50051 上）的通信通道，grpc用这个通道通信
        std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", 
            grpc::InsecureChannelCredentials());  // 不使用任何加密或身份验证
        stub_ = VarifyService::NewStub(channel);  // 将channel分配给stub
    }

    std::unique_ptr<VarifyService::Stub> stub_;  // 信使，通过它才能跟别人通信
};

