// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#include <API/GEM_Timer.h>

class Timer : public gem::ITimerService
{
public:
	Timer();

	void Pause();
	void Resume();
	void Tick();
	unsigned int GetPeriod();

protected:

	int onRegisterListener(gem::ITimerListener* listener);
	void onUnregisterListener();

	int onStartTimer(int intervalMs);
	int onStopTimer();

protected:
	gem::ITimerListener* m_pListener;
	int m_nTimerId;
	bool m_bPauseTimer;
	unsigned int m_nPeriod;
};
