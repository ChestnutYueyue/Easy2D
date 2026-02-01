#include <easy2d/e2dnode.h>
#include <easy2d/e2dmanager.h>
#include <easy2d/e2daction.h>
#include <easy2d/e2dtool.h>
#include <easy2d/e2dbase.h>
#include <easy2d/GLRenderer.h>
#include <algorithm> // std::sort

// 默认中心点位置
static float s_fDefaultAnchorX = 0;
static float s_fDefaultAnchorY = 0;

easy2d::Node::Node()
	: _children()
	, _listeners()
	, _transform()
	, _inverseTransform()
	, _parentScene(nullptr)
	, _parent(nullptr)
	, _body(nullptr)
	, _pos()
	, _size()
	, _scale(1.0f, 1.0f)
	, _skewAngle()
	, _anchor(s_fDefaultAnchorX, s_fDefaultAnchorY)
	, _rotation(0)
	, _displayOpacity(1.0f)
	, _realOpacity(1.0f)
	, _order(0)
	, _flags(0)
{
	// 使用位域初始化标志位
	_visible = true;
	_autoUpdate = true;
	_needSort = false;
	_showBodyShape = false;
	_removed = false;
	_dirtyTransform = false;
	_dirtyInverseTransform = false;
}

void easy2d::Node::reserveChildren(size_t capacity)
{
	_children.reserve(capacity);
}

easy2d::Node::~Node()
{
	__clearListeners();
	Timer::removeBoundWith(this);
	ActionManager::__removeAllBoundWith(this);

	for (auto child : _children)
	{
		child->_parent = nullptr;
		GC::release(child);
	}

	GC::release(_body);
}

void easy2d::Node::_update()
{
	// 更新转换矩阵
	_updateTransform();

	if (_children.empty())
	{
		if (_autoUpdate && !Game::isPaused())
		{
			this->onUpdate();
			__updateListeners();
		}
	}
	else
	{
		// 子节点排序
		_sortChildren();

		// 遍历子节点
		size_t size = _children.size();
		size_t cursor = 0;
		bool hasRemoved = false;
		for (; cursor < size; ++cursor)
		{
			auto child = _children[cursor];
			if (child->getOrder() >= 0)
				break;
			if (child->_removed)
			{
				hasRemoved = true;
				continue;
			}
			child->_update();
		}

		if (_autoUpdate && !Game::isPaused())
		{
			this->onUpdate();
			__updateListeners();
		}

		size = _children.size();
		for (; cursor < size; ++cursor)
		{
			auto child = _children[cursor];
			if (child->_removed)
			{
				hasRemoved = true;
				continue;
			}
			child->_update();
		}

		if (hasRemoved)
		{
			size_t firstRemoved = 0;
			size = _children.size();
			for (size_t i = 0; i < size; ++i)
			{
				auto child = _children[i];
				if (child->_removed)
				{
					child->__clearParents();
					child->release();
					continue;
				}
				_children[firstRemoved] = child;
				++firstRemoved;
			}
			if (firstRemoved < size)
				_children.erase(_children.begin() + firstRemoved, _children.end());
		}
	}
}

void easy2d::Node::_render()
{
	if (!_visible)
	{
		return;
	}

	// 更新转换矩阵
	_updateTransform();

	if (_children.empty())
	{
		// 设置OpenGL模型矩阵
		GLRenderer* glRenderer = Renderer::getGLRenderer();
		if (glRenderer)
		{
			glRenderer->setModelMatrix(_transform);
		}
		// 渲染自身
		this->onRender();
	}
	else
	{
		// 子节点排序
		_sortChildren();

		size_t size = _children.size();
		size_t i;
		for (i = 0; i < size; ++i)
		{
			auto child = _children[i];
			// 访问 Order 小于零的节点
			if (child->getOrder() >= 0)
				break;
			child->_render();
		}

		// 设置OpenGL模型矩阵
		GLRenderer* glRenderer = Renderer::getGLRenderer();
		if (glRenderer)
		{
			glRenderer->setModelMatrix(_transform);
		}
		// 渲染自身
		this->onRender();

		// 访问剩余节点
		for (; i < size; ++i)
			_children[i]->_render();
	}
}

void easy2d::Node::_renderBodyShape()
{
	size_t size = _children.size();
	size_t i;
	for (i = 0; i < size; ++i)
	{
		auto child = _children[i];
		if (child->getOrder() < 0)
		{
			child->_renderBodyShape();
		}
		else
		{
			break;
		}
	}

	if (_body && _showBodyShape)
	{
		GLRenderer* glRenderer = Renderer::getGLRenderer();
		if (glRenderer)
		{
			glRenderer->setModelMatrix(_transform);
			
			// 使用红色绘制物理形状轮廓
			Rect bounds = _body->getBoundingBox();
			glRenderer->drawRectOutline(bounds, Color(255, 0, 0, 128), 2.0f);
		}
	}

	for (; i < size; ++i)
		_children[i]->_renderBodyShape();
}

void easy2d::Node::_updateTransform() const
{
	if (!_dirtyTransform)
		return;

	// 标志已执行过变换
	_dirtyTransform = false;
	_dirtyInverseTransform = true;

	_transform = Matrix32::scaling(_scale.x, _scale.y)
		* Matrix32::skewing(_skewAngle.x, _skewAngle.y)
		* Matrix32::rotation(_rotation)
		* Matrix32::translation(_pos.x, _pos.y);

	_transform.translate(-_size.width * _anchor.x, -_size.height * _anchor.y);

	if (_parent)
	{
		_transform = _transform * _parent->_transform;
	}

	// 通知子节点进行转换
	for (auto& child : _children)
	{
		child->_dirtyTransform = true;
	}
}

void easy2d::Node::_updateInverseTransform() const
{
	_updateTransform();
	if (_dirtyInverseTransform)
	{
		_inverseTransform = Matrix32::invert(_transform);
		_dirtyInverseTransform = false;
	}
}

void easy2d::Node::_sortChildren()
{
	if (!_needSort)
		return;

	// 优化：对于少量子节点或已接近有序的数组，使用插入排序
	// 对于一般情况，使用std::sort（内省排序）
	const size_t n = _children.size();
	
	if (n <= 1)
	{
		_needSort = false;
		return;
	}
	
	// 小数组使用插入排序（缓存友好，常数小）
	if (n <= 16)
	{
		for (size_t i = 1; i < n; ++i)
		{
			Node* key = _children[i];
			int keyOrder = key->getOrder();
			size_t j = i;
			
			while (j > 0 && _children[j - 1]->getOrder() > keyOrder)
			{
				_children[j] = _children[j - 1];
				--j;
			}
			_children[j] = key;
		}
	}
	else
	{
		// 大数组使用标准排序
		std::sort(
			std::begin(_children),
			std::end(_children),
			[](Node* n1, Node* n2) { return n1->getOrder() < n2->getOrder(); }
		);
	}

	_needSort = false;
}

void easy2d::Node::_updateOpacity()
{
	if (_parent)
	{
		_displayOpacity = _realOpacity * _parent->_displayOpacity;
	}
	for (auto child : _children)
	{
		child->_updateOpacity();
	}
}

bool easy2d::Node::isVisible() const
{
	return _visible;
}

float easy2d::Node::getPosX() const
{
	return _pos.x;
}

float easy2d::Node::getPosY() const
{
	return _pos.y;
}

easy2d::Point easy2d::Node::getPos() const
{
	return _pos;
}

float easy2d::Node::getWidth() const
{
	return _size.width * _scale.x;
}

float easy2d::Node::getHeight() const
{
	return _size.height * _scale.y;
}

float easy2d::Node::getRealWidth() const
{
	return _size.width;
}

float easy2d::Node::getRealHeight() const
{
	return _size.height;
}

easy2d::Size easy2d::Node::getRealSize() const
{
	return Size(_size.width, _size.height);
}

float easy2d::Node::getAnchorX() const
{
	return _anchor.x;
}

float easy2d::Node::getAnchorY() const
{
	return _anchor.y;
}

easy2d::Size easy2d::Node::getSize() const
{
	return Size(getWidth(), getHeight());
}

float easy2d::Node::getScaleX() const
{
	return _scale.x;
}

float easy2d::Node::getScaleY() const
{
	return _scale.y;
}

float easy2d::Node::getSkewX() const
{
	return _skewAngle.x;
}

float easy2d::Node::getSkewY() const
{
	return _skewAngle.y;
}

float easy2d::Node::getRotation() const
{
	return _rotation;
}

float easy2d::Node::getOpacity() const
{
	return _realOpacity;
}

easy2d::Node::Property easy2d::Node::getProperty() const
{
	Property prop;
	prop.visable = _visible;
	prop.pos = _pos;
	prop.size = _size;
	prop.opacity = _realOpacity;
	prop.anchor = _anchor;
	prop.scale = _scale;
	prop.rotation = _rotation;
	prop.skewAngle = _skewAngle;
	return prop;
}

easy2d::Rect easy2d::Node::getBounds() const
{
	return Rect(Point{}, _size);
}

easy2d::Rect easy2d::Node::getBoundingBox() const
{
	return getTransform().transform(getBounds());
}

int easy2d::Node::getOrder() const
{
	return _order;
}

void easy2d::Node::setOrder(int order)
{
	if (_order != order)
	{
		_order = order;

		if (this->_parent != nullptr)
		{
			this->_parent->_needSort = true;
		}
	}
}

void easy2d::Node::setPosX(float x)
{
	this->setPos(x, _pos.y);
}

void easy2d::Node::setPosY(float y)
{
	this->setPos(_pos.x, y);
}

void easy2d::Node::setPos(const Point& p)
{
	if (_pos != p)
	{
		_pos = p;
		_dirtyTransform = true;
	}
}

void easy2d::Node::setPos(float x, float y)
{
	this->setPos(Point{ x, y });
}

void easy2d::Node::movePosX(float x)
{
	this->movePos(x, 0);
}

void easy2d::Node::movePosY(float y)
{
	this->movePos(0, y);
}

void easy2d::Node::movePos(float x, float y)
{
	this->setPos(_pos.x + x, _pos.y + y);
}

void easy2d::Node::movePos(const Vector2& v)
{
	this->movePos(v.x, v.y);
}

void easy2d::Node::setScaleX(float scaleX)
{
	this->setScale(scaleX, _scale.y);
}

void easy2d::Node::setScaleY(float scaleY)
{
	this->setScale(_scale.x, scaleY);
}

void easy2d::Node::setScale(float scale)
{
	this->setScale(scale, scale);
}

void easy2d::Node::setScale(float scaleX, float scaleY)
{
	if (_scale.x == scaleX && _scale.y == scaleY)
		return;

	_scale.x = float(scaleX);
	_scale.y = float(scaleY);
	_dirtyTransform = true;
}

void easy2d::Node::setSkewX(float angleX)
{
	this->setSkew(angleX, _skewAngle.y);
}

void easy2d::Node::setSkewY(float angleY)
{
	this->setSkew(_skewAngle.x, angleY);
}

void easy2d::Node::setSkew(float angleX, float angleY)
{
	if (_skewAngle.x == angleX && _skewAngle.y == angleY)
		return;

	_skewAngle.x = float(angleX);
	_skewAngle.y = float(angleY);
	_dirtyTransform = true;
}

void easy2d::Node::setRotation(float angle)
{
	if (_rotation == angle)
		return;

	_rotation = float(angle);
	_dirtyTransform = true;
}

void easy2d::Node::setOpacity(float opacity)
{
	if (_realOpacity == opacity)
		return;

	_displayOpacity = _realOpacity = std::max(std::min(float(opacity), 1.0f), 0.0f);
	// 更新节点透明度
	_updateOpacity();
}

void easy2d::Node::setAnchorX(float anchorX)
{
	this->setAnchor(anchorX, _anchor.y);
}

void easy2d::Node::setAnchorY(float anchorY)
{
	this->setAnchor(_anchor.x, anchorY);
}

void easy2d::Node::setAnchor(float anchorX, float anchorY)
{
	if (_anchor.x == anchorX && _anchor.y == anchorY)
		return;

	_anchor.x = std::max(std::min(float(anchorX), 1.0f), 0.0f);
	_anchor.y = std::max(std::min(float(anchorY), 1.0f), 0.0f);
	_dirtyTransform = true;
}

void easy2d::Node::setWidth(float width)
{
	this->setSize(width, _size.height);
}

void easy2d::Node::setHeight(float height)
{
	this->setSize(_size.width, height);
}

void easy2d::Node::setSize(float width, float height)
{
	if (_size.width == width && _size.height == height)
		return;

	_size.width = float(width);
	_size.height = float(height);
	_dirtyTransform = true;
}

void easy2d::Node::setSize(Size size)
{
	this->setSize(size.width, size.height);
}

void easy2d::Node::setProperty(Property prop)
{
	this->setVisible(prop.visable);
	this->setPos(prop.pos);
	this->setSize(prop.size);
	this->setOpacity(prop.opacity);
	this->setAnchor(prop.anchor.x, prop.anchor.y);
	this->setScale(prop.scale.x, prop.scale.y);
	this->setRotation(prop.rotation);
	this->setSkew(prop.skewAngle.x, prop.skewAngle.y);
}

void easy2d::Node::addChild(Node* child)
{
	addChild(child, child->getOrder());
}

void easy2d::Node::addChild(Node* child, int order)
{
	if (child == nullptr) E2D_WARNING("Node::addChild NULL pointer exception.");

	if (child)
	{
		if (child->_parent != nullptr)
		{
			E2D_WARNING("节点已有父节点, 不能再添加到其他节点");
			return;
		}

		for (Node* parent = this; parent != nullptr; parent = parent->getParent())
		{
			if (child == parent)
			{
				E2D_WARNING("一个节点不能同时是另一个节点的父节点和子节点");
				return;
			}
		}

		_children.push_back(child);

		child->setOrder(order);

		child->retain();

		child->_parent = this;

		if (this->_parentScene)
		{
			child->_setParentScene(this->_parentScene);
		}

		// 更新子节点透明度
		child->_updateOpacity();
		// 更新节点转换
		child->_dirtyTransform = true;
		// 更新子节点排序
		_needSort = true;
	}
}

void easy2d::Node::addChildren(std::initializer_list<Node*> nodes)
{
	for (auto node : nodes)
	{
		this->addChild(node, node->getOrder());
	}
}

void easy2d::Node::addChildren(std::initializer_list<Node*> nodes, int order)
{
	for (auto node : nodes)
	{
		this->addChild(node, order);
	}
}

easy2d::Matrix32 easy2d::Node::getTransform() const
{
	_updateTransform();
	return _transform;
}

easy2d::Matrix32 easy2d::Node::getInverseTransform() const
{
	_updateInverseTransform();
	return _inverseTransform;
}

easy2d::Node* easy2d::Node::getParent() const
{
	return _parent;
}

easy2d::Scene* easy2d::Node::getParentScene() const
{
	return _parentScene;
}

bool easy2d::Node::containsPoint(Point const& point) const
{
	if (_size.width == 0.f || _size.height == 0.f)
		return false;

	Point local = getInverseTransform().transform(point);
	return getBounds().containsPoint(local);
}

std::vector<easy2d::Node*> easy2d::Node::getChildren(const String& name) const
{
	std::vector<Node*> vChildren;
	size_t hash = std::hash<String>{}(name);

	for (auto child : _children)
	{
		if (child->isName(name, hash))
		{
			vChildren.push_back(child);
		}
	}
	return vChildren;
}

easy2d::Node* easy2d::Node::getChild(const String& name) const
{
	size_t hash = std::hash<String>{}(name);

	for (auto child : _children)
	{
		if (child->isName(name, hash))
		{
			return child;
		}
	}
	return nullptr;
}

const std::vector<easy2d::Node*>& easy2d::Node::getAllChildren() const
{
	return _children;
}

int easy2d::Node::getChildrenCount() const
{
	return static_cast<int>(_children.size());
}

void easy2d::Node::removeSelfInNextUpdate()
{
	_removed = true;
}

bool easy2d::Node::removeChild(Node* child)
{
	if (child == nullptr) E2D_WARNING("Node::removeChildren NULL pointer exception.");

	if (_children.empty())
	{
		return false;
	}

	if (child)
	{
		auto iter = std::find(_children.begin(), _children.end(), child);
		if (iter != _children.end())
		{
			_children.erase(iter);
			child->__clearParents();
			child->release();
			return true;
		}
	}
	return false;
}

void easy2d::Node::removeChildren(const String& childName)
{
	if (childName.empty()) E2D_WARNING("Invalid Node name.");

	if (_children.empty())
	{
		return;
	}

	// 计算名称 Hash 值
	size_t hash = std::hash<String>{}(childName);
	auto equals = [&](Node* child)
		{
			return child->isName(childName, hash);
		};

	auto last = _children.end();
	auto first = std::find_if(_children.begin(), last, equals);
	if (first != last)
	{
		for (auto i = first; i != last; ++i)
		{
			auto child = *i;
			if (equals(child))
			{
				// 移除子节点
				child->__clearParents();
				child->release();
			}
			else
			{
				*first = std::move(child);
				++first;
			}
		}
	}
	_children.erase(first, last);
}

void easy2d::Node::removeAllChildren()
{
	// 所有节点的引用计数减一
	for (auto child : _children)
	{
		child->__clearParents();
		child->release();
	}
	// 清空储存节点的容器
	_children.clear();
}

void easy2d::Node::__clearParents()
{
	this->_parent = nullptr;
	if (this->_parentScene)
	{
		this->_setParentScene(nullptr);
	}
}

void easy2d::Node::runAction(Action* action)
{
	ActionManager::start(action, this, false);
}

void easy2d::Node::resumeAction(const String& name)
{
	const auto& actions = ActionManager::get(name);
	for (auto action : actions)
	{
		if (action->getTarget() == this)
		{
			action->resume();
		}
	}
}

void easy2d::Node::pauseAction(const String& name)
{
	const auto& actions = ActionManager::get(name);
	for (auto action : actions)
	{
		if (action->getTarget() == this)
		{
			action->pause();
		}
	}
}

void easy2d::Node::stopAction(const String& name)
{
	const auto& actions = ActionManager::get(name);
	for (auto action : actions)
	{
		if (action->getTarget() == this)
		{
			action->stop();
		}
	}
}

void easy2d::Node::setAutoUpdate(bool autoUpdate)
{
	_autoUpdate = autoUpdate;
}

void easy2d::Node::setDefaultAnchor(float defaultAnchorX, float defaultAnchorY)
{
	s_fDefaultAnchorX = std::max(std::min(float(defaultAnchorX), 1.0f), 0.0f);
	s_fDefaultAnchorY = std::max(std::min(float(defaultAnchorY), 1.0f), 0.0f);
}

void easy2d::Node::resumeAllActions()
{
	ActionManager::__resumeAllBoundWith(this);
}

void easy2d::Node::pauseAllActions()
{
	ActionManager::__pauseAllBoundWith(this);
}

void easy2d::Node::stopAllActions()
{
	ActionManager::__stopAllBoundWith(this);
}

void easy2d::Node::setVisible(bool value)
{
	_visible = value;
}

void easy2d::Node::_setParentScene(Scene* scene)
{
	_parentScene = scene;
	for (auto child : _children)
	{
		child->_setParentScene(scene);
	}
}

easy2d::Listener* easy2d::Node::addListener(const Listener::Callback& func, const String& name, bool paused)
{
	auto listener = gcnew Listener(func, name, paused);
	GC::retain(listener);
	_listeners.push_back(listener);
	return listener;
}

void easy2d::Node::addListener(ListenerBase* listener)
{
	if (listener)
	{
		// 使用unordered_set进行O(1)去重检查
		if (_listenerSet.insert(listener).second)
		{
			GC::retain(listener);
			_listeners.push_back(listener);
		}
	}
}

void easy2d::Node::removeListener(ListenerBase* listener)
{
	if (listener)
	{
		// 使用unordered_set进行O(1)查找
		if (_listenerSet.erase(listener))
		{
			// 标记为脏，延迟从vector中移除
			_listenersDirty = true;
			listener->done();
		}
	}
}

void easy2d::Node::stopListener(const String& name)
{
	if (_listeners.empty() || name.empty())
		return;

	for (auto listener : _listeners)
	{
		if (listener->getName() == name)
		{
			listener->stop();
		}
	}
}

void easy2d::Node::startListener(const String& name)
{
	if (_listeners.empty() || name.empty())
		return;

	for (auto listener : _listeners)
	{
		if (listener->getName() == name)
		{
			listener->start();
		}
	}
}

void easy2d::Node::removeListener(const String& name)
{
	if (_listeners.empty() || name.empty())
		return;

	for (auto listener : _listeners)
	{
		if (listener->getName() == name)
		{
			listener->done();
		}
	}
}

void easy2d::Node::stopAllListeners()
{
	for (auto listener : _listeners)
	{
		listener->stop();
	}
}

void easy2d::Node::startAllListeners()
{
	for (auto listener : _listeners)
	{
		listener->start();
	}
}

void easy2d::Node::removeAllListeners()
{
	for (auto listener : _listeners)
	{
		listener->done();
	}
}

void easy2d::Node::__updateListeners()
{
	if (_listeners.empty() && !_listenersDirty)
		return;

	// 如果有延迟删除的监听器，执行清理
	if (_listenersDirty)
	{
		for (auto it = _listeners.begin(); it != _listeners.end(); )
		{
			auto listener = *it;
			if (listener->isDone())
			{
				GC::release(listener);
				_listenerSet.erase(listener);
				it = _listeners.erase(it);
			}
			else
			{
				++it;
			}
		}
		_listenersDirty = false;
	}

	// 清理已停止的监听器
	for (size_t i = 0; i < _listeners.size(); )
	{
		auto listener = _listeners[i];
		if (listener->isDone())
		{
			GC::release(listener);
			_listenerSet.erase(listener);
			_listeners.erase(_listeners.begin() + i);
		}
		else
		{
			++i;
		}
	}

	if (!_listeners.empty())
	{
		SceneManager::__pushDispatcher(this);
	}
}

void easy2d::Node::__dispatchEvent(Event* evt)
{
	for (auto listener : _listeners)
	{
		listener->receive(this, evt);
	}
}

void easy2d::Node::__clearListeners()
{
	for (auto listener : _listeners)
	{
		GC::release(listener);
	}
	_listeners.clear();
	_listenerSet.clear();
	_listenersDirty = false;
}

easy2d::Shape* easy2d::Node::getBodyShape() const
{
	return _body;
}

void easy2d::Node::setBodyShape(Shape* shape)
{
	GC::release(_body);
	_body = shape;
	GC::retain(_body);
}

void easy2d::Node::showBodyShape(bool enabled)
{
	_showBodyShape = enabled;
}

easy2d::BodyRelation easy2d::Node::compareWithBody(Node* other) const
{
	if (!_body || !other->_body)
	{
		return BodyRelation::Disjoint;
	}

	// OpenGL 模式下暂时简化处理
	// TODO: 实现完整的几何体碰撞检测
	// 使用包围盒进行简单的碰撞检测
	Rect bounds1 = _body->getBoundingBox();
	Rect bounds2 = other->_body->getBoundingBox();

	if (bounds1.intersects(bounds2))
	{
		return BodyRelation::Overlap;
	}

	return BodyRelation::Disjoint;
}
