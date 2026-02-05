#include <easy2d/base/e2dbase.h>
#include <easy2d/node/e2dnode.h>
#include <easy2d/manager/e2dmanager.h>

easy2d::Scene::Scene()
{
	_setParentScene(this);
}

easy2d::Scene::~Scene()
{
}
