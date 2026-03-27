// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_PublicTransportOverlay.h>

using namespace gem;

class ScheduleProgressListenerImpl : public gem::IProgressListener
{

public:
    ScheduleProgressListenerImpl() { Reset(); }

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
        GEM_INFO_LOG( "Schedule fetch completed with reason: %d, hint: %s", reason, hint.constData() );

        // Start using the information, it must contain data now.
        if (!ptStopInfo.empty()) 
        {
            // Information about agencies.
            auto agencies = ptStopInfo.find<gem::SearchableParameterList>(opid::kPT_agencies);

            // Information about stops and generic routes
            // (routes that don't have `heading` set)
            auto stops = ptStopInfo.find<gem::SearchableParameterList>(opid::kPT_stops);

            // Information about trips (together with 
            // route, agency, stop times, real-time info, etc.)
            auto trips = ptStopInfo.find<gem::SearchableParameterList>(opid::kPT_trips);

            // Go through stops.
            for (auto stop : stops.first) 
            {
                // Get the stop details.
                auto stopDetails = stop.getValue<gem::SearchableParameterList>();

                GEM_INFO_LOG("Stop id: %llu", stopDetails.find<gem::LargeInteger>(opid::kPT_stop_id).first);
                GEM_INFO_LOG("Stop name: %s", stopDetails.find<gem::String>(opid::kPT_stop_name).first);

                GEM_INFO_LOG("Routes:");

                auto stopRoutes = stopDetails.find<gem::SearchableParameterList>(opid::kPT_stop_routes);

                // Go through the routes that visit this stop
                for (auto route : stopRoutes.first)
                {
                    auto routeDetails = route.getValue<gem::SearchableParameterList>();

                    GEM_INFO_LOG("  Route id: %llu", routeDetails.find<gem::LargeInteger>(opid::kPT_route_id).first);
                    GEM_INFO_LOG("  Route short name: %s", routeDetails.find<gem::String>(opid::kPT_route_short_name).first);
                    GEM_INFO_LOG("  Route long name: %s", routeDetails.find<gem::String>(opid::kPT_route_long_name).first);
                }
            }
        }

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

    SearchableParameterList& GetStopInfo()
    {
        return ptStopInfo;
    }

private:
    bool m_bFinished, m_bStarted, m_bProgress;
    int m_error;
    int m_progressValue;
    int m_status;
    gem::String m_hint;
    SearchableParameterList ptStopInfo;
};

class MyMapViewListenerImpl : public IMapViewListener
{
public:

    void setMapView(StrongPointer<MapView> mv)
    {
        mapView = mv;
    }

    void onLongDown(const Xy &pos) override
    {
        GEM_INFO_LOG( "Long down at position: (%d, %d)", pos.x, pos.y );

        if(scheduleProgressListenerPtr && !scheduleProgressListenerPtr->IsFinished())
        {
            GEM_INFO_LOG( "Schedule fetch already in progress, please wait..." );
            return;
        }

        // Set cursor position on the screen.
        mapView->setCursorScreenPosition(pos);

        // Wait for the map view to fully load the tiles

        // Get the overlay items at that position.
        auto items = mapView->cursorSelectionOverlayItems();

        // For each overlay item at that position.
        for (auto item : items) 
        {
            auto overlayId = item.getOverlayUid();

            // We're interested only in public tranposrt overlay items.
            if(overlayId == ECommonOverlayId::OID_PublicTransport)
            {
                GEM_INFO_LOG( "Public transport overlay item found with UID: %llu, fetching schedule", item.getUid() );
                scheduleProgressListenerPtr = StrongPointerFactory<ScheduleProgressListenerImpl>();

                item.getPreviewExtendedData(scheduleProgressListenerPtr->GetStopInfo(), scheduleProgressListenerPtr);
            }
        }

    }
private:
    StrongPointer<MapView> mapView;
    StrongPointer<ScheduleProgressListenerImpl> scheduleProgressListenerPtr;
};

int main( int argc, char** argv )
{
	// Get new project API token from:
	// https://developer.magiclane.com/api/projects
	std::string projectApiToken = "";

#if defined(API_TOKEN)
	projectApiToken = std::string( API_TOKEN );
#else
	auto value = std::getenv( "GEM_TOKEN" );
	if( value != nullptr )
		projectApiToken = value;
#endif

	// Sdk objects can be created & used below this line
	Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


	// Create an interactive map view
	CTouchEventListener pTouchEventListener;
    auto myMapViewListenerImpl = gem::StrongPointerFactory<MyMapViewListenerImpl>();
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "PublicTransitStop", &pTouchEventListener), myMapViewListenerImpl); 
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    myMapViewListenerImpl->setMapView( mapView );

	mapView->centerOnCoordinates({ 52.37912, 4.90027 }, 95);

	WAIT_UNTIL_WINDOW_CLOSE();
	return 0;
}

#if ( defined(_WIN32) || defined(_WIN64) ) && !defined(__MINGW32__) && !defined(__MINGW64__)

int WINAPI WinMain( HINSTANCE hInstance, // Instance
	HINSTANCE hPrevInstance, // Previous Instance
	LPSTR lpCmdLine, // Command Line Parameters
	int nCmdShow )
{
	main( 0, nullptr );

	return 0;
}

#endif
