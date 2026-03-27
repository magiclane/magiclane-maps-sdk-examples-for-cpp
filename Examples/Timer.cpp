// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Timer.h"

#include <API/GEM_Error.h>

Timer::Timer() 
	: m_pListener(nullptr)
	, m_nTimerId(0)
	, m_bPauseTimer(false)
	, m_nPeriod(5)
{

}

void Timer::Pause()
{
	m_bPauseTimer = true;
}

void Timer::Resume()
{
	m_bPauseTimer = false;
}

void Timer::Tick()
{
	if (!m_bPauseTimer && m_pListener)
		m_pListener->onTimer();
}

unsigned int Timer::GetPeriod()
{
	return m_nPeriod;
}

int Timer::onRegisterListener(gem::ITimerListener* listener)
{
	m_pListener = listener;
	return gem::KNoError;
}

void Timer::onUnregisterListener()
{
	m_pListener = nullptr;
}

int Timer::onStartTimer(int intervalMs)
{
	m_nPeriod = intervalMs;
	return gem::KNoError;
}

int Timer::onStopTimer()
{
	return gem::KNoError;
}
