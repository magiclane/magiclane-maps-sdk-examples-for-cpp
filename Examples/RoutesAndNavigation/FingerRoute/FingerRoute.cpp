// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_Types.h>
#include <API/GEM_Time.h>
#include <API/GEM_MapView.h>

#define MSEC_TOUCH_TRIGGER_DRAW 300

class CTouchDrawRoute
{
	public:

	CTouchDrawRoute()
	{
		m_isDrawing = false;
		m_isRouteRendered = false;
		m_is2WaypointRoute = false;
		m_touchDownTime = 0;
		m_lastXyAdded.x = -1;
		m_lastXyAdded.y = -1;
	}
	void addWaypoint(gem::Xy xy, gem::ETouchEvent state, std::shared_ptr<gem::MapView> mapView)
	{
		long long int inputTime = gem::Time::getUniversalTime().asInt();
		if ( m_isDrawing )
		{
			if ( state == gem::ETouchEvent::TE_Down )
			{
				m_isDrawingStarted = true;
				m_touchDownTime = inputTime;
			}
			if ( m_isDrawingStarted )
			{
				// do not add the same waypoint more than once
				if ( xy.x != m_lastXyAdded.x || xy.y != m_lastXyAdded.y )
				{
					auto coord = mapView->transformScreenToWgs(xy);
					if ( !m_is2WaypointRoute
					|| (m_is2WaypointRoute && (m_waypoints.size() < 1 || state == gem::ETouchEvent::TE_Up)))
					{
						m_waypoints.push_back(gem::Landmark("waypoint", { coord.getLatitude(), coord.getLongitude() }));
						fprintf(stderr,"added waypoint %d\n",(int)m_waypoints.size());
						m_lastXyAdded.x = xy.x;
						m_lastXyAdded.y = xy.y;
					}
				}
else fprintf(stderr,"addwaypoint SKIP IDENTICAL x y %d %d\n",xy.x,xy.y);
			}
			if ( m_isDrawingStarted && state == gem::ETouchEvent::TE_Up )
			{
fprintf(stderr,"addwaypoint ROUTE %d\n",(int)m_waypoints.size());
				m_isDrawing = false;
				m_isDrawingStarted = false;
				if ( m_waypoints.size() > 1 )
				{
					ProgressListener calculateRouteListener;
					gem::RoutingService().calculateRoute(m_routes, m_waypoints, m_preferences, &calculateRouteListener);
					auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &calculateRouteListener), 15000);
					if ( m_routes.size() > 0 )
					{
fprintf(stderr,"addwaypoint CENTER\n");
						m_isRouteRendered = true;
						m_is2WaypointRoute = false;
						mapView->centerOnRoute(m_routes[0], gem::Rect(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 3000));
						mapView->preferences().routes().add(m_routes[0], true);
					}
				}
				else
				{
					int tdiff = m_touchDownTime > 0 ? (int)(inputTime - m_touchDownTime) : -1;
fprintf(stderr,"addwaypoint FAST 2CLICK VERIFY diff %d\n",tdiff);
					if ( tdiff < MSEC_TOUCH_TRIGGER_DRAW && tdiff >= 0 )
					{
fprintf(stderr,"addwaypoint FAST 2CLICK DETECTED\n");
						//////////////////////////////////////////////////
						//fast double click, activate 2-waypoint route, only departure and destination
						//////////////////////////////////////////////////
						m_is2WaypointRoute = true;
						m_isDrawing = true;
						m_isDrawingStarted = true;
					}
				}
				m_touchDownTime = 0;
			}
		}
		else
		{
			// not drawing - detect a touch less than MSEC_TOUCH_TRIGGER_DRAW
			// to start drawing route
			/////////////////////////
			if ( state == gem::ETouchEvent::TE_Down )
			{
				m_touchDownTime = inputTime;
			}
			else if ( state == gem::ETouchEvent::TE_Up )
			{
				int tdiff = m_touchDownTime > 0 ? (int)(inputTime - m_touchDownTime) : -1;
				if ( tdiff < MSEC_TOUCH_TRIGGER_DRAW && tdiff >= 0 )
				{
					//////////////////////////////////////////////////
					//fast single click, activate multi-waypoint route
					//////////////////////////////////////////////////
fprintf(stderr,"addwaypoint DRAWING ACTIVATED\n");
					m_isDrawing = true;
					m_isDrawingStarted = false;
					m_lastXyAdded.x = -1;
					m_lastXyAdded.y = -1;
					m_waypoints.clear();
					m_routes.clear();
					m_isRouteRendered = false;
				}
				m_touchDownTime = 0;
			}
		}
	}
	bool isDrawing() const { return m_isDrawingStarted; }
	bool isRouteRendered() const { return m_isRouteRendered; }

	private:

	gem::Xy m_lastXyAdded;
	bool m_isDrawing;
	bool m_isDrawingStarted;
	bool m_isRouteRendered;
	bool m_is2WaypointRoute;
	long long int m_touchDownTime;
	gem::LandmarkList m_waypoints;
	gem::RouteList m_routes;
	gem::RoutePreferences m_preferences;
};

// Derive from the standard touch event handler class which makes the map view interactive

class MyTouchEventListener : public CTouchEventListener
{
public:

	// This function from the standard touch event handler for the map view is
	// overridden to add our own processing - enabling drawing a route by dragging
	// after a single or double click on the map.

	void handleTouchEvent(int eventType,int pointerId,int x,int y)
	{
		auto mapView = getMapViewPointer();
		setCursorPosition(x, y);
		gem::Xy mousePos(x,y);
		if ( mapView.get()!=nullptr )
		{
			drawRoute.addWaypoint(mousePos, (gem::ETouchEvent)eventType, mapView);
			if ( !drawRoute.isDrawing() )
			{
				mapView->getScreen()->handleTouchEvent((gem::ETouchEvent)eventType,pointerId,mousePos);
			}
		}
		else
		{
			fprintf(stderr,"null mapView!\n");
		}
		auto coord = mapView->transformScreenToWgs(mousePos);
		fprintf(stderr,"xy %d %d mapcoord %f %f %s\n",x,y,coord.getLongitude(),coord.getLatitude(),drawRoute.isDrawing()?"DRAWING":"VIEW");
	}

private:

	CTouchDrawRoute drawRoute;
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
	MyTouchEventListener pTouchEventListener;
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "FingerRoute", &pTouchEventListener)); 
    if ( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

	// Coordinates are specified as lat,lon in degrees
	mapView->centerOnCoordinates({ 42.63134, 3.02693 }, 60);

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
