// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>

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
    gem::StrongPointer<gem::Screen> screen = gem::Screen::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "CalculateRouteMultiView", &pTouchEventListener));

    // Map view in lower left viewport
    gem::StrongPointer<gem::MapView> mapView1 = gem::MapView::produce(screen, gem::RectF(0.f, 0.f, 0.5f, 0.5f));
    // Map view in upper left viewport
    gem::StrongPointer<gem::MapView> mapView2 = gem::MapView::produce(screen, gem::RectF(0.f, 0.5f, 0.5f, 1.0f));
    // Map view in lower right viewport
    gem::StrongPointer<gem::MapView> mapView3 = gem::MapView::produce(screen, gem::RectF(0.5f, 0.f, 1.0f, 0.5f));
    // Map view in upper right viewport
    gem::StrongPointer<gem::MapView> mapView4 = gem::MapView::produce(screen, gem::RectF(0.5f, 0.5f, 1.f, 1.f));

    // At least 2 waypoints define the route, the first is the departure position, and the last is the destination.
    // There can be zero or more intermediate waypoints through which the route passes in the order they are listed.
    // The coordinates are {latitude,longitude} in degrees; the landmark name is optional and can be an empty string.
    gem::LandmarkList waypoints1({
        { "San Francisco", { 37.77903, -122.41991 } },
        { "San Jose", { 37.33619, -121.89058 } }
        });
    gem::LandmarkList waypoints2({
        { "London", { 51.516128, -0.142828 } },
        { "Paris", { 48.848462, 2.327315 } }
        });
    gem::LandmarkList waypoints3({
        { "Andorra", { 42.50971 , 1.53787 } },
        { "Port Verdes", { 42.51851 , 3.10359 } }
        });
    gem::LandmarkList waypoints4({
        { "Strasbourg", { 48.51999 , 7.73254 } },
        { "Oslo", { 59.9265 , 10.7511 } }
        });

    gem::RouteList routes1, routes2, routes3, routes4;
    ProgressListener routeListener1, routeListener2, routeListener3, routeListener4;

    // Calculate route - car / fastest / without alternatives in result
    gem::RoutingService().calculateRoute(routes1, waypoints1, gem::RoutePreferences().setTransportMode(gem::RTM_Car).setRouteType(gem::RT_Fastest).setAlternativesSchema(gem::AS_Never), &routeListener1);
    gem::RoutingService().calculateRoute(routes2, waypoints2, gem::RoutePreferences().setTransportMode(gem::RTM_Car).setRouteType(gem::RT_Fastest).setAlternativesSchema(gem::AS_Never), &routeListener2);
    gem::RoutingService().calculateRoute(routes3, waypoints3, gem::RoutePreferences().setTransportMode(gem::RTM_Car).setRouteType(gem::RT_Fastest).setAlternativesSchema(gem::AS_Never), &routeListener3);
    gem::RoutingService().calculateRoute(routes4, waypoints4, gem::RoutePreferences().setTransportMode(gem::RTM_Car).setRouteType(gem::RT_Fastest).setAlternativesSchema(gem::AS_Never), &routeListener4);
    // Wait until route calculation finished & check success
    if (WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener1), 30000) && routeListener1.GetError() == gem::KNoError
        && WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener2), 30000) && routeListener2.GetError() == gem::KNoError
        && WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener3), 30000) && routeListener3.GetError() == gem::KNoError
        && WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener4), 30000) && routeListener4.GetError() == gem::KNoError
        && !routes1.empty()
        && !routes2.empty()
        && !routes3.empty()
        && !routes4.empty())
    {
        // Draw computed route on map view in lower left viewport
        mapView1->preferences().routes().add(routes1[0]);
        mapView1->centerOnRoute(routes1[0], gem::Rect(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
        // Draw computed route on map view in upper left viewport
        mapView2->preferences().routes().add(routes2[0]);
        mapView2->centerOnRoute(routes2[0], gem::Rect(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
        // Draw computed route on map view in lower right viewport
        mapView3->preferences().routes().add(routes3[0]);
        mapView3->centerOnRoute(routes3[0], gem::Rect(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
        // Draw computed route on map view in upper right viewport
        mapView4->preferences().routes().add(routes4[0]);
        mapView4->centerOnRoute(routes4[0], gem::Rect(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
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
