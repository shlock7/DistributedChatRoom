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
        // ������Զ�̷��������� 127.0.0.1:50051 �ϣ���ͨ��ͨ����grpc�����ͨ��ͨ��
        std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", 
            grpc::InsecureChannelCredentials());  // ��ʹ���κμ��ܻ������֤
        stub_ = VarifyService::NewStub(channel);  // ��channel�����stub
    }

    std::unique_ptr<VarifyService::Stub> stub_;  // ��ʹ��ͨ�������ܸ�����ͨ��
};

