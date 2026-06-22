const path = require('path');
const grpc = require('@grpc/grpc-js'); 
const protoloader = require('@grpc/proto-loader');
const message_proto = require('./proto'); 
const const_module = require('./const');
const emailModule = require('./email');
const redisModule = require('./redis');

async function GetVarifyCode(call, callback) {
    const email = call.request.email;
    console.log("🟢 [gRPC Request] Incoming email for verify code:", email);
    
    try {
        // 🚀 1. 从缓存安全捞货
        let query_req = await redisModule.RedisGet(const_module.code_prefix + email);
        console.log("📡 [Redis Query Result] Cached code is:", query_req);
        
        let unique_id = query_req; // ⚖️ 全量统一使用下划线变量 unique_id
        
        // 🛡️ 2. 如果缓存中不存在 (NIL)，才拉起新验证码车间
        if (query_req == null) {
            // 🎲 用 4 位干净的纯数字代替容易自爆的 UUID 截取法
            unique_id = Math.floor(1000 + Math.random() * 9000).toString();
            console.log(`✨ [Code Generator] New 4-digit code created: ${unique_id}`);
            
            // 🚀 3. 安全落盘 Redis 
            let bres = await redisModule.SetRedisExpire(const_module.code_prefix + email, unique_id, 600);
            
            if (!bres) {
                console.error("🔴 [Redis Error] SetRedisExpire failed.");
                // 🔒 必须带上 return 拦截！
                return callback(null, {
                    email: email,
                    error: const_module.Errors.RedisErr
                });
            }
        }
        
        // 📨 4. 变量名完全咬合，组装完全体发信文本
        let text_str = 'Your verify code is ' + unique_id + ' Please do the verification within 3 minutes.';
        
        let mailOptions = {
            from: 'workedcwy@gmail.com',
            to: email,
            subject: '验证码',
            text: text_str,
        };
        
        console.log(`📧 [Email Dispatcher] Dispatching mail to ${email}...`);
        
        // 🚀 5. 全自动发信
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("📡 [Email System Response] Send result is:", send_res);
        
        if (!send_res) {
            console.error("🔴 [Email Error] SendMail returned false.");
            return callback(null, { 
                email: email, 
                error: const_module.Errors.EmailErr 
            });
        }
        
        // 🎉 6. 全链路大获全胜，向 C++ GateServer 轰回成功应答
        console.log(`🎯 [gRPC Success] Verification workflow fully closed for ${email}`);
        callback(null, { 
            email: email,
            error: const_module.Errors.Success
        }); 
        
    } catch (error) {
        // 🔒 捕获任何可能残留的语法炸弹
        console.error("❌ [Server Exception Occurred] catch block triggered:", error);
        callback(null, { 
            email: email,
            error: const_module.Errors.Exception
        }); 
    }
}

function main() {
    var server = new grpc.Server()
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('🚀 [gRPC Ready] Verify gRPC server started on 0.0.0.0:50051')        
    })
}
main();