// 验证码前缀
let code_prefix = "code_";

// 错误码
const Errors = {
    Success : 0,
    RedisErr : 1,
    Exception : 2,
};

// 导出
module.exports = {code_prefix,Errors}
