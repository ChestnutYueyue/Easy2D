#include <easy2d/e2dbase.h>
#include <unordered_map>
#include <vector>

//
// gcnew helper
//
easy2d::__gc_helper::GCNewHelper easy2d::__gc_helper::GCNewHelper::instance;

// GC 释放池的实现机制：
// Object 类中的引用计数记录了对象被使用的次数，当计数为 0 时，GC 会自动释放这个对象
// 所有的 Object 对象都应在被使用时（例如 Text 添加到了场景中）
// 调用 retain 函数保证该对象不被删除，并在不再使用时调用 release 函数
//
// 分代GC策略：
// - 新生代：新创建的对象，在本帧结束时清理
// - 老年代：存活多帧的对象，定期清理
// - 这种策略基于"大多数对象都是临时的"这一观察

namespace
{
	enum class Generation : uint8_t
	{
		Young = 0,
		Old = 1,
	};

	struct TrackedInfo
	{
		Generation gen = Generation::Young;
		uint32_t age = 0;
		size_t index = 0;
	};

	std::vector<easy2d::Object*> s_youngPool;
	std::vector<easy2d::Object*> s_oldPool;
	std::unordered_map<easy2d::Object*, TrackedInfo> s_tracked;
	
	// GC配置
	constexpr uint32_t PROMOTE_AGE = 3;
	constexpr uint32_t OLD_GC_INTERVAL = 60;
	
	uint32_t s_frameCount = 0;
	bool s_bClearing = false;
	bool s_enableGenerational = true;

	static void removeAt(std::vector<easy2d::Object*>& pool, size_t index)
	{
		const size_t lastIndex = pool.size() - 1;
		if (index != lastIndex)
		{
			easy2d::Object* moved = pool[lastIndex];
			pool[index] = moved;
			auto it = s_tracked.find(moved);
			if (it != s_tracked.end())
			{
				it->second.index = index;
			}
		}
		pool.pop_back();
	}

	static void untrackUnsafe(easy2d::Object* pObject)
	{
		auto it = s_tracked.find(pObject);
		if (it == s_tracked.end())
			return;

		const Generation gen = it->second.gen;
		const size_t index = it->second.index;
		s_tracked.erase(it);

		if (gen == Generation::Young)
		{
			if (index < s_youngPool.size() && s_youngPool[index] == pObject)
				removeAt(s_youngPool, index);
		}
		else
		{
			if (index < s_oldPool.size() && s_oldPool[index] == pObject)
				removeAt(s_oldPool, index);
		}
	}

	static void collectYoung()
	{
		size_t i = 0;
		while (i < s_youngPool.size())
		{
			easy2d::Object* pObj = s_youngPool[i];
			auto it = s_tracked.find(pObj);
			if (it == s_tracked.end())
			{
				removeAt(s_youngPool, i);
				continue;
			}

			if (pObj->getRefCount() <= 1)
			{
				s_tracked.erase(it);
				removeAt(s_youngPool, i);
				pObj->release();
				continue;
			}

			TrackedInfo& info = it->second;
			++info.age;
			if (info.age >= PROMOTE_AGE)
			{
				info.gen = Generation::Old;
				removeAt(s_youngPool, i);
				info.index = s_oldPool.size();
				s_oldPool.push_back(pObj);
				continue;
			}

			++i;
		}
	}

	static void collectOld()
	{
		size_t i = 0;
		while (i < s_oldPool.size())
		{
			easy2d::Object* pObj = s_oldPool[i];
			auto it = s_tracked.find(pObj);
			if (it == s_tracked.end())
			{
				removeAt(s_oldPool, i);
				continue;
			}

			if (pObj->getRefCount() <= 1)
			{
				s_tracked.erase(it);
				removeAt(s_oldPool, i);
				pObj->release();
				continue;
			}

			++i;
		}
	}
}

bool easy2d::GC::isInPool(Object* pObject)
{
	if (!pObject)
		return false;
	return s_tracked.find(pObject) != s_tracked.end();
}

bool easy2d::GC::isClearing()
{
	return s_bClearing;
}

void easy2d::GC::clear()
{
	s_bClearing = true;
	
	if (s_enableGenerational)
	{
		++s_frameCount;
		collectYoung();
		if (s_frameCount % OLD_GC_INTERVAL == 0)
			collectOld();
	}
	else
	{
		for (auto* pObj : s_youngPool)
			pObj->release();
		for (auto* pObj : s_oldPool)
			pObj->release();

		s_youngPool.clear();
		s_oldPool.clear();
		s_tracked.clear();
	}
	
	s_bClearing = false;
}

void easy2d::GC::trace(easy2d::Object * pObject)
{
	if (!pObject)
		return;
	
	if (s_tracked.find(pObject) != s_tracked.end())
		return;

	TrackedInfo info;
	info.gen = Generation::Young;
	info.age = 0;
	info.index = s_youngPool.size();
	s_tracked.emplace(pObject, info);
	s_youngPool.push_back(pObject);
}

void easy2d::GC::tracePersistent(Object* pObject)
{
	if (!pObject)
		return;
	
	if (s_tracked.find(pObject) != s_tracked.end())
		return;

	TrackedInfo info;
	info.gen = Generation::Old;
	info.age = PROMOTE_AGE;
	info.index = s_oldPool.size();
	s_tracked.emplace(pObject, info);
	s_oldPool.push_back(pObject);
}

size_t easy2d::GC::getPoolSize()
{
	return s_tracked.size();
}

size_t easy2d::GC::getYoungPoolSize()
{
	return s_youngPool.size();
}

size_t easy2d::GC::getOldPoolSize()
{
	return s_oldPool.size();
}

void easy2d::GC::setGenerationalEnabled(bool enabled)
{
	s_enableGenerational = enabled;
}

bool easy2d::GC::isGenerationalEnabled()
{
	return s_enableGenerational;
}

void easy2d::GC::forceFullCollect()
{
	s_bClearing = true;
	
	for (auto* pObj : s_youngPool)
		pObj->release();
	for (auto* pObj : s_oldPool)
		pObj->release();

	s_youngPool.clear();
	s_oldPool.clear();
	s_tracked.clear();
	
	s_bClearing = false;
}

void easy2d::GC::untrace(Object* pObject)
{
	if (!pObject)
		return;
	if (s_bClearing)
		return;
	untrackUnsafe(pObject);
}
