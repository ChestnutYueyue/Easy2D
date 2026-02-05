#include <easy2d/core/e2dtool.h>
#include <easy2d/node/e2dnode.h>
#include <easy2d/base/e2dobjectpool.h>
#include <atomic>
#include <map>

namespace easy2d
{
	class TimerEntity
		: public Object
	{
	public:
		TimerEntity(
			const easy2d::Function<void()>& func,
			const easy2d::String& name,
			float interval,
			int updateTimes
		)
			: running(true)
			, removed(false)
			, runTimes(0)
			, totalTimes(updateTimes)
			, interval(std::max(interval, 0.0f))
			, lastTime(easy2d::Time::getTotalTime())
			, callback(func)
			, name(name)
			, node(nullptr)
		{
		}

		void update()
		{
			if (callback)
			{
				callback();
			}

			++runTimes;
			lastTime += interval;

			if (runTimes == totalTimes)
			{
				removed = true;
			}
		}

		bool isReady() const
		{
			if (this->running)
			{
				if (this->interval == 0)
					return true;

				if ((easy2d::Time::getTotalTime() - this->lastTime) >= this->interval)
					return true;
			}
			return false;
		}

		void reset()
		{
			running = true;
			removed = false;
			runTimes = 0;
			lastTime = easy2d::Time::getTotalTime();
			node = nullptr;
		}

	public:
		bool	running;
		bool	removed;
		int		runTimes;
		int		totalTimes;
		float	interval;
		float	lastTime;
		Node*	node;
		easy2d::String name;
		easy2d::Function<void()> callback;
	};
}

static std::atomic<size_t> s_vTimerId = 0;
static std::map<size_t, easy2d::TimerEntity*> s_vTimers;
// 名称到定时器 ID 的哈希索引，用于 O(1) 查找
static std::map<easy2d::String, std::vector<size_t>> s_nameIndex;


size_t easy2d::Timer::add(const Function<void()>& func, float interval, int updateTimes, const String& name)
{
	auto pool = ObjectPoolManager::getInstance().getPool<TimerEntity>();
	auto timer = pool->acquire(func, name, interval, updateTimes);
	GC::retain(timer);

	const auto id = s_vTimerId++;
	s_vTimers.insert(std::make_pair(id, timer));
	
	if (!name.empty())
	{
		s_nameIndex[name].push_back(id);
	}
	
	return id;
}

size_t easy2d::Timer::runDelayed(float delay, const Function<void()>& func)
{
	return Timer::add(func, delay, 1, "");
}

void easy2d::Timer::bind(size_t id, Node* node)
{
	auto iter = s_vTimers.find(id);
	if (iter != s_vTimers.end())
	{
		iter->second->node = node;
	}
}

void easy2d::Timer::removeBoundWith(Node* node)
{
	for (const auto& pair : s_vTimers)
	{
		auto timer = pair.second;
		if (timer->node == node)
		{
			timer->removed = true;
		}
	}
}

void easy2d::Timer::start(size_t id)
{
	auto iter = s_vTimers.find(id);
	if (iter != s_vTimers.end())
	{
		iter->second->running = true;
	}
}

void easy2d::Timer::stop(size_t id)
{
	auto iter = s_vTimers.find(id);
	if (iter != s_vTimers.end())
	{
		iter->second->running = false;
	}
}

void easy2d::Timer::remove(size_t id)
{
	auto iter = s_vTimers.find(id);
	if (iter != s_vTimers.end())
	{
		iter->second->removed = true;
	}
}

// 辅助函数：从名称索引中移除指定的定时器 ID
static void removeIdFromNameIndex(const easy2d::String& name, size_t id)
{
	if (name.empty()) return;
	
	auto indexIter = s_nameIndex.find(name);
	if (indexIter != s_nameIndex.end())
	{
		auto& idList = indexIter->second;
		// 使用双指针交换删除策略移除 ID
		size_t writeIdx = 0;
		for (size_t readIdx = 0; readIdx < idList.size(); ++readIdx)
		{
			if (idList[readIdx] != id)
			{
				if (writeIdx != readIdx)
				{
					idList[writeIdx] = idList[readIdx];
				}
				++writeIdx;
			}
		}
		if (writeIdx < idList.size())
		{
			idList.erase(idList.begin() + writeIdx, idList.end());
		}
		// 如果该名称下没有定时器了，删除该索引条目
		if (idList.empty())
		{
			s_nameIndex.erase(indexIter);
		}
	}
}

void easy2d::Timer::start(const String& name)
{
	// 使用哈希索引进行 O(1) 查找，替代 O(n) 线性查找
	auto indexIter = s_nameIndex.find(name);
	if (indexIter != s_nameIndex.end())
	{
		for (size_t id : indexIter->second)
		{
			auto timerIter = s_vTimers.find(id);
			if (timerIter != s_vTimers.end())
			{
				timerIter->second->running = true;
			}
		}
	}
}

void easy2d::Timer::stop(const String& name)
{
	// 使用哈希索引进行 O(1) 查找，替代 O(n) 线性查找
	auto indexIter = s_nameIndex.find(name);
	if (indexIter != s_nameIndex.end())
	{
		for (size_t id : indexIter->second)
		{
			auto timerIter = s_vTimers.find(id);
			if (timerIter != s_vTimers.end())
			{
				timerIter->second->running = false;
			}
		}
	}
}

void easy2d::Timer::remove(const String& name)
{
	// 使用哈希索引进行 O(1) 查找，替代 O(n) 线性查找
	auto indexIter = s_nameIndex.find(name);
	if (indexIter != s_nameIndex.end())
	{
		for (size_t id : indexIter->second)
		{
			auto timerIter = s_vTimers.find(id);
			if (timerIter != s_vTimers.end())
			{
				timerIter->second->removed = true;
			}
		}
	}
}

void easy2d::Timer::stopAll()
{
	for (const auto& pair : s_vTimers)
	{
		auto timer = pair.second;
		timer->running = false;
	}
}

void easy2d::Timer::startAll()
{
	for (const auto& pair : s_vTimers)
	{
		auto timer = pair.second;
		timer->running = true;
	}
}

void easy2d::Timer::removeAll()
{
	for (const auto& pair : s_vTimers)
	{
		auto timer = pair.second;
		timer->removed = true;
	}
}

void easy2d::Timer::__update()
{
	if (s_vTimers.empty() || Game::isPaused())
		return;

	auto pool = ObjectPoolManager::getInstance().getPool<TimerEntity>();

	for (auto iter = s_vTimers.begin(); iter != s_vTimers.end();)
	{
		auto timer = iter->second;
		if (timer->removed)
		{
			removeIdFromNameIndex(timer->name, iter->first);
			GC::release(timer);
			pool->release(timer);
			iter = s_vTimers.erase(iter);
		}
		else
		{
			if (timer->isReady())
			{
				timer->update();
			}
			++iter;
		}
	}
}

void easy2d::Timer::__resetAll()
{
	for (const auto& pair : s_vTimers)
	{
		auto timer = pair.second;
		timer->lastTime = Time::getTotalTime();
	}
}

void easy2d::Timer::__uninit()
{
	auto pool = ObjectPoolManager::getInstance().getPool<TimerEntity>();
	
	for (const auto& pair : s_vTimers)
	{
		auto timer = pair.second;
		GC::release(timer);
		pool->release(timer);
	}
	s_vTimers.clear();
	s_nameIndex.clear();
}
