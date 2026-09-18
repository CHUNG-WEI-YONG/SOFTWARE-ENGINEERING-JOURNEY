#include "const.h"
#include <boost/asio.hpp>
#include "ConfigMgr.h"
#include "RedisMjr.h"
#include <iostream>
#include <thread>
#include <memory>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
#include "Singleton.h"

class CSession;
using boost::asio::ip::tcp;


class cserver :public std::enable_shared_from_this<cserver> {
	friend class CSession;
public:
	cserver(boost::asio::io_context& ioc, uint16_t port);
	void Start();
	void ClearSession(const std::string& session_id);
	~cserver();

private:
	void StartAccept();
	std::mutex _mutex;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	uint16_t _port;
	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;

};