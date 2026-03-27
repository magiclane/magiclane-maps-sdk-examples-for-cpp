// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Listeners.h"

#include <API/GEM_Debug.h>

#include "OpenGLContext.h"

MapViewListenerImpl::MapViewListenerImpl()
{
}

void MapViewListenerImpl::onViewRendered(gem::EViewDataTransitionStatus tivStatus, gem::EViewCameraTransitionStatus camStatus)
{
	if (tivStatus == gem::EViewDataTransitionStatus::VD_Complete)
		m_bStatus = true;
}

bool MapViewListenerImpl::IsFinished()
{
	return m_bStatus;
}

OffboardListenerImpl::OffboardListenerImpl()
{
}

void OffboardListenerImpl::onConnectionStatusUpdated(bool connected)
{
	m_bStatus = connected;
}

void OffboardListenerImpl::onWorldwideRoadMapSupportDisabled(EReason reason)
{

}

void OffboardListenerImpl::onWorldwideRoadMapSupportStatus(EStatus state)
{

}

void OffboardListenerImpl::onApiTokenRejected()
{
	gem::Debug().log(gem::LogWarn, "Listeners", __FUNCTION__, __FILE__, __LINE__, "API token rejected!");
}

bool OffboardListenerImpl::IsOnline() const
{
	return m_bStatus;
}
