#include "const.h"
class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
public:
    using IO_context = boost::asio::io_context;
    using Work = boost::asio::executor_work_guard<IO_context::executor_type>;

    ~AsioIOServicePool();

    IO_context& GetIOService();
    void Stop();

private:
    friend Singleton<AsioIOServicePool>;

    AsioIOServicePool(std::size_t size = 2);

    std::vector<std::shared_ptr<IO_context>> _ioServices;
    std::vector<Work> _works;
    std::vector<std::thread> _threads;

    std::size_t _nextIOService;
};