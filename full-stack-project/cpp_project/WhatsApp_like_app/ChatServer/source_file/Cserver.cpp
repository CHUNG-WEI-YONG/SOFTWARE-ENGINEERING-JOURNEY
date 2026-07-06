#include "Cserver.h"
#include "AsioIOServerPool.h"
#include "CSession.h"

Cserver::Cserver(boost::asio::io_context& ioc, short port):_port(port),_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(),port)) {
	std::cout << "Server start at port: " << port << std::endl;
	StartAccept();

}

void Cserver::StartAccept() {
	auto& ioc = AsioIOServerPool::GetInstance()->get_io_context();
	shared_ptr<CSession> session = make_shared<CSession>(ioc, this);
	_acceptor.async_accept(session->GetSocket(), [this, session](const boost::system::error_code& ec) {
		if (!ec) {
			// 顺利接驳，执行原有的 HandleAccept 业务逻辑
			this->HandleAccept(session, ec);
		}
		else {
			std::cout << "Accept failed, error: " << ec.message() << std::endl;
		}

		// 3. 🎯 核心回环：无论这次成功还是失败，都必须继续拉起下一次异步监听，接力响应后续客户端！
		this->StartAccept();
		});
	
}

void Cserver::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	if (!error) {
		new_session->Start();
		lock_guard<mutex> lock(_mutex);
		_sessions.insert(make_pair(new_session->get_uuid(), new_session));
	}
	else {
		std::cout << "Session accept wrong error: " << error.message() << std::endl;
	}
	StartAccept();
}

void Cserver::ClearSession(std::string session_id) {
	lock_guard<mutex> lock(_mutex);
	_sessions.erase(session_id);

}

Cserver::~Cserver() {
	std::cout << "Cserver destructor called" << std::endl;
}