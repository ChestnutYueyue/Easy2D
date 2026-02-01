#include <easy2d/e2dtransition.h>
#include <easy2d/e2dnode.h>

easy2d::FadeTransition::FadeTransition(float duration, bool sequential)
	: Transition(duration)
	, _sequential(sequential)
{
}

void easy2d::FadeTransition::_init(Scene * prev, Scene * next)
{
	Transition::_init(prev, next);
	// 初始化场景透明度
	if (_outScene)
	{
		_outScene->setOpacity(1.0f);
	}
	if (_inScene)
	{
		_inScene->setOpacity(0.0f);
	}
}

void easy2d::FadeTransition::_updateCustom()
{
	if (_sequential)
	{
		if (_delta < 0.5f)
		{
			// 第一阶段：旧场景淡出
			if (_outScene)
			{
				_outScene->setOpacity(1.0f - _delta * 2.0f);
			}
			if (_inScene)
			{
				_inScene->setOpacity(0.0f);
			}
		}
		else
		{
			// 第二阶段：新场景淡入
			if (_outScene)
			{
				_outScene->setOpacity(0.0f);
			}
			if (_inScene)
			{
				_inScene->setOpacity((_delta - 0.5f) * 2.0f);
			}
		}
	}
	else
	{
		// 同时淡入淡出
		if (_outScene)
		{
			_outScene->setOpacity(1.0f - _delta);
		}
		if (_inScene)
		{
			_inScene->setOpacity(_delta);
		}
	}

	if (_delta >= 1.0f)
	{
		this->_stop();
	}
}

void easy2d::FadeTransition::_reset()
{
	// 重置场景透明度
	if (_outScene)
	{
		_outScene->setOpacity(1.0f);
	}
	if (_inScene)
	{
		_inScene->setOpacity(1.0f);
	}
}
