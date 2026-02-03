#include <easy2d/e2dmanager.h>
#include <easy2d/e2daction.h>
#include <easy2d/e2dnode.h>
#include <algorithm>

static std::vector<easy2d::Action*> s_vActions;


void easy2d::ActionManager::__update()
{
	if (s_vActions.empty() || Game::isPaused())
		return;

	// 使用双指针交换删除策略，将复杂度从 O(n²) 降为 O(n)
	size_t writeIndex = 0;
	for (size_t readIndex = 0; readIndex < s_vActions.size(); ++readIndex)
	{
		auto action = s_vActions[readIndex];
		// 获取动作运行状态
		if (action->_isDone())
		{
			action->_target = nullptr;
			action->release();
			// 不保留已完成的动作，跳过写入
		}
		else
		{
			if (action->isRunning())
			{
				// 执行动作
				action->_update();
			}
			// 将未完成的动作移动到 writeIndex 位置
			if (writeIndex != readIndex)
			{
				s_vActions[writeIndex] = action;
			}
			++writeIndex;
		}
	}
	// 删除尾部多余的元素
	if (writeIndex < s_vActions.size())
	{
		s_vActions.erase(s_vActions.begin() + writeIndex, s_vActions.end());
	}
}

void easy2d::ActionManager::__resumeAllBoundWith(Node * target)
{
	if (s_vActions.empty() || target == nullptr)
		return;

	for (auto action : s_vActions)
	{
		if (action->getTarget() == target)
		{
			action->resume();
		}
	}
}

void easy2d::ActionManager::__pauseAllBoundWith(Node * target)
{
	if (s_vActions.empty() || target == nullptr)
		return;

	for (auto action : s_vActions)
	{
		if (action->getTarget() == target)
		{
			action->pause();
		}
	}
}

void easy2d::ActionManager::__stopAllBoundWith(Node * target)
{
	if (s_vActions.empty() || target == nullptr)
		return;

	for (auto action : s_vActions)
	{
		if (action->getTarget() == target)
		{
			action->stop();
		}
	}
}

void easy2d::ActionManager::start(Action * action, Node * target, bool paused)
{
	if (action == nullptr) E2D_WARNING("Action NULL pointer exception!");
	if (target == nullptr) E2D_WARNING("Target node NULL pointer exception!");

	if (action && target)
	{
		if (action->_target == nullptr)
		{
			auto iter = std::find(s_vActions.begin(), s_vActions.end(), action);
			if (iter == s_vActions.end())
			{
				action->_startWithTarget(target);
				action->retain();
				action->_running = !paused;
				s_vActions.push_back(action);
			}
		}
		else
		{
			E2D_WARNING("该 Action 已有执行目标");
		}
	}
}

void easy2d::ActionManager::resume(const String& name)
{
	if (s_vActions.empty() || name.empty())
		return;

	for (auto action : s_vActions)
	{
		if (action->getName() == name)
		{
			action->resume();
		}
	}
}

void easy2d::ActionManager::pause(const String& name)
{
	if (s_vActions.empty() || name.empty())
		return;

	for (auto action : s_vActions)
	{
		if (action->getName() == name)
		{
			action->pause();
		}
	}
}

void easy2d::ActionManager::stop(const String& name)
{
	if (s_vActions.empty() || name.empty())
		return;

	for (auto action : s_vActions)
	{
		if (action->getName() == name)
		{
			action->stop();
		}
	}
}

void easy2d::ActionManager::__removeAllBoundWith(Node * target)
{
	if (target)
	{
		for (size_t i = 0; i < s_vActions.size();)
		{
			auto a = s_vActions[i];
			if (a->getTarget() == target)
			{
				GC::release(a);
				s_vActions.erase(s_vActions.begin() + i);
			}
			else
			{
				++i;
			}
		}
	}
}

void easy2d::ActionManager::__uninit()
{
	for (auto action : s_vActions)
	{
		GC::release(action);
	}
	s_vActions.clear();
}

std::vector<easy2d::Action*> easy2d::ActionManager::get(const String& name)
{
	std::vector<Action*> actions;
	for (auto action : s_vActions)
	{
		if (action->isName(name))
		{
			actions.push_back(action);
		}
	}
	return actions;
}

const std::vector<easy2d::Action*>& easy2d::ActionManager::getAll()
{
	return s_vActions;
}

void easy2d::ActionManager::__resetAll()
{
	for (auto action : s_vActions)
	{
		action->_resetTime();
	}
}
