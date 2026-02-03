#include <easy2d/e2dobjectpool.h>

namespace easy2d
{

ObjectPoolManager::ObjectPoolManager()
{
}

ObjectPoolManager::~ObjectPoolManager()
{
	clearAll();
}

ObjectPoolManager& ObjectPoolManager::getInstance()
{
	static ObjectPoolManager instance;
	return instance;
}

void ObjectPoolManager::clearAll()
{
	std::lock_guard<std::mutex> lock(_poolsMutex);
	_pools.clear();
}

size_t ObjectPoolManager::getTotalAvailableCount() const
{
	std::lock_guard<std::mutex> lock(_poolsMutex);
	size_t total = 0;
	for (const auto& pair : _pools)
	{
		if (auto pool = std::static_pointer_cast<ObjectPool<Object>>(pair.second))
		{
			total += pool->getAvailableCount();
		}
	}
	return total;
}

size_t ObjectPoolManager::getTotalInUseCount() const
{
	std::lock_guard<std::mutex> lock(_poolsMutex);
	size_t total = 0;
	for (const auto& pair : _pools)
	{
		if (auto pool = std::static_pointer_cast<ObjectPool<Object>>(pair.second))
		{
			total += pool->getInUseCount();
		}
	}
	return total;
}

}
