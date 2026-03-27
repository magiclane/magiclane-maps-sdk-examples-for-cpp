// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>
#include <API/GEM_NavigationListener.h>
#include <API/GEM_NavigationService.h>
#include <API/GEM_Alarms.h>

class MyNavigationListener : public gem::INavigationListener
{
private:
    void onNavigationStarted() override
    {
        GEM_LOGI( "Simulation started" );
    }
    void onNavigationInstructionUpdated( const gem::NavigationInstruction &inst ) override
    {
        GEM_LOGI( "New instruction" );
    }
    void onWaypointReached( const gem::Landmark& lmk ) override
    {
        GEM_LOGI( "Intermediary destination reached" );
    }
    void onDestinationReached( const gem::Landmark& lmk ) override
    {
        GEM_LOGI( "Final destination reached" );
    }

    void onNavigationError( int error ) override
    {
        GEM_LOGI( "Nav error: %d", error );
    }
    
    void onRouteUpdated( const gem::Route& route ) override
    {
        GEM_LOGI( "Route updated" );
    }

    void onNavigationSound( gem::ISound const& sound ) override
    {
        GEM_LOGI( "Nav sound play request" );
    }

    bool canPlayNavigationSound() override
    {
        return true;
    }

    void onBetterRouteDetected( const gem::Route& route, int travelTime, int delay, int timeGain ) override
    {
    }
};

class MyAlarmListener : public gem::IAlarmListener
{
    void onBoundaryCrossed( const gem::AlarmMonitoredAreaList &enteredAreas, const gem::AlarmMonitoredAreaList &exitedAreas ) override
    {
        for (auto area : enteredAreas)
        {
            GEM_INFO_LOG("ENTERED AREA: %s", area.id);
        }

        for (auto area : exitedAreas)
        {
            GEM_INFO_LOG("EXITED AREA: %s", area.id);
        }
    }
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


	// Create a map view
	CTouchEventListener pTouchEventListener;
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "AreasAlarms", &pTouchEventListener)); 
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    // Create and register an alarm listener
    auto alarmListener = gem::StrongPointerFactory<MyAlarmListener>();

    auto alarmService = gem::AlarmService::produce( alarmListener );

    // Register an area to monitor for alarms ( example: a circular area around San Francisco )
    auto circleArea = gem::CircleGeographicArea({37.77933, -122.41841}, 50);
    alarmService->monitorArea( circleArea, "SF circle area" );

    // Add the area to the map view for visualization

    // Create a marker collection for monitored areas
    gem::MarkerCollection monitoredAreaCollection( gem::EMarkerType::MT_Polygon, "Monitored Areas" );
    
    // Create a marker from the geographic area
    gem::Marker areaMarker( circleArea );
    monitoredAreaCollection.add( areaMarker );

    gem::MarkerCollectionRenderSettings renderSettings;
    renderSettings.setPolygonFillColor( gem::Rgba( 210, 104, 102, 111 ) );

    // Add the monitored area collection to the map view
    mapView->preferences().markers().add( monitoredAreaCollection, renderSettings );

	// At least 2 waypoints define the route, the first is the departure position, and the last is the destination.
	// There can be zero or more intermediate waypoints through which the route passes in the order they are listed.
	// The coordinates are {latitude,longitude} in degrees; the landmark name is optional and can be an empty string.
	gem::LandmarkList waypoints( { { "San Francisco", { 37.77903, -122.41991 } }, { "San Jose", { 37.33619, -121.89058 } } } );

	// Compute route using these preferences: car / fastest / without alternatives in result
	gem::RouteList routes;
	ProgressListener routeListener;
    auto navListener = gem::StrongPointerFactory<MyNavigationListener>();
    gem::RoutingService().calculateRoute( routes, waypoints, gem::RoutePreferences().setTransportMode( gem::RTM_Car ).setRouteType( gem::RT_Fastest ).setAlternativesSchema( gem::AS_Never ), &routeListener );

	// Wait until route calculation finished & check success
	if ( WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener), 30000) && routeListener.GetError() == gem::KNoError && !routes.empty() )
	{
		// Add the first resulting route (at index 0) to map view
		mapView->preferences().routes().add( routes[0] );

		// Start simulated navigation along the route
		gem::NavigationService().startSimulation(routes[0], navListener, gem::ProgressListener());

		// Start follow GPS positions ( generated by the simulation ) - camera follows the position along the route
		mapView->startFollowingPosition();
	}

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
