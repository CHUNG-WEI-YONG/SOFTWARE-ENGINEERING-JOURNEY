#include "Cserver.h"
#include "AsioIOServerPool.h"
#include "CSession.h"
#include "UserMgr.h"
#include "LogicSystem.h"
#include "ConfigMgr.h"
#include "RedisMjr.h"

Cserver::Cserver(boost::asio::io_context& ioc, uint16_t port):_port(port),_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(),port)) ,_timer(_ioc,std::chrono::seconds(60)) {
	auto self = shared_from_this();
	std::cout << "Server start at port: " << port << std::endl;
	StartAccept();
	_timer.async_wait([this,self](const boost::system::error_code& ec) {
		self->on_timer(ec);
	});


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
			this->StartAccept();
		}
		// 3. 🎯 核心回环：无论这次成功还是失败，都必须继续拉起下一次异步监听，接力响应后续客户端！
		
		});
	
}

void Cserver::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	std::cout << "✅ [Cserver] Accepted new connection, session id: " << new_session->GetSessionId() << std::endl;
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
	if (_sessions.find(session_id) != _sessions.end()) {
		UserMgr::GetInstance()->RmvUserSession(_sessions[session_id]->GetUserId(), session_id);
	}
	{
		lock_guard<mutex> lock(_mutex);
		_sessions.erase(session_id);
	}

}

bool Cserver::CheckSessionId(std::string session_id)
{
	auto iter = _sessions.find(session_id);
	if (iter != _sessions.end()) {
		return true;
	}
	return false;
}

void Cserver::on_timer(const boost::system::error_code &ec)
{
	if (ec)return;
	std::vector<std::shared_ptr<CSession>> _expired_sessions;
	int session_count = 0;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		time_t now = time(nullptr);
		for (auto iter = _sessions.begin();iter != _sessions.end();++iter) {
			bool is_expired = iter->second->isHeartBeatExpired(now);
			if (is_expired) {
				iter->second->Close();
				_expired_sessions.push_back(iter->second);
			}
			else {
				session_count++;
			}
		}
		
	}

	auto& cfg = ConfigMgr::Inst();
	auto server = cfg["SelfServer"]["Name"];
	auto count_str = std::to_string(session_count);
	RedisMjr::GetInstance()->HSet(LOGIN_COUNT, server, count_str);


	for (auto& session : _expired_sessions) {
		session->DealExceptionSession();
	}

	_timer.expires_after(std::chrono::seconds(60));
	_timer.async_wait([this](const boost::system::error_code& ec) {
		on_timer(ec);
	});


}

Cserver::~Cserver() {
	std::cout << "Cserver destructor called" << std::endl;
	_timer.cancel();
}