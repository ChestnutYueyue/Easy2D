#define NOMINMAX
#include <easy2d/e2dbase.h>
#include <easy2d/e2dtransition.h>
#include <easy2d/e2dnode.h>
#include <easy2d/GLRenderer.h>
#include <algorithm>

easy2d::Transition::Transition(float duration)
	: _end(false)
	, _last(0)
	, _delta(0)
	, _outScene(nullptr)
	, _inScene(nullptr)
{
	_duration = std::max(duration, 0.0f);
}

easy2d::Transition::~Transition()
{
	GC::release(_outScene);
	GC::release(_inScene);
}

bool easy2d::Transition::isDone()
{
	return _end;
}

void easy2d::Transition::_init(Scene * prev, Scene * next)
{
	_last = Time::getTotalTime();
	_outScene = prev;
	_inScene = next;
	if (_outScene) _outScene->retain();
	if (_inScene) _inScene->retain();
	_windowSize = Window::getSize();
}

void easy2d::Transition::_update()
{
	// 计算动作进度
	if (_duration == 0)
	{
		_delta = 1;
	}
	else
	{
		_delta = std::min((Time::getTotalTime() - _last) / _duration, 1.0f);
	}
	this->_updateCustom();
	
	// 更新场景内容
	if (_outScene)
	{
		_outScene->_update();
	}
	if (_inScene)
	{
		_inScene->_update();
	}
}

void easy2d::Transition::_render()
{
	GLRenderer* glRenderer = Renderer::getGLRenderer();
	if (!glRenderer)
	{
		if (_outScene)
		{
			_outScene->_render();
		}
		if (_inScene)
		{
			_inScene->_render();
		}
		return;
	}
	
	// OpenGL 场景过渡效果
	// 使用淡入淡出效果
	if (_outScene)
	{
		glRenderer->setOpacity(1.0f - _delta);
		_outScene->_render();
	}
	
	if (_inScene)
	{
		glRenderer->setOpacity(_delta);
		_inScene->_render();
	}
	
	glRenderer->setOpacity(1.0f);
}

void easy2d::Transition::_stop()
{
	_end = true;
	_reset();
}
