#include <easy2d/manager/e2dmanager.h>
#include <easy2d/action/e2daction.h>
#include <easy2d/node/e2dnode.h>
#include <easy2d/base/e2dobjectpool.h>
#include <algorithm>

static std::vector<easy2d::Action*> s_vActions;


void easy2d::ActionManager::__update()
{
	if (s_vActions.empty() || Game::isPaused())
		return;

	size_t writeIndex = 0;
	for (size_t readIndex = 0; readIndex < s_vActions.size(); ++readIndex)
	{
		auto action = s_vActions[readIndex];
		if (action->_isDone())
		{
			action->_target = nullptr;
			ActionPoolHelper::release(action);
		}
		else
		{
			if (action->isRunning())
			{
				action->_update();
			}
			if (writeIndex != readIndex)
			{
				s_vActions[writeIndex] = action;
			}
			++writeIndex;
		}
	}
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
		size_t writeIndex = 0;
		for (size_t readIndex = 0; readIndex < s_vActions.size(); ++readIndex)
		{
			auto a = s_vActions[readIndex];
			if (a->getTarget() == target)
			{
				ActionPoolHelper::release(a);
			}
			else
			{
				if (writeIndex != readIndex)
				{
					s_vActions[writeIndex] = a;
				}
				++writeIndex;
			}
		}
		if (writeIndex < s_vActions.size())
		{
			s_vActions.erase(s_vActions.begin() + writeIndex, s_vActions.end());
		}
	}
}

void easy2d::ActionManager::__uninit()
{
	for (auto action : s_vActions)
	{
		ActionPoolHelper::release(action);
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
