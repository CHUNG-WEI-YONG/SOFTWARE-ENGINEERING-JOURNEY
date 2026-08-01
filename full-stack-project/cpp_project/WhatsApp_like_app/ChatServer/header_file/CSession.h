#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <functional>
#include "const.h"

// 🎯 核心修正 1：对于在容器 (std::queue) 中作为模板参数的类，
// 必须直接包含其头文件，而不能仅仅依靠前置声明！

class MsgNode;
class RecvNode;
class SendNode;
//class Cserver;

class Cserver;

class CSession :public std::enable_shared_from_this<CSession> {
public:
	CSession(boost::asio::io_context&, Cserver*);
	std::string GetSessionId();
	void SetUserId(int id);
	int GetUserId();
	void async_read_head(int);
	void Start();
	std::string get_uuid();
	boost::asio::ip::tcp::socket& GetSocket();
	void asyncReadFull(std::size_t max_len, std::function<void(const boost::system::error_code& ec, std::size_t)>handler);
	void asyncReadLen(std::size_t read_len, std::size_t total_len, std::function<void(const boost::system::error_code&, std::size_t)> handler);
	void asyncReadBody(int);
	void Close();
	void Send(std::string msg, short msg_id);
	void HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> self);
	boost::asio::ip::tcp::socket& GetIoContext();

private:
	boost::asio::ip::tcp::socket _socket;
	std::atomic<bool> _b_Stop;
	std::atomic<bool> _head_is_parsed;
	Cserver* _server;
	std::shared_ptr<CSession>Sharedself();

	char _data[MAX_LENGTH];
	std::shared_ptr<MsgNode> _recv_head_node;
	std::shared_ptr<RecvNode> _recv_msg_node;

	int _uid;
	std::string _session_id;

	std::queue<std::shared_ptr<SendNode> > _send_que;
	std::mutex _send_lock;
	std::mutex _session_mtx;

};

class LogicNode {
	friend class LogicSystem;
public:
	LogicNode(shared_ptr<CSession>session, shared_ptr<RecvNode> recvnode) :_session(session), _recvnode(recvnode) {};
private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recvnode;
};