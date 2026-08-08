#include "ConfigMgr.h"
#include "Cserver.h"
#include "AsioIOServerPool.h"
#include "LogicSystem.h"
#include <csignal>
#include <mutex>
#include <thread>
#include "RedisMjr.h"
#include "ChatServiceImpl.h"

bool b_stop = false;
std::mutex mutex_quit;
std::condition_variable cv_quit;

void InitServerStatus() {
	std::string server_name = ConfigMgr::Inst()["SelfServer"]["Name"];

	// 🎯 服务器启动时，强制将当前 ChatServer 节点在 Redis 中的在线人数归零
	bool res = RedisMjr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");
	if (res) {
		std::cout << "Successfully initialized " << server_name << " logincount to 0 in Redis." << std::endl;
	}
	else {
		std::cerr << "Failed to initialize " << server_name << " logincount in Redis!" << std::endl;
	}
}

int main() {
	try {
		//reset login count
		auto cfg = ConfigMgr::Inst();
		auto pool = AsioIOServerPool::GetInstance();
		std::cout << "This is Chatserver " <<cfg["SelfServer"]["Name"] << std::endl;
		//std::string status_server_address = cfg["StatusServer"]["Host"] + ":" + cfg["StatusServer"]["Port"];
		std::string redishost = cfg["RedisServer"]["Host"];
		std::string redisport = cfg["RedisServer"]["Port"];
		std::string password = cfg["RedisServer"]["Passwd"];
		std::string server_name = cfg["SelfServer"]["Name"];
		int redis_int = std::stoi(redisport);
		if (RedisMjr::GetInstance()->Connect(redishost, redis_int, password, 5)) {
			std::cout << "Redis connect correctly at " << redishost << ":" << redisport;
		}
		else {
			std::cout << "Redis cannot connect";
		}
		InitServerStatus();
		//initialize grpc connection
		std::string server_address(cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["Port"]);
		ChatServiceImpl Service;
		grpc::ServerBuilder builder;
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&Service);
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "grpc build and run on " << server_address << std::endl;

		std::thread server_thread([&server]() {
			server->Wait();
			});

		boost::asio::io_context ioc;
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc,pool,&server](auto, auto) {
			ioc.stop();
			pool->close();
			server->Shutdown();
			});
		RedisMjr::GetInstance()->HDel(LOGIN_COUNT, server_name);
		RedisMjr::GetInstance()->Close();
		auto self_name = cfg["SelfServer"]["Name"];
		auto port_str = cfg[self_name]["Port"];
		uint16_t port = static_cast<uint16_t>(std::stoul(port_str));
		Cserver s(ioc, port);
		ioc.run();


	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}
}