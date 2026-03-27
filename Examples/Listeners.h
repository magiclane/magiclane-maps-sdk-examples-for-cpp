// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#include <API/GEM_Sdk.h>
#include <API/GEM_MapViewListener.h>
#include <API/GEM_OffboardListener.h>
#include <API/GEM_ProgressListener.h>
#include <API/GEM_LogUploader.h>
#include <API/GEM_RenderContext.h>
#include <API/GEM_MapView.h>

#include "OpenGLContext.h"

class MapViewListenerImpl : public gem::IMapViewListener
{
public:
	MapViewListenerImpl();

	void onViewRendered(gem::EViewDataTransitionStatus tivStatus, gem::EViewCameraTransitionStatus camStatus) override;
	bool IsFinished();

private:

	bool m_bStatus{ false };
};

class OffboardListenerImpl : public gem::IOffboardListener
{

public:
	OffboardListenerImpl();

	void onConnectionStatusUpdated(bool connected) override;
	void onWorldwideRoadMapSupportDisabled(EReason reason) override;
	void onWorldwideRoadMapSupportStatus(EStatus state) override;
	void onApiTokenRejected() override;

	bool IsOnline() const;
private:

	bool m_bStatus{ false };
};

class ProgressListener : public gem::IProgressListener
{

public:
	ProgressListener() { Reset(); }

	void notifyStart(bool) override
	{
		m_bStarted = true;
		m_bFinished = false;
		m_hint = "";
		m_error = gem::KNoError;
	}
	void notifyProgress(int value) override
	{
		m_bProgress = true;
		m_progressValue = value;
	}
	void notifyComplete(int reason, gem::String hint = gem::String()) override
	{
		m_bFinished = true;
		m_error = reason;
		m_hint = hint;
	}
	void notifyStatusChanged(int value) override
	{
		m_status = value;
	}

	int GetStatus() const
	{
		return m_status;
	}
	int GetProgressValue() const
	{
		return m_progressValue;
	}
	bool IsStarted()
	{
		return m_bStarted;
	}
	bool HasProgress()
	{
		return m_bProgress;
	}
	bool IsFinished()
	{
		return m_bFinished;
	}
	int GetError()
	{
		return m_error;
	}
	gem::String GetHint()
	{
		return m_hint;
	}

    void Reset()
    {
        m_bFinished = m_bStarted = m_bProgress = false;
        m_error = gem::KNoError;
        m_progressValue = 0;
        m_status = 0;
        m_hint.clear();
    }

private:
	bool m_bFinished, m_bStarted, m_bProgress;
	int m_error;
	int m_progressValue;
	int m_status;
	gem::String m_hint;
};

class CTouchEventListener : public ITouchEventsListener
{
public:
    std::shared_ptr<gem::MapView> getMapViewPointer()
    {
        return m_mapView;
    }

    void setParent( gem::StrongPointer<gem::Screen> parent ) override
    {
        if( !m_mapView )
        {
            parent->iterateViews( [this](std::shared_ptr<gem::MapView> view )
                {
                    m_mapView = view;
                    lastXpos = 0; lastYpos = 0;
                    return false;
                }
            );
        }
    }
    void handleTouchEvent( int eventType, int pointerId, int x, int y ) override
	{
		lastXpos = x; lastYpos = y;
		if (eventType == gem::ETouchEvent::TE_Down)
			m_pressed = true;
		else if (eventType == gem::ETouchEvent::TE_Up)
			m_pressed = false;
		if (!m_mapInteractionEnabled) return;
		gem::Xy mousePos(x,y);
		if ( m_mapView.get()!=nullptr )
		{
			m_mapView->getScreen()->handleTouchEvent((gem::ETouchEvent)eventType,pointerId,mousePos);
		}
	}

	bool isPressed() const { return m_pressed; }
	void setMapInteractionEnabled(bool enabled) { m_mapInteractionEnabled = enabled; }

	void handleMouseScrollEvent(int delta,int x,int y) override
	{
		if (!m_mapInteractionEnabled) return;
		gem::Xy mousePos(x,y);
		if ( m_mapView.get()!=nullptr )
		{
			m_mapView->getScreen()->scrollEvent(delta,mousePos);
		}
	}
	void getCursorPosition(int &x, int &y) override
	{
		x = lastXpos; y = lastYpos;
	}
	void setCursorPosition(const int &x, const int &y)
	{
		lastXpos = x; lastYpos = y;
	}

private:

	std::shared_ptr<gem::MapView> m_mapView;
	int lastXpos, lastYpos;
	bool m_pressed = false;
	bool m_mapInteractionEnabled = true;
};
