#pragma once
#include "const.h"
#include "Singleton.h"

class AsioIOServerPool :public Singleton<AsioIOServerPool> {
	friend class Singleton<AsioIOServerPool>;
public:
	using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using IOService = boost::asio::io_context;
	using Workptr = std::unique_ptr<WorkGuard>;
	~AsioIOServerPool();
	boost::asio::io_context& get_io_context();
	void close();

private:
	std::vector<IOService> _io_contexts;
	std::size_t _next_io_context;
	std::vector<std::thread> _threads;
	std::vector<Workptr> _work_guards;
	AsioIOServerPool(std::size_t size = std::thread::hardware_concurrency());
};