#include "Timer.h"

bool Timer::init()
{
	if (QueryPerformanceFrequency(&ticksPerSecond))
	{
		printf("hi-res timer initialized, ticks per sec: %d\n", ticksPerSecond.QuadPart);
		QueryPerformanceCounter(&lastTime);
		QueryPerformanceCounter(&lastFPSTime);
		return true;
	}
	else
		return false;	
}

void Timer::reset()
{
	QueryPerformanceCounter(&lastTime);
	QueryPerformanceCounter(&lastFPSTime);
}

float Timer::getElapsedSeconds()
{
	LARGE_INTEGER currentTime;
	float ms;

	QueryPerformanceCounter(&currentTime);
	ms = ((float)currentTime.QuadPart - (float)lastTime.QuadPart) / (float)ticksPerSecond.QuadPart;

	lastTime = currentTime;

	return ms;
}

double Timer::getElapsedSeconds2()
{
	LARGE_INTEGER currentTime;
	double ms;

	QueryPerformanceCounter(&currentTime);
	ms = ((double)currentTime.QuadPart - (double)lastTime.QuadPart) / (double)ticksPerSecond.QuadPart;

	lastTime = currentTime;

	return ms;
}

float Timer::getElapsedMilliseconds()
{
	LARGE_INTEGER currentTime;
	float ms;

	QueryPerformanceCounter(&currentTime);
	ms = ((float)currentTime.QuadPart - (float)lastTime.QuadPart) / (float)ticksPerSecond.QuadPart * 1000.0f;

	lastTime = currentTime;

	return ms;
}

float Timer::getFPS(unsigned int elapsedFrames)
{
	LARGE_INTEGER currentTime;
	float fps;

	QueryPerformanceCounter(&currentTime);
	fps = (float)elapsedFrames * (float)ticksPerSecond.QuadPart / ((float)currentTime.QuadPart - (float)lastFPSTime.QuadPart);

	lastFPSTime = currentTime;

	return fps;
}

double Timer::getFPS2(unsigned int elapsedFrames)
{
	LARGE_INTEGER currentTime;
	double fps;

	QueryPerformanceCounter(&currentTime);
	fps = (double)elapsedFrames * (double)ticksPerSecond.QuadPart / ((double)currentTime.QuadPart - (double)lastFPSTime.QuadPart);

	lastFPSTime = currentTime;

	return fps;
}
