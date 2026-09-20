#pragma once
#include "const.h"
#include "boost/asio.hpp"
#include <mutex>
#include <map>
#include <memory.h>
#include <string>

class CSession;
using boost::asio::ip::tcp;
class Cserver:public std::enable_shared_from_this<Cserver> {
public:
	Cserver(boost::asio::io_context& ioc, uint16_t port);
	~Cserver();
	void ClearSession(std::string);
	bool CheckSessionId(std::string session_id);
private:
	void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code& error);
	void StartAccept();
	std::mutex _mutex;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	uint16_t _port;
	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;


};