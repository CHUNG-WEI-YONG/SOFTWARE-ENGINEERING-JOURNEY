#include "const.h"
#include <boost/asio.hpp>
#include "ConfigMgr.h"
#include "RedisMjr.h"
#include <iostream>
#include <thread>
#include <memory>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
#include <cserver.h>
#include "FileServiceImpl.h"


// 包含你的业务头文件...

int main() {
    try {
        auto& cfg = ConfigMgr::Inst();

        // 1. 读取基础配置
        std::string host = cfg["FileServer"]["Host"];
        std::string port_str = cfg["FileServer"]["Port"];
        uint16_t port = static_cast<uint16_t>(std::stoi(port_str));
        std::string rpc_port = cfg["FileServer"]["RPCPort"];

        std::string redishost = cfg["RedisServer"]["Host"];
        std::string redisPort = cfg["RedisServer"]["Port"];
        std::string redisPass = cfg["RedisServer"]["Passwd"]; // 建议避免硬编码密码

        // 2. 初始化 Redis 连接（失败则阻断启动）
        if (!RedisMjr::GetInstance()->Connect(redishost, std::stoi(redisPort), redisPass, 5)) {
            std::cerr << "[FileServer] Fatal: Failed to connect to Redis ("
                << redishost << ":" << redisPort << ")" << std::endl;
            return 1;
        }

        // 3. 启动 Boost.Asio 数据面 (Data Plane)
        boost::asio::io_context ioc;

        // 保证即使临时无 IO 事件，ioc.run() 也不会提前退出
        auto work_guard = boost::asio::make_work_guard(ioc);

        auto server = std::make_shared<cserver>(ioc, port);
        server->Start();

        std::thread asio_thread([&ioc]() {
            try {
                ioc.run();
            }
            catch (const std::exception& e) {
                std::cerr << "[Asio Server Exception]: " << e.what() << std::endl;
            }
            });

        // 4. 配置并启动 gRPC 控制面 (Control Plane)
        // 修正端口拼接：统一绑定 0.0.0.0:RPCPort
        std::string grpc_address = "0.0.0.0:" + rpc_port;

        grpc::ServerBuilder builder;
        FileServiceImpl impl;
        builder.AddListeningPort(grpc_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&impl);

        std::unique_ptr<grpc::Server> grpc_server(builder.BuildAndStart());
        if (!grpc_server) {
            std::cerr << "[FileServer] Fatal: Failed to start gRPC server on " << grpc_address << std::endl;
            work_guard.reset();
            ioc.stop();
            if (asio_thread.joinable()) asio_thread.join();
            return 1;
        }

        std::cout << "[FileServer] gRPC Control Plane listening on: " << grpc_address << std::endl;
        std::cout << "[FileServer] Boost.Asio Data Plane listening on: " << port << std::endl;

        // 5. 监听系统中断信号以实现优雅退出 (Ctrl+C / SIGTERM)
        boost::asio::io_context sig_ioc;
        boost::asio::signal_set signals(sig_ioc, SIGINT, SIGTERM);
        signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
            if (!error) {
                std::cout << "\n[FileServer] Shutting down gracefully..." << std::endl;
                // 停止 gRPC 服务，唤醒 Wait()
                grpc_server->Shutdown();
            }
            });

        std::thread sig_thread([&sig_ioc]() {
            sig_ioc.run();
            });

        // 阻塞主线程，等待退出信号
        grpc_server->Wait();

        // 6. 清理退出流程
        sig_ioc.stop();
        if (sig_thread.joinable()) sig_thread.join();

        work_guard.reset();
        ioc.stop();
        if (asio_thread.joinable()) {
            asio_thread.join();
        }

        std::cout << "[FileServer] Clean exit completed." << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "[FileServer] Fatal Exception: " << e.what() << std::endl;
        return 1;
    }
}