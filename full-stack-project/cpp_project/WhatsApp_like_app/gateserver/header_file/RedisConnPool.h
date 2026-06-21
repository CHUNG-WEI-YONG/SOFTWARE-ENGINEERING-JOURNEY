#include "const.h"

class RedisConnPool :public Singleton<RedisConnPool> {
	friend class Singleton<RedisConnPool>;
public:

private:
	RedisConnPool(std::size);

};