#include "cserver.h"
#include "CSession.h"
#include <iostream>

using boost::asio::ip::tcp;

cserver::cserver(boost::asio::io_context& ioc, uint16_t port)
    : _port(port),
    _ioc(ioc),
    _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "[FileServer CServer] Listening on port: " << _port << std::endl;
}

void cserver::Start() {
    StartAccept();
}

void cserver::ClearSession(const std::string& session_id) {
    std::lock_guard<std::mutex> lock(_mutex);
    _sessions.erase(session_id);
    std::cout << "[FileServer CServer] Session removed: " << session_id
        << ", remaining: " << _sessions.size() << std::endl;
}

cserver::~cserver()
{
    _sessions.clear();
}

void cserver::StartAccept() {
    auto self = shared_from_this();
    _acceptor.async_accept([this, self](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto session = std::make_shared<CSession>(_ioc, std::move(socket), self);

            // 关键修正：记录到 sessions 表中
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _sessions[session->GetSessionId()] = session;
            }

            session->Start();
        }
        else {
            std::cerr << "[FileServer CServer] Accept error: " << ec.message() << std::endl;
        }

        // 继续循环接收下一个连接
        StartAccept();
        });
}



