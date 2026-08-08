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
		auto cfg = ConfigMgr::Inst();
		auto pool = AsioIOServerPool::GetInstance();
		std::cout << "This is Chatserver " << cfg["SelfServer"]["Name"] << std::endl;

		std::string redishost = cfg["RedisServer"]["Host"];
		std::string redisport = cfg["RedisServer"]["Port"];
		std::string password = cfg["RedisServer"]["Passwd"];
		std::string server_name = cfg["SelfServer"]["Name"];
		int redis_int = std::stoi(redisport);

		if (RedisMjr::GetInstance()->Connect(redishost, redis_int, password, 5)) {
			std::cout << "Redis connected correctly at " << redishost << ":" << redisport << std::endl;
		}
		else {
			std::cout << "Redis cannot connect" << std::endl;
		}

		InitServerStatus();

		// 1. Start gRPC Server
		std::string server_address(cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"]);
		ChatServiceImpl Service;
		grpc::ServerBuilder builder;
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&Service);
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "gRPC built and running on " << server_address << std::endl;

		// 2. Launch gRPC Worker Thread
		std::thread server_thread([&server]() {
			if (server) {
				server->Wait();
			}
			});

		// 🛡️ Guard: Guarantees server_thread.join() runs during stack unwinding / exceptions
		struct ThreadJoiner {
			std::thread& t;
			~ThreadJoiner() {
				if (t.joinable()) {
					t.join();
				}
			}
		} thread_guard{ server_thread };

		// 3. Setup Boost.Asio Context and Signal Handling
		boost::asio::io_context ioc;
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

		signals.async_wait([&ioc, pool, &server](auto, auto) {
			std::cout << "Shutdown signal received..." << std::endl;
			ioc.stop();
			pool->close();
			if (server) {
				server->Shutdown();
			}
			});

		auto port_str = cfg["SelfServer"]["Port"];
		uint16_t port = static_cast<uint16_t>(std::stoul(port_str));

		Cserver s(ioc, port);

		// 4. Block on Asio Event Loop
		ioc.run();

		// 5. Cleanup AFTER ioc.run() stops (When server shuts down)
		RedisMjr::GetInstance()->HDel(LOGIN_COUNT, server_name);
		RedisMjr::GetInstance()->Close();

		std::cout << "ChatServer shut down cleanly." << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "Exception in main: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}