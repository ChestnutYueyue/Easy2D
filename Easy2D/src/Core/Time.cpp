#include <easy2d/e2dbase.h>
#include <SDL3/SDL.h>


// 游戏开始时间（毫秒）
static Uint64 s_tStart = 0;
// 当前时间（毫秒）
static Uint64 s_tNow = 0;
// 上一帧刷新时间（毫秒）
static Uint64 s_tLast = 0;
// 固定的刷新时间（毫秒）
static Uint64 s_tFixed = 0;
// 每一帧间隔（毫秒）
static Uint64 s_tExceptedInterval = 0;
// SDL时间频率
static Uint64 s_sdlFrequency = 0;


// 将SDL时间转换为毫秒
static inline float ticksToSeconds(Uint64 ticks)
{
	return static_cast<float>(ticks) / static_cast<float>(s_sdlFrequency);
}

static inline Uint64 ticksToMilliseconds(Uint64 ticks)
{
	return (ticks * 1000) / s_sdlFrequency;
}

static inline Uint64 millisecondsToTicks(Uint64 ms)
{
	return (ms * s_sdlFrequency) / 1000;
}


float easy2d::Time::getTotalTime()
{
	return ticksToSeconds(s_tNow - s_tStart);
}

unsigned int easy2d::Time::getTotalTimeMilliseconds()
{
	return static_cast<unsigned int>(ticksToMilliseconds(s_tNow - s_tStart));
}

float easy2d::Time::getDeltaTime()
{
	return ticksToSeconds(s_tNow - s_tLast);
}

unsigned int easy2d::Time::getDeltaTimeMilliseconds()
{
	return static_cast<unsigned int>(ticksToMilliseconds(s_tNow - s_tLast));
}

void easy2d::Time::__init(int expectedFPS)
{
	// 获取SDL时间频率
	s_sdlFrequency = SDL_GetPerformanceFrequency();

	if (expectedFPS > 0)
	{
		s_tExceptedInterval = s_sdlFrequency / expectedFPS;
	}
	else
	{
		s_tExceptedInterval = 0;
	}

	// 获取当前时间
	Uint64 now = SDL_GetPerformanceCounter();
	s_tStart = s_tFixed = s_tLast = s_tNow = now;
}

bool easy2d::Time::__isReady()
{
	return s_tExceptedInterval < (s_tNow - s_tFixed);
}

void easy2d::Time::__updateNow()
{
	// 刷新时间
	s_tNow = SDL_GetPerformanceCounter();
}

void easy2d::Time::__updateLast()
{
	s_tFixed += s_tExceptedInterval;

	s_tLast = s_tNow;
	s_tNow = SDL_GetPerformanceCounter();
}

void easy2d::Time::__reset()
{
	Uint64 now = SDL_GetPerformanceCounter();
	s_tLast = s_tFixed = s_tNow = now;
}

void easy2d::Time::__sleep()
{
	if (s_tExceptedInterval > 0)
	{
		// 计算挂起时长（毫秒）
		Uint64 waitTicks = s_tExceptedInterval - (s_tNow - s_tFixed);
		Uint64 waitMs = ticksToMilliseconds(waitTicks);

		if (waitMs > 1)
		{
			// 挂起线程，释放 CPU 占用
			SDL_Delay(static_cast<Uint32>(waitMs));
		}
	}
}
