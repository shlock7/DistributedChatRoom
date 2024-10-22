// 导入依赖库
const path = require('path')
const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

// 拼接出proto文件的绝对路径 __dirname表示当前文件夹所在目录
const PROTO_PATH = path.join(__dirname, 'message.proto')
// 加载并解析proto文件
const packageDefinition = protoLoader.loadSync(PROTO_PATH, { 
    keepCase: true, 
    longs: String, 
    enums: String, 
    defaults: true, 
    oneofs: true 
  })

// 将 packageDefinition 载入 gRPC，使其成为一个可供 gRPC 使用的服务定义或消息描述符对象。
const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)
// 从 protoDescriptor 中获取名为 message 的服务或消息类型
const message_proto = protoDescriptor.message

module.exports = message_proto