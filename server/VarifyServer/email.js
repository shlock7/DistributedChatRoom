// nodemailer: 一个用于发送电子邮件的 Node.js 模块
// config_module: 自定义配置模块（config.js）
const nodemailer = require('nodemailer');
const config_module = require("./config")

/**
 * 创建发送邮件的代理
 */
let transport = nodemailer.createTransport({
    host: 'smtp.163.com',               // 指定使用的邮件服务器 163.com 的 SMTP 服务
    port: 465,
    secure: true,                       // 启用 SSL 安全连接
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass  // 邮箱授权码或者密码
    }
});

/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_) {
    return new Promise(function(resolve, reject){
        // 使用 nodemailer 的 transport 实例发送邮件
        transport.sendMail(mailOptions_, function(error, info){
            if (error) {
                console.log(error);
                reject(error);  // 异常，需要捕获
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)
            }
        });
    })
   
}

module.exports.SendMail = SendMail