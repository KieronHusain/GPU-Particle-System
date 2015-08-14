//SIMPLE FPS COUNTER CLASS TAKEN FROM - http://www.rastertek.com/dx11tut15.html

///////////////////////////////////////////////////////////////////////////////
// Filename: fpsclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "FPS.h"


FpsClass::FpsClass()
{
}


FpsClass::FpsClass(const FpsClass& other)
{
}


FpsClass::~FpsClass()
{
}

void FpsClass::Initialize()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();
	return;
}

void FpsClass::Frame()
{
	m_count++;

	if (timeGetTime() >= (m_startTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;

		m_startTime = timeGetTime();
	}
}

int FpsClass::GetFps()
{
	return m_fps;
}