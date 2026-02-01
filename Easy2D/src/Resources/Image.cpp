#include <easy2d/e2dcommon.h>
#include <easy2d/e2dbase.h>
#include <easy2d/e2dtool.h>
#include <unordered_map>
#include <easy2d/GLTexture.h>

namespace
{
	std::unordered_map<easy2d::String, easy2d::Image*> s_mImagesFromFile;
	std::unordered_map<easy2d::Resource, easy2d::Image*> s_mImagesFromResource;
}

namespace easy2d
{
	easy2d::Image::Image(GLTexture* texture)
		: _texture(texture)
	{
	}

	easy2d::Image::~Image()
	{
		// GLTexture由GLTextureCache管理，不需要在这里释放
		_texture = nullptr;
	}

	GLTexture* easy2d::Image::getTexture() const
	{
		return _texture;
	}

	void* easy2d::Image::getBitmap()
	{
		// 已废弃，返回nullptr
		return nullptr;
	}

	void easy2d::Image::resetTexture(GLTexture* texture)
	{
		_texture = texture;
	}

	float easy2d::Image::getWidth() const
	{
		if (_texture)
		{
			return static_cast<float>(_texture->getWidth());
		}
		else
		{
			return 0;
		}
	}

	float easy2d::Image::getHeight() const
	{
		if (_texture)
		{
			return static_cast<float>(_texture->getHeight());
		}
		else
		{
			return 0;
		}
	}

	easy2d::Size easy2d::Image::getSize() const
	{
		if (_texture)
		{
			return _texture->getSize();
		}
		else
		{
			return Size();
		}
	}

	easy2d::Image* easy2d::Image::load(const String& filePath)
	{
		auto iter = s_mImagesFromFile.find(filePath);
		if (iter != s_mImagesFromFile.end())
		{
			return iter->second;
		}

		// 使用GLTextureCache加载纹理
		GLTexture* texture = E2D_GL_TEXTURE_CACHE.getTexture(filePath);
		if (!texture)
		{
			E2D_ERROR("Load image failed! File: %s", filePath.c_str());
			return nullptr;
		}

		Image* image = gcnew Image(texture);
		image->retain();
		s_mImagesFromFile.insert(std::make_pair(filePath, image));

		return image;
	}

	easy2d::Image* easy2d::Image::load(const Resource& res)
	{
		auto iter = s_mImagesFromResource.find(res);
		if (iter != s_mImagesFromResource.end())
		{
			return iter->second;
		}

		// 使用GLTextureCache加载纹理
		GLTexture* texture = E2D_GL_TEXTURE_CACHE.getTexture(res);
		if (!texture)
		{
			E2D_ERROR("Load image failed! Resource ID: %d", res.getId());
			return nullptr;
		}

		Image* image = gcnew Image(texture);
		image->retain();
		s_mImagesFromResource.insert(std::make_pair(res, image));

		return image;
	}

	easy2d::Image* easy2d::Image::load(int resNameId, const String& resType)
	{
		return load(Resource{ resNameId, resType });
	}

	void easy2d::Image::clearCache()
	{
		for (auto pair : s_mImagesFromFile)
		{
			GC::release(pair.second);
		}
		s_mImagesFromFile.clear();

		for (auto pair : s_mImagesFromResource)
		{
			GC::release(pair.second);
		}
		s_mImagesFromResource.clear();
	}

	void easy2d::Image::reloadCache()
	{
		// OpenGL纹理在上下文丢失后需要重新加载
		// GLTextureCache会自动处理重新加载
		E2D_GL_TEXTURE_CACHE.reloadAll();
	}

}
