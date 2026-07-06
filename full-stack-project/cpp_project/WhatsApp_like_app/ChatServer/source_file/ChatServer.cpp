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
		auto cfg = ConfigMgr::Inst();
		auto pool = AsioIOServerPool::GetInstance();
		boost::asio::io_context ioc;
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc,pool](auto, auto) {
			ioc.stop();
			pool->close();
			});
		auto port = cfg["Host"]["Port"];
		Cserver s(ioc, atoi(port.c_str()));
		ioc.run();


	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}
}