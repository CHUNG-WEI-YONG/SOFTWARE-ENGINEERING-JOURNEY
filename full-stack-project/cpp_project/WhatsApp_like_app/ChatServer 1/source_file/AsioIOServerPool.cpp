#include "AsioIOServerPool.h"

// 🎯 核心修正 3：完全删掉这里的默认参数后缀，只写类型签名，消除 C2572/C2533 报错！
AsioIOServerPool::AsioIOServerPool(std::size_t size)
	: _io_contexts(size), _work_guards(size), _next_io_context(0) {

	// 🎯 修正类型安全：改用 std::size_t i
	for (std::size_t i = 0; i < size; ++i) {
		_work_guards[i] = std::make_unique<WorkGuard>(_io_contexts[i].get_executor());
	}

	for (std::size_t i = 0; i < size; ++i) {
		_threads.emplace_back([this, i]() {
			_io_contexts[i].run();
			});
	}
}

AsioIOServerPool::~AsioIOServerPool() {
	std::cout << "AsioIOServerPool ended" << std::endl;
}

boost::asio::io_context& AsioIOServerPool::get_io_context() {
	auto& ioc = _io_contexts[_next_io_context++];
	if (_next_io_context >= _io_contexts.size()) {
		_next_io_context = 0;
	}
	return ioc;
}

void AsioIOServerPool::close() {
	// 1. 强行拉响停机警报，让所有的 io_context 事件循环终止
	for (auto& work : _work_guards) {
		if (work) {
			work->get_executor().context().stop();
		}
	}

	// 2. 核心安全修正：必须先 join 挂起等待物理线程完全退出！
	for (auto& t : _threads) {
		if (t.joinable()) {
			t.join();
		}
	}

	// 3. 线程安全着陆后，最后才清空守卫指针内存
	for (auto& work : _work_guards) {
		work.reset();
	}
}

