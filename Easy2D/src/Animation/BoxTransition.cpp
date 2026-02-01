#include <easy2d/e2dtransition.h>
#include <easy2d/e2dnode.h>

easy2d::BoxTransition::BoxTransition(float duration)
	: Transition(duration)
{
}

void easy2d::BoxTransition::_init(Scene * prev, Scene * next)
{
	Transition::_init(prev, next);
}

void easy2d::BoxTransition::_updateCustom()
{
	if (_delta <= 0.5f)
	{
		// 第一阶段：缩小旧场景
		float scale = 1.0f - _delta * 2.0f; // 从1.0缩小到0
		if (_outScene)
		{
			_outScene->setScale(scale, scale);
			_outScene->setAnchor(0.5f, 0.5f);
			_outScene->setPos(Point(_windowSize.width / 2.0f, _windowSize.height / 2.0f));
		}
	}
	else
	{
		// 第二阶段：放大新场景
		float scale = (_delta - 0.5f) * 2.0f; // 从0放大到1.0
		if (_inScene)
		{
			_inScene->setScale(scale, scale);
			_inScene->setAnchor(0.5f, 0.5f);
			_inScene->setPos(Point(_windowSize.width / 2.0f, _windowSize.height / 2.0f));
			_inScene->setOpacity(1.0f);
		}
		if (_outScene)
		{
			_outScene->setOpacity(0.0f);
		}
		if (_delta >= 1.0f)
		{
			this->_stop();
		}
	}
}

void easy2d::BoxTransition::_reset()
{
	// 重置场景变换
	if (_outScene)
	{
		_outScene->setScale(1.0f, 1.0f);
		_outScene->setOpacity(1.0f);
	}
	if (_inScene)
	{
		_inScene->setScale(1.0f, 1.0f);
		_inScene->setOpacity(1.0f);
	}
}
