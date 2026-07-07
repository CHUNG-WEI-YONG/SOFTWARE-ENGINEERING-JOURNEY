#include "ConfigMgr.h"
#include "Cserver.h"
#include "AsioIOServerPool.h"
#include "LogicSystem.h"
#include <csignal>
#include <mutex>
#include <thread>

bool b_stop = false;
std::mutex mutex_quit;
std::condition_variable cv_quit;



int main() {
	try {
		std::cout << "This is Chatserver" << std::endl;
		auto cfg = ConfigMgr::Inst();
		auto pool = AsioIOServerPool::GetInstance();
		boost::asio::io_context ioc;
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc,pool](auto, auto) {
			ioc.stop();
			pool->close();
			});
		auto port_str = cfg["ChatServer1"]["Port"];
		uint16_t port = static_cast<uint16_t>(std::stoul(port_str));
		Cserver s(ioc, port);
		ioc.run();


	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}
}