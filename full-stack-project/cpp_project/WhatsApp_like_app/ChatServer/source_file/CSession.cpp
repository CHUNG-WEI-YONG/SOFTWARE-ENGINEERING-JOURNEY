#include "CSession.h"
#include <boost/uuid/uuid.hpp>            // 🎯 让编译器认领 "uuid" 类型
#include <boost/uuid/uuid_generators.hpp> // 🎯 让编译器认领 "random_generator" 发生器
#include <boost/uuid/uuid_io.hpp>
#include "Cserver.h"
#include "const.h"
#include "MsgNode.h"
#include "LogicSystem.h"

CSession::CSession(boost::asio::io_context& ioc, Cserver* server):_server(server),_socket(ioc),_b_Stop(false),_head_is_parsed(false) {
	boost::uuids::uuid u = boost::uuids::random_generator()();
	_session_id = boost::uuids::to_string(u);	
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);

}

void CSession::Start() {
	async_read_head(HEAD_TOTAL_LEN);
}

void CSession::async_read_head(int len) {
	auto self = shared_from_this();
	asyncReadFull(len, [self, this, len](const boost::system::error_code &ec, std::size_t byte_transfer) {
		try{
			if (ec) {
				std::cout << "Read head error: " << ec << std::endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}
			if (byte_transfer < len) {
				std::cout << "Read head error: byte_transfer < len" << std::endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}
			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, byte_transfer);

			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);

			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "Message id is : " << msg_id << std::endl;
			if (msg_id > MAX_LENGTH) {
				std::cout << "Invalid message id: " << msg_id << std::endl;
				_server->ClearSession(_session_id);
				return;
			}

			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			if (msg_len > MAX_LENGTH) {
				std::cout << "Invalid message length: " << msg_len << std::endl;
				_server->ClearSession(_session_id);
				return;
			}
			_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
			asyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
		});

	
}

void CSession::asyncReadFull(std::size_t max_len, std::function<void(const boost::system::error_code& ec, std::size_t)>handler) {
	std::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, max_len, handler);
}

void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len,
	std::function<void(const boost::system::error_code&, std::size_t)> handler) {
	auto self = shared_from_this();
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len), [read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t byte_transfer) {
		if (ec) {
			handler(ec, read_len + byte_transfer);
			return;
		}
		if (read_len + byte_transfer >= total_len) {
			handler(ec, read_len + byte_transfer);
			return;
		}
		self->asyncReadLen(read_len + byte_transfer, total_len, handler);
		});
}

void CSession::asyncReadBody(int total_len) {
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t byte_transfer) {
		try{
			if (ec) {
				std::cout << "Handle read failed, error code is " << ec.message() << std::endl;
				Close();
				_server->ClearSession(_session_id);
				}
			if (byte_transfer < total_len) {
				std::cout << "read length not match, read [" << byte_transfer << "] , total ["
					<< total_len << "]" << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}
			memcpy(_recv_msg_node->_data, _data, byte_transfer);
			_recv_msg_node->_total_len += byte_transfer;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
				
			async_read_head(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
		});
}

std::shared_ptr<CSession>CSession::Sharedself() {
	return shared_from_this();
}

void CSession::Send(std::string msg,short msg_id) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size >= MAX_SENDQUE) {
		std::cout << "Session "<<_session_id<<"failed. " << "Send queue is full, size is " << send_que_size << std::endl;
		return;
	}
	_send_que.push(std::make_shared<SendNode>(msg.c_str(), static_cast<short>(msg.length()), msg_id));
	if (_send_que.size() > 1) {
		return;
	}

	auto& msgNode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgNode->_data, msgNode->_total_len), [this, self = Sharedself()](const boost::system::error_code& ec, std::size_t byte_transfer) {
		this->HandleWrite(ec, self);
		});
}

void CSession::HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> self) {
	try {
		if (!ec) {
			std::unique_lock<std::mutex> lock(_send_lock);
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgNode = _send_que.front();
				lock.unlock();

				boost::asio::async_write(
					_socket,
					boost::asio::buffer(msgNode->_data, msgNode->_total_len),
					[this, self = Sharedself()](const boost::system::error_code& ec, std::size_t byte_transer) {
						self->HandleWrite(ec, self);
					}
				);
			}

		}

	}
	catch (std::exception& e) {
		std::cout << "Exception code is " << e.what() << std::endl;
	}
}

tcp::socket& CSession::GetIoContext() {
	return _socket;
}

std::string CSession::GetSessionId() {
	return _session_id;
}

void CSession::Close() {
	std::lock_guard<std::mutex> lock(_session_mtx);
	_b_Stop = true;
	_socket.close();
}

//LogicNode::LogicNode(shared_ptr<CSession>  session,
//	shared_ptr<RecvNode> recvnode) :_session(session), _recvnode(recvnode) {
//
//}

boost::asio::ip::tcp::socket& CSession::GetSocket() {
	return _socket;
}

std::string CSession::get_uuid() {
	return _session_id;
}