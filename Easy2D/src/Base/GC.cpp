#include <easy2d/base/e2dbase.h>
#include <vector>
#include <unordered_set>

//
// gcnew helper
//
easy2d::__gc_helper::GCNewHelper easy2d::__gc_helper::GCNewHelper::instance;

// GC 释放池的实现机制：
// Object 类中的引用计数记录了对象被使用的次数，当计数为 0 时，GC 会自动释放这个对象
// 所有的 Object 对象都应在被使用时（例如 Text 添加到了场景中）
// 调用 retain 函数保证该对象不被删除，并在不再使用时调用 release 函数

namespace
{
	std::vector<easy2d::Object*> s_vObjectPool;
	// 使用 unordered_set 存储对象指针，实现 O(1) 查找
	std::unordered_set<easy2d::Object*> s_objectSet;
	bool s_bClearing = false;
}

bool easy2d::GC::isInPool(Object* pObject)
{
	if (pObject)
	{
		// 使用 unordered_set 实现 O(1) 查找
		return s_objectSet.find(pObject) != s_objectSet.end();
	}
	return false;
}

bool easy2d::GC::isClearing()
{
	return s_bClearing;
}

void easy2d::GC::clear()
{
	std::vector<Object*> releaseThings;
	releaseThings.swap(s_vObjectPool);
	// 清空 unordered_set
	s_objectSet.clear();

	s_bClearing = true;
	for (auto pObj : releaseThings)
	{
		pObj->release();
	}
	s_bClearing = false;
}

void easy2d::GC::trace(easy2d::Object * pObject)
{
	if (pObject)
	{
		s_vObjectPool.push_back(pObject);
		// 同时添加到 unordered_set
		s_objectSet.insert(pObject);
	}
}
