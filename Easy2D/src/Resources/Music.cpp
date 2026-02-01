#include <easy2d/e2dtool.h>

// 使用 miniaudio 作为音频后端
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=nullptr; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=nullptr; } }
#endif


namespace
{
	// miniaudio 引擎实例
	ma_engine* s_pEngine = nullptr;
	
	// 初始化 miniaudio 引擎
	bool InitEngine()
	{
		if (s_pEngine != nullptr)
		{
			return true;
		}
		
		s_pEngine = new (std::nothrow) ma_engine();
		if (s_pEngine == nullptr)
		{
			E2D_ERROR("Failed to allocate memory for audio engine");
			return false;
		}
		
		ma_result result = ma_engine_init(nullptr, s_pEngine);
		if (result != MA_SUCCESS)
		{
			E2D_ERROR("Failed to initialize audio engine (error: %d)", result);
			SAFE_DELETE(s_pEngine);
			return false;
		}
		
		return true;
	}
	
	// 反初始化 miniaudio 引擎
	void UninitEngine()
	{
		if (s_pEngine != nullptr)
		{
			ma_engine_uninit(s_pEngine);
			SAFE_DELETE(s_pEngine);
		}
	}
	
	// 获取引擎实例
	ma_engine* GetEngine()
	{
		if (s_pEngine == nullptr)
		{
			InitEngine();
		}
		return s_pEngine;
	}
}


/**
 * @brief Music 类的内部实现，使用 miniaudio 管理音频播放
 */
class easy2d::Music::Media
{
public:
	/**
	 * @brief 构造函数
	 */
	Media();

	/**
	 * @brief 析构函数
	 */
	~Media();

	/**
	 * @brief 从文件打开音频
	 * @param filePath 音频文件路径
	 * @return 是否成功打开
	 */
	bool open(
		const String& filePath
	);

	/**
	 * @brief 从资源打开音频
	 * @param resNameId 资源名称ID
	 * @param resType 资源类型
	 * @return 是否成功打开
	 */
	bool open(
		int resNameId,
		const String& resType
	);

	/**
	 * @brief 播放音频
	 * @param nLoopCount 循环次数，0表示不循环，-1表示无限循环
	 * @return 是否成功开始播放
	 */
	bool play(
		int nLoopCount = 0
	);

	/**
	 * @brief 暂停播放
	 */
	void pause();

	/**
	 * @brief 恢复播放
	 */
	void resume();

	/**
	 * @brief 停止播放
	 */
	void stop();

	/**
	 * @brief 关闭音频资源
	 */
	void close();

	/**
	 * @brief 检查是否正在播放
	 * @return 是否正在播放
	 */
	bool isPlaying() const;

	/**
	 * @brief 设置音量
	 * @param volume 音量值，范围通常为 0.0 ~ 1.0
	 * @return 是否设置成功
	 */
	bool setVolume(
		float volume
	);

private:
	/**
	 * @brief 从内存数据加载音频
	 * @param data 音频数据指针
	 * @param dataSize 数据大小
	 * @return 是否加载成功
	 */
	bool _loadFromMemory(
		const void* data,
		size_t dataSize
	);

private:
	bool _opened;
	mutable bool _playing;
	ma_sound* _sound;
	ma_decoder* _decoder;
	void* _memoryData;
	size_t _memoryDataSize;
	String _filePath;
};

easy2d::Music::Media::Media()
	: _opened(false)
	, _playing(false)
	, _sound(nullptr)
	, _decoder(nullptr)
	, _memoryData(nullptr)
	, _memoryDataSize(0)
{
}

easy2d::Music::Media::~Media()
{
	close();
}

bool easy2d::Music::Media::open(const easy2d::String& filePath)
{
	if (_opened)
	{
		E2D_WARNING("Music can be opened only once!");
		return false;
	}

	if (filePath.empty())
	{
		E2D_WARNING("Music::open Invalid file name.");
		return false;
	}

	String actualFilePath = Path::searchForFile(filePath);
	if (actualFilePath.empty())
	{
		E2D_WARNING("Music::open File not found: %s", filePath.c_str());
		return false;
	}

	ma_engine* engine = GetEngine();
	if (engine == nullptr)
	{
		E2D_WARNING("Audio engine not initialized!");
		return false;
	}

	// 分配声音对象
	_sound = new (std::nothrow) ma_sound();
	if (_sound == nullptr)
	{
		E2D_WARNING("Failed to allocate memory for sound");
		return false;
	}

	// 从文件初始化声音，使用 DECODE 标志立即解码，不使用空间音频
	ma_result result = ma_sound_init_from_file(
		engine,
		actualFilePath.c_str(),
		MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr,  // 不指定组
		nullptr,  // 不使用通知回调
		_sound
	);

	if (result != MA_SUCCESS)
	{
		E2D_WARNING("Failed to load sound from file: %s (error: %d)", actualFilePath.c_str(), result);
		SAFE_DELETE(_sound);
		return false;
	}

	_filePath = actualFilePath;
	_opened = true;
	_playing = false;
	return true;
}

bool easy2d::Music::Media::open(int resNameId, const easy2d::String& resType)
{
	if (_opened)
	{
		E2D_WARNING("Music can be opened only once!");
		return false;
	}

	// 从资源加载数据
	HRSRC hResInfo = FindResourceA(HINST_THISCOMPONENT, MAKEINTRESOURCEA(resNameId), resType.c_str());
	if (hResInfo == nullptr)
	{
		E2D_WARNING("Failed to find resource: %d", resNameId);
		return false;
	}

	HGLOBAL hResData = LoadResource(HINST_THISCOMPONENT, hResInfo);
	if (hResData == nullptr)
	{
		E2D_WARNING("Failed to load resource: %d", resNameId);
		return false;
	}

	DWORD dwSize = SizeofResource(HINST_THISCOMPONENT, hResInfo);
	if (dwSize == 0)
	{
		E2D_WARNING("Resource size is zero: %d", resNameId);
		return false;
	}

	void* pvRes = LockResource(hResData);
	if (pvRes == nullptr)
	{
		E2D_WARNING("Failed to lock resource: %d", resNameId);
		return false;
	}

	// 复制资源数据到本地缓冲区
	_memoryData = new (std::nothrow) BYTE[dwSize];
	if (_memoryData == nullptr)
	{
		E2D_WARNING("Failed to allocate memory for resource data");
		return false;
	}
	memcpy(_memoryData, pvRes, dwSize);
	_memoryDataSize = dwSize;

	// 从内存加载音频
	if (!_loadFromMemory(_memoryData, _memoryDataSize))
	{
		delete[] static_cast<BYTE*>(_memoryData);
		_memoryData = nullptr;
		_memoryDataSize = 0;
		return false;
	}

	_opened = true;
	_playing = false;
	return true;
}

bool easy2d::Music::Media::_loadFromMemory(const void* data, size_t dataSize)
{
	ma_engine* engine = GetEngine();
	if (engine == nullptr)
	{
		E2D_WARNING("Audio engine not initialized!");
		return false;
	}

	// 分配解码器
	_decoder = new (std::nothrow) ma_decoder();
	if (_decoder == nullptr)
	{
		E2D_WARNING("Failed to allocate memory for decoder");
		return false;
	}

	// 从内存初始化解码器
	ma_result result = ma_decoder_init_memory(data, dataSize, nullptr, _decoder);
	if (result != MA_SUCCESS)
	{
		E2D_WARNING("Failed to initialize decoder from memory (error: %d)", result);
		SAFE_DELETE(_decoder);
		return false;
	}

	// 分配声音对象
	_sound = new (std::nothrow) ma_sound();
	if (_sound == nullptr)
	{
		E2D_WARNING("Failed to allocate memory for sound");
		ma_decoder_uninit(_decoder);
		SAFE_DELETE(_decoder);
		return false;
	}

	// 从数据源初始化声音
	result = ma_sound_init_from_data_source(
		engine,
		_decoder,
		MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr,  // 不指定组
		_sound
	);

	if (result != MA_SUCCESS)
	{
		E2D_WARNING("Failed to initialize sound from data source (error: %d)", result);
		ma_sound_uninit(_sound);
		SAFE_DELETE(_sound);
		ma_decoder_uninit(_decoder);
		SAFE_DELETE(_decoder);
		return false;
	}

	return true;
}

bool easy2d::Music::Media::play(int nLoopCount)
{
	if (!_opened || _sound == nullptr)
	{
		E2D_WARNING("Music::play Failed: Music must be opened first!");
		return false;
	}

	// 如果正在播放，先停止
	if (_playing)
	{
		stop();
	}

	// 设置循环次数
	// miniaudio 中：0 表示不循环，MA_TRUE 表示无限循环
	// 我们需要映射：nLoopCount = 0 -> 不循环，nLoopCount < 0 -> 无限循环，nLoopCount > 0 -> 循环 nLoopCount 次
	ma_bool32 looping = (nLoopCount != 0) ? MA_TRUE : MA_FALSE;
	ma_sound_set_looping(_sound, looping);

	// 开始播放
	ma_result result = ma_sound_start(_sound);
	if (result != MA_SUCCESS)
	{
		E2D_WARNING("Failed to start sound playback (error: %d)", result);
		return false;
	}

	_playing = true;
	return true;
}

void easy2d::Music::Media::pause()
{
	if (_sound)
	{
		ma_result result = ma_sound_stop(_sound);
		if (result == MA_SUCCESS)
		{
			_playing = false;
		}
	}
}

void easy2d::Music::Media::resume()
{
	if (_sound)
	{
		ma_result result = ma_sound_start(_sound);
		if (result == MA_SUCCESS)
		{
			_playing = true;
		}
	}
}

void easy2d::Music::Media::stop()
{
	if (_sound)
	{
		ma_sound_stop(_sound);
		ma_sound_seek_to_pcm_frame(_sound, 0);
		_playing = false;
	}
}

void easy2d::Music::Media::close()
{
	if (_sound)
	{
		ma_sound_stop(_sound);
		ma_sound_uninit(_sound);
		SAFE_DELETE(_sound);
	}

	if (_decoder)
	{
		ma_decoder_uninit(_decoder);
		SAFE_DELETE(_decoder);
	}

	if (_memoryData)
	{
		delete[] static_cast<BYTE*>(_memoryData);
		_memoryData = nullptr;
		_memoryDataSize = 0;
	}

	_opened = false;
	_playing = false;
}

bool easy2d::Music::Media::isPlaying() const
{
	if (_opened && _sound)
	{
		// 使用 miniaudio 的 is_playing 函数
		_playing = ma_sound_is_playing(_sound);
		return _playing;
	}
	return false;
}

bool easy2d::Music::Media::setVolume(float volume)
{
	if (_sound)
	{
		// miniaudio 音量范围是 0.0 到 1.0，但也可以超过 1.0 来放大
		// 保持与原 API 兼容，允许 -224 到 224 的范围映射
		// 这里简单处理：将输入值归一化到 0.0-1.0 范围
		float normalizedVolume = volume;
		if (normalizedVolume < 0.0f) normalizedVolume = 0.0f;
		if (normalizedVolume > 1.0f) normalizedVolume = 1.0f;
		
		ma_sound_set_volume(_sound, normalizedVolume);
		return true;
	}
	return false;
}


/**
 * @brief 初始化音频系统
 * @return 是否初始化成功
 */
bool easy2d::Music::__init()
{
	return InitEngine();
}

/**
 * @brief 反初始化音频系统
 */
void easy2d::Music::__uninit()
{
	UninitEngine();
}


easy2d::Music::Music()
	: _media(new Music::Media())
{
}

easy2d::Music::Music(const easy2d::String & filePath)
	: Music()
{
	this->open(filePath);
}

easy2d::Music::Music(int resNameId, const String & resType)
	: Music()
{
	this->open(resNameId, resType);
}

easy2d::Music::~Music()
{
	delete _media;
	_media = nullptr;
}

bool easy2d::Music::open(const easy2d::String& filePath)
{
	return _media->open(filePath);
}

bool easy2d::Music::open(int resNameId, const easy2d::String& resType)
{
	return _media->open(resNameId, resType);
}

bool easy2d::Music::play(int nLoopCount)
{
	return _media->play(nLoopCount);
}

void easy2d::Music::pause()
{
	_media->pause();
}

void easy2d::Music::resume()
{
	_media->resume();
}

void easy2d::Music::stop()
{
	_media->stop();
}

void easy2d::Music::close()
{
	_media->close();
}

bool easy2d::Music::isPlaying() const
{
	return _media->isPlaying();
}

bool easy2d::Music::setVolume(float volume)
{
	return _media->setVolume(volume);
}
