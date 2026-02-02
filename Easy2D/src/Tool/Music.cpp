#include <easy2d/e2dtool.h>
#include <thread>
#include <chrono>

#if !defined(E2D_USE_MCI)

///////////////////////////////////////////////////////////////////////////////////////////
//
// Music with miniaudio
//
///////////////////////////////////////////////////////////////////////////////////////////

#define MINIAUDIO_IMPLEMENTATION
#include <easy2d/miniaudio.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=nullptr; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=nullptr; } }
#endif


namespace
{
	ma_engine* s_pEngine = nullptr;
}


inline bool TraceError(const char* sPrompt)
{
	E2D_WARNING("%s!", sPrompt);
	return false;
}


inline bool TraceError(const char* sPrompt, ma_result result)
{
	E2D_WARNING("%s (%d)", sPrompt, result);
	return false;
}

class easy2d::Music::Media
{
public:
	Media();

	~Media();

	bool open(
		const String& filePath
	);

	bool open(
		int resNameId,
		const String& resType
	);

	bool play(
		int nLoopCount = 0
	);

	void pause();

	void resume();

	void stop();

	void close();

	bool isPlaying() const;

	bool setVolume(
		float volume
	);

private:
	bool _opened;
	mutable bool _playing;
	int _loopCount;
	ma_sound _sound;
	ma_decoder _decoder;
	BYTE* _pResourceData;
	size_t _resourceDataSize;

	bool _initSoundFromDecoder();
};

easy2d::Music::Media::Media()
	: _opened(false)
	, _playing(false)
	, _loopCount(0)
	, _pResourceData(nullptr)
	, _resourceDataSize(0)
{
	memset(&_sound, 0, sizeof(_sound));
	memset(&_decoder, 0, sizeof(_decoder));
}

easy2d::Music::Media::~Media()
{
	close();
}

bool easy2d::Music::Media::_initSoundFromDecoder()
{
	if (!s_pEngine)
	{
		E2D_WARNING("Engine not initialized!");
		return false;
	}

	// 使用 decoder 创建 sound
	ma_result result = ma_sound_init_from_data_source(
		s_pEngine,
		&_decoder,
		MA_SOUND_FLAG_DECODE,
		nullptr,
		&_sound
	);

	if (result != MA_SUCCESS)
	{
		return TraceError("Failed to create sound from decoder", result);
	}

	return true;
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
		E2D_WARNING("Music::open File not found.");
		return false;
	}

	if (!s_pEngine)
	{
		E2D_WARNING("Engine not initialized!");
		return false;
	}

	// 使用 miniaudio 直接从文件创建 sound
	ma_result result = ma_sound_init_from_file(
		s_pEngine,
		actualFilePath.c_str(),
		MA_SOUND_FLAG_DECODE,
		nullptr,
		nullptr,
		&_sound
	);

	if (result != MA_SUCCESS)
	{
		return TraceError("Failed to load sound file", result);
	}

	_opened = true;
	_playing = false;
	return true;
}

bool easy2d::Music::Media::open(int resNameId, const easy2d::String& resType)
{
	HRSRC hResInfo;
	HGLOBAL hResData;
	DWORD dwSize;
	void* pvRes;

	if (_opened)
	{
		E2D_WARNING("Music can be opened only once!");
		return false;
	}

	if (!s_pEngine)
	{
		E2D_WARNING("Engine not initialized!");
		return false;
	}

	if (nullptr == (hResInfo = FindResourceA(HINST_THISCOMPONENT, MAKEINTRESOURCEA(resNameId), resType.c_str())))
		return TraceError("FindResource");

	if (nullptr == (hResData = LoadResource(HINST_THISCOMPONENT, hResInfo)))
		return TraceError("LoadResource");

	if (0 == (dwSize = SizeofResource(HINST_THISCOMPONENT, hResInfo)))
		return TraceError("SizeofResource");

	if (nullptr == (pvRes = LockResource(hResData)))
		return TraceError("LockResource");

	// 复制资源数据到内存中
	_pResourceData = new (std::nothrow) BYTE[dwSize];
	if (!_pResourceData)
	{
		E2D_WARNING("Low memory");
		return false;
	}
	memcpy(_pResourceData, pvRes, dwSize);
	_resourceDataSize = dwSize;

	// 初始化 decoder
	ma_result result = ma_decoder_init_memory(
		_pResourceData,
		_resourceDataSize,
		nullptr,
		&_decoder
	);

	if (result != MA_SUCCESS)
	{
		SAFE_DELETE_ARRAY(_pResourceData);
		return TraceError("Failed to init decoder", result);
	}

	// 从 decoder 创建 sound
	if (!_initSoundFromDecoder())
	{
		ma_decoder_uninit(&_decoder);
		SAFE_DELETE_ARRAY(_pResourceData);
		return false;
	}

	_opened = true;
	_playing = false;
	return true;
}

bool easy2d::Music::Media::play(int nLoopCount)
{
	if (!_opened)
	{
		E2D_WARNING("Music::play Failed: Music must be opened first!");
		return false;
	}

	_loopCount = nLoopCount;

	// 如果正在播放，先停止
	if (_playing)
	{
		stop();
	}

	// 设置循环
	ma_bool32 isLooping = (nLoopCount < 0 || nLoopCount > 1) ? MA_TRUE : MA_FALSE;
	ma_sound_set_looping(&_sound, isLooping);

	// 开始播放
	ma_result result = ma_sound_start(&_sound);
	if (result == MA_SUCCESS)
	{
		_playing = true;
		return true;
	}

	return TraceError("Failed to start sound", result);
}

void easy2d::Music::Media::pause()
{
	if (_opened)
	{
		ma_sound_stop(&_sound);
		_playing = false;
	}
}

void easy2d::Music::Media::resume()
{
	if (_opened)
	{
		ma_sound_start(&_sound);
		_playing = true;
	}
}

void easy2d::Music::Media::stop()
{
	if (_opened)
	{
		ma_sound_stop(&_sound);
		ma_sound_seek_to_pcm_frame(&_sound, 0);
		_playing = false;
	}
}

void easy2d::Music::Media::close()
{
	if (_opened)
	{
		ma_sound_uninit(&_sound);
		memset(&_sound, 0, sizeof(_sound));

		// 如果是从资源加载的，清理 decoder 和资源数据
		if (_decoder.data.memory.pData != nullptr)
		{
			ma_decoder_uninit(&_decoder);
			memset(&_decoder, 0, sizeof(_decoder));
		}

		SAFE_DELETE_ARRAY(_pResourceData);
		_resourceDataSize = 0;

		_opened = false;
		_playing = false;
	}
}

bool easy2d::Music::Media::isPlaying() const
{
	if (_opened)
	{
		// 检查是否还在播放
		if (ma_sound_is_playing(&_sound))
		{
			return true;
		}
		_playing = false;
	}
	return false;
}

bool easy2d::Music::Media::setVolume(float volume)
{
	if (_opened)
	{
		// miniaudio 音量范围是 0.0 到 1.0
		float linearVolume = std::max(0.0f, std::min(1.0f, volume));
		ma_sound_set_volume(&_sound, linearVolume);
		return true;
	}
	return false;
}

bool easy2d::Music::__init()
{
	if (s_pEngine)
	{
		return true; // 已经初始化
	}

	s_pEngine = new (std::nothrow) ma_engine();
	if (!s_pEngine)
	{
		E2D_WARNING("Failed to allocate engine");
		return false;
	}

	ma_result result = ma_engine_init(nullptr, s_pEngine);
	if (result != MA_SUCCESS)
	{
		delete s_pEngine;
		s_pEngine = nullptr;
		return TraceError("Failed to init engine", result);
	}

	return true;
}

void easy2d::Music::__uninit()
{
	if (s_pEngine)
	{
		// 先停止所有正在播放的音频，避免引擎卸载时等待音频线程
		ma_engine_stop(s_pEngine);
		// 短暂等待确保音频线程退出
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		ma_engine_uninit(s_pEngine);
		delete s_pEngine;
		s_pEngine = nullptr;
	}
}

#else

///////////////////////////////////////////////////////////////////////////////////////////
//
// Music with MCI
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <mmsystem.h>

#pragma comment(lib , "winmm.lib")

#define WIN_CLASS_NAME "Easy2DMciCallbackWnd"

namespace
{
	HINSTANCE s_hInstance = nullptr;

	void TraceMCIError(LPCSTR info, MCIERROR error);
}

class easy2d::Music::Media
{
public:
	Media();

	~Media();

	bool open(
		const String& filePath
	);

	bool open(
		int resNameId,
		const String& resType
	);

	bool play(
		int nLoopCount = 0
	);

	void pause();

	void resume();

	void stop();

	void close();

	bool isPlaying() const;

	bool setVolume(
		float volume
	);

	static LRESULT WINAPI MciProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

protected:
	void _sendCommand(int nCommand, DWORD_PTR param1 = 0, DWORD_PTR parma2 = 0);

protected:
	MCIDEVICEID _dev;
	HWND _wnd;
	bool _playing;
	int _repeatTimes;
};

easy2d::Music::Media::Media()
	: _wnd(NULL)
	, _dev(0L)
	, _playing(false)
	, _repeatTimes(0)
{
	_wnd = CreateWindowExA(
		WS_EX_APPWINDOW,
		WIN_CLASS_NAME,
		NULL,
		WS_POPUPWINDOW,
		0, 0, 0, 0,
		NULL,
		NULL,
		s_hInstance,
		NULL);

	if (_wnd)
	{
		SetWindowLongPtrA(_wnd, GWLP_USERDATA, (LONG_PTR)this);
	}
}

easy2d::Music::Media::~Media()
{
	close();
	DestroyWindow(_wnd);
}

bool easy2d::Music::Media::open(const String& pFileName)
{
	if (pFileName.empty())
		return false;

	close();

	MCI_OPEN_PARMSA mciOpen = { 0 };
	mciOpen.lpstrDeviceType = 0;
	mciOpen.lpstrElementName = pFileName.c_str();

	MCIERROR mciError = mciSendCommandA(
		0,
		MCI_OPEN,
		MCI_OPEN_ELEMENT,
		reinterpret_cast<DWORD_PTR>(&mciOpen)
	);

	if (mciError == 0)
	{
		_dev = mciOpen.wDeviceID;
		_playing = false;
		return true;
	}

	TraceMCIError("Music::open failed!", mciError);
	return false;
}

bool easy2d::Music::Media::open(int resNameId, const String& resType)
{
	E2D_ERROR("Music::open failed! Play sound from memory is not supported when use E2D_WIN7 macro");
	return false;
}

bool easy2d::Music::Media::play(int nLoopCount)
{
	if (!_dev)
	{
		return false;
	}

	MCI_PLAY_PARMS mciPlay = { 0 };
	mciPlay.dwCallback = reinterpret_cast<DWORD_PTR>(_wnd);

	// 播放声音
	MCIERROR mciError = mciSendCommandA(
		_dev,
		MCI_PLAY,
		MCI_FROM | MCI_NOTIFY,
		reinterpret_cast<DWORD_PTR>(&mciPlay)
	);

	if (!mciError)
	{
		_playing = true;
		_repeatTimes = nLoopCount;
		return true;
	}
	return false;
}

void easy2d::Music::Media::close()
{
	if (_playing)
	{
		stop();
	}

	if (_dev)
	{
		_sendCommand(MCI_CLOSE);
	}

	_dev = 0;
	_playing = false;
}

void easy2d::Music::Media::pause()
{
	_sendCommand(MCI_PAUSE);
	_playing = false;
}

void easy2d::Music::Media::resume()
{
	_sendCommand(MCI_RESUME);
	_playing = true;
}

void easy2d::Music::Media::stop()
{
	_sendCommand(MCI_STOP);
	_playing = false;
}

bool easy2d::Music::Media::isPlaying() const
{
	return _playing;
}

bool easy2d::Music::Media::setVolume(float volume)
{
	// NOT SUPPORTED
	return false;
}

void easy2d::Music::Media::_sendCommand(int nCommand, DWORD_PTR param1, DWORD_PTR parma2)
{
	// 空设备时忽略这次操作
	if (!_dev)
	{
		return;
	}
	// 向当前设备发送操作
	mciSendCommandA(_dev, nCommand, param1, parma2);
}

LRESULT WINAPI easy2d::Music::Media::MciProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	easy2d::Music::Media* pMusic = NULL;

	if (Msg == MM_MCINOTIFY
		&& wParam == MCI_NOTIFY_SUCCESSFUL
		&& (pMusic = (easy2d::Music::Media*)GetWindowLongPtr(hWnd, GWLP_USERDATA)))
	{
		if (pMusic->_repeatTimes > 0)
		{
			pMusic->_repeatTimes--;
		}

		if (pMusic->_repeatTimes)
		{
			mciSendCommandA(static_cast<MCIDEVICEID>(lParam), MCI_SEEK, MCI_SEEK_TO_START, 0);

			MCI_PLAY_PARMS mciPlay = { 0 };
			mciPlay.dwCallback = reinterpret_cast<DWORD_PTR>(hWnd);
			mciSendCommandA(static_cast<MCIDEVICEID>(lParam), MCI_PLAY, MCI_NOTIFY, reinterpret_cast<DWORD_PTR>(&mciPlay));
		}
		else
		{
			pMusic->_playing = false;
			return 0;
		}
	}
	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

bool easy2d::Music::__init()
{
	s_hInstance = HINST_THISCOMPONENT;

	WNDCLASSA wc = { 0 };
	wc.style = 0;
	wc.lpfnWndProc = Media::MciProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = s_hInstance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WIN_CLASS_NAME;

	if (!RegisterClassA(&wc) && 1410 != GetLastError())
	{
		return false;
	}
	return true;
}

void easy2d::Music::__uninit()
{
}

namespace
{
	void TraceMCIError(LPCSTR info, MCIERROR error)
	{
		char errorStr[128] = { 0 };
		if (mciGetErrorStringA(error, errorStr, 128))
		{
			E2D_ERROR("%s: %s (%d)", info, errorStr, error);
		}
		else
		{
			E2D_ERROR("%s: Unknown error (%d)", info, error);
		}
	}
}

#endif

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
