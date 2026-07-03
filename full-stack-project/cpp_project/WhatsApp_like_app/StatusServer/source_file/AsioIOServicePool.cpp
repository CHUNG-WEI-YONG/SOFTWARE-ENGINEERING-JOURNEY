#include "AsioIOServicePool.h"
#include <iostream>
using namespace std;
AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : _ioServices(size),
    _nextIOService(0)
{
    for (std::size_t i = 0; i < size; ++i) {

        auto io = std::make_shared<IO_context>();

        _works.emplace_back(
            boost::asio::make_work_guard(*io)
        );

        _ioServices[i] = io;
    }

    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() {
            _ioServices[i]->run();
            });
    }
}

AsioIOServicePool::~AsioIOServicePool() {
    Stop();
    std::cout << "AsioIOServicePool destruct" << endl;
}
AsioIOServicePool::IO_context& AsioIOServicePool::GetIOService()
{
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {
        _nextIOService = 0;
    }
    return *service;
}

void AsioIOServicePool::Stop()
{
    for (auto& work : _works) {
        work.reset();
    }

    for (auto& io : _ioServices) {
        io->stop();
    }

    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}