#include <easy2d/e2dbase.h>
#include <unordered_set>
#include <vector>
#include <array>

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
	// 分代GC池
	std::vector<easy2d::Object*> s_youngPool;      // 新生代（本帧创建）
	std::vector<easy2d::Object*> s_oldPool;        // 老年代（存活多帧）
	std::unordered_set<easy2d::Object*> s_tracked; // 跟踪所有对象，防止重复添加
	
	// GC配置
	constexpr uint32_t PROMOTE_AGE = 3;            // 晋升到老年代的帧数阈值
	constexpr uint32_t OLD_GC_INTERVAL = 60;       // 老年代清理间隔（帧）
	
	uint32_t s_frameCount = 0;
	bool s_bClearing = false;
	bool s_enableGenerational = true;              // 是否启用分代GC
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
		// 分代清理策略
		
		// 1. 清理新生代（每帧都清理）
		for (auto* pObj : s_youngPool)
		{
			pObj->release();
			s_tracked.erase(pObj);
		}
		s_youngPool.clear();
		
		// 2. 定期清理老年代
		if (++s_frameCount % OLD_GC_INTERVAL == 0)
		{
			for (auto* pObj : s_oldPool)
			{
				pObj->release();
				s_tracked.erase(pObj);
			}
			s_oldPool.clear();
		}
	}
	else
	{
		// 传统清理策略：全部清理
		for (auto* pObj : s_youngPool)
		{
			pObj->release();
		}
		s_youngPool.clear();
		
		for (auto* pObj : s_oldPool)
		{
			pObj->release();
		}
		s_oldPool.clear();
		
		s_tracked.clear();
	}
	
	s_bClearing = false;
}

void easy2d::GC::trace(easy2d::Object * pObject)
{
	if (!pObject)
		return;
	
	// 检查是否已跟踪
	if (!s_tracked.insert(pObject).second)
		return;  // 已存在，跳过
	
	// 添加到新生代
	s_youngPool.push_back(pObject);
}

void easy2d::GC::tracePersistent(Object* pObject)
{
	if (!pObject)
		return;
	
	// 直接添加到老年代，跳过分代晋升过程
	if (s_tracked.insert(pObject).second)
	{
		s_oldPool.push_back(pObject);
	}
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
	
	// 强制清理所有代
	for (auto* pObj : s_youngPool)
	{
		pObj->release();
	}
	s_youngPool.clear();
	
	for (auto* pObj : s_oldPool)
	{
		pObj->release();
	}
	s_oldPool.clear();
	
	s_tracked.clear();
	
	s_bClearing = false;
}
