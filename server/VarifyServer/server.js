
const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const const_module = require('./const')
const { v4: uuidv4 } = require('uuid');
const emailModule = require('./email');
const redis_module = require('./redis')

/**
 * GetVarifyCode grpc响应获取验证码的服务
 * @param {*} call 为来自客户端的 gRPC 请求数据
 * @param {*} callback 为gRPC 的回调函数，处理完请求后调用此函数向客户端返回结果
 * @returns 
 */
async function GetVarifyCode(call, callback)
{
    // 获取用户提交的邮箱地址，并输出日志
    console.log("email is ", call.request.email)
    try
    {
        // 前缀+邮箱地址作为key查询redis中是否有值（uuid）
        let query_res = await redis_module.GetRedis(const_module.code_prefix+call.request.email);
        console.log("query_res is ", query_res)
        let uniqueId = query_res;
        // 如果查询redis结果为空，说明没有uuid，在这里生成
        if(query_res ==null)
        {
            uniqueId = uuidv4();
            if (uniqueId.length > 4)
            {   // 截取前四位
                uniqueId = uniqueId.substring(0, 4);
            }
            // 把请求的邮箱地址作为key，uuid作为值，加上超时时间(600s)添加到redis中
            let bres = await redis_module.SetRedisExpire(const_module.code_prefix+call.request.email, uniqueId,600)
            if(!bres)
            {
                // 如果添加失败，返回错误码
                callback(null, 
                    { 
                        email:  call.request.email,
                        error:const_module.Errors.RedisErr
                    });
                return;
            }
        }

        //uniqueId = uuidv4();        // 使用 uuid 库生成唯一的验证码。v4 是 UUID 的一种版本
        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: 'shk1717@163.com',  // 发送者邮箱地址
            to: call.request.email,   // 接收者邮箱地址，即从 gRPC 请求中获取的 call.request.email
            subject: '验证码',         // 邮件主题
            text: text_str,            // 邮件正文内容
        };
    
        // 该函数返回一个 Promise，所以使用 await 等待发送结果
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res is ", send_res)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Success
        }); 
        
 
    }
    catch(error)
    {
        console.log("catch error is ", error)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
     
}

function main()
{
    var server = new grpc.Server()  // 启动grpc Server
    // 向 gRPC 服务器添加服务  从 proto 文件中加载的服务定义
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })
    // 异步绑定服务器地址
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')        
    })
}

main()
