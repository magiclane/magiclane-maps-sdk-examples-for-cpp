// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_MapViewPreferences.h>
#include <API/GEM_RoutingService.h>
#include <API/GEM_NavigationListener.h>
#include <API/GEM_NavigationService.h>
#include <API/Extensions/GEM_Utils.h>

#include <filesystem>
#include <chrono>
#include <thread>

class MyNavigationListener : public gem::INavigationListener
{
private:
    void onNavigationStarted() override
    {
        GEM_LOGI("Simulation started");
    }
    void onNavigationInstructionUpdated(const gem::NavigationInstruction& inst) override
    {
        GEM_LOGI("New instruction");
    }
    void onWaypointReached(const gem::Landmark& lmk) override
    {
        GEM_LOGI("Intermediary destination reached");
    }
    void onDestinationReached(const gem::Landmark& lmk) override
    {
        GEM_LOGI("Final destination reached");
    }

    void onNavigationError(int error) override
    {
        GEM_LOGI("Nav error: %d", error);
    }

    void onRouteUpdated(const gem::Route& route) override
    {
        GEM_LOGI("Route updated");
    }

    void onNavigationSound(gem::ISound const& sound) override
    {
        GEM_LOGI("Nav sound play request");
    }

    bool canPlayNavigationSound() override
    {
        return true;
    }

    void onBetterRouteDetected(const gem::Route& route, int travelTime, int delay, int timeGain) override
    {
    }
};

// This thread demonstrates active detection of the position indicator presence within the viewport
bool navThreadStop = false;
void navThread(std::string navThread, gem::StrongPointer<gem::MapView> mapView)
{
    auto positionArrow = gem::MapSceneObject::getDefPositionTracker().first;
    bool isPositionArrowVisible = true;
    while (!navThreadStop)
    {
        auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count();
        if (currentTimeMs % 500 == 0)
        {
            isPositionArrowVisible = mapView.get()->checkObjectVisibility(*positionArrow);
            GEM_LOGE("POSITION ARROW IS %s THE VIEWPORT # # #", isPositionArrowVisible ? "WITHIN" : "OUTSIDE");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

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
    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "NavigationLowEndHW", &pTouchEventListener)); 
    if ( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    ////////////////////////////////////
    // low end CPU hardware sdk settings
    ////////////////////////////////////
    auto mapPrefs = mapView.get()->preferences();
    mapPrefs.enableCursorRender(false);
    auto positionArrow = gem::MapSceneObject::getDefPositionTracker().first;
    positionArrow->setVisibility(true);
    mapPrefs.setBuildingsVisibility(gem::EBuildingsVisibility::BV_2D);
    mapPrefs.setMapDetailsQualityLevel(gem::EMapDetailsQualityLevel::MDQL_Low);
    mapPrefs.setDrawFPS(true, gem::Xy(120, 60));
    mapPrefs.setViewAngle(0);
    mapView.get()->setZoomLevel(60);
    mapView.get()->extensions().setNavigationRouteLowRateUpdate(true);
    mapView.get()->extensions().setLowEndCPUOptimizations(true);
    mapPrefs.setMapLabelsFading(false);
    mapPrefs.followPositionPreferences().setPerspective(gem::EMapViewPerspective::MVP_2D);
    mapPrefs.followPositionPreferences().setViewAngle(0);
    mapPrefs.followPositionPreferences().setTouchHandlerModifyPersistent(true);
    ////////////////////////////////////
    // path to input files
    ////////////////////////////////////
    auto sdkExamplesPath = Environment::GetInstance().GetSDKExamplesPath();
    auto sdkCachePath = Environment::GetInstance().GetCachePath();

    auto dstCacheResPath = gem::FileSystem().makePath(sdkCachePath.c_str(), u"Data", u"Res/");
    gem::FileSystem().createFolder(dstCacheResPath, true);
    auto dstCacheSceneResPath = gem::FileSystem().makePath(sdkCachePath.c_str(), u"Data", u"SceneRes/");
    gem::FileSystem().createFolder(dstCacheSceneResPath, true);
    auto srcGPXPath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"RoutesAndNavigation", u"NavigationLowEndHW", u"strasbourg_points.gpx");
    auto srcNMEAPath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"RoutesAndNavigation", u"NavigationLowEndHW", u"strasbourg.nmea");
    auto srcStylePath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"RoutesAndNavigation", u"NavigationLowEndHW", u"MobileCycleBasic.style");

    int ret;
    if ((ret = gem::FileSystem().copyFile(srcGPXPath, dstCacheResPath)) != gem::KNoError)
    {
        GEM_LOGE("Error copy GPX resource (%d)", GEM_GET_API_ERROR());
    }
    if ((ret = gem::FileSystem().copyFile(srcNMEAPath, dstCacheResPath)) != gem::KNoError)
    {
        GEM_LOGE("Error copy NMEA resource (%d)", GEM_GET_API_ERROR());
    }
    if ((ret = gem::FileSystem().copyFile(srcStylePath, dstCacheSceneResPath)) != gem::KNoError)
    {
        GEM_LOGE("Error copy style resource (%d)", GEM_GET_API_ERROR());
    }

    auto srcMapStyle = gem::FileSystem().makePath(dstCacheSceneResPath, "MobileCycleBasic.style");
    mapView->preferences().setMapStyleByPath(srcMapStyle);

    ////////////////////////////////////
    // read waypoints from gpx
    ////////////////////////////////////
    srcGPXPath = gem::FileSystem().makePath(dstCacheResPath, "strasbourg_points.gpx");
    gem::DataBuffer gpxDataBuf;
    std::ifstream gpxDataStream;
    gpxDataStream.open(srcGPXPath.toStdString().c_str(), std::ios::binary);
    if (gpxDataStream.good())
    {
        gpxDataStream.seekg(0, std::ios::end);
        gpxDataBuf.reserve(int(gpxDataStream.tellg()));
        gpxDataStream.seekg(0, std::ios::beg);
        gpxDataStream.read(gpxDataBuf.getBytes<char>(), gpxDataBuf.size());
    }

    ////////////////////////////////////
    // calculate and render GPX route
    ////////////////////////////////////
    gem::Path path(gpxDataBuf, gem::PFF_Gpx);
    if (!path.getWayPoints().empty() && !path.getCoordinates().empty())
    {
        gem::RouteList routes;
        ProgressListener routeListener;
        gem::Landmark lmk;
        gem::LandmarkList waypoints;
        waypoints.emplace_back();
        gem::RouteBookmarks::setWaypointTrackData( waypoints.back_nc(), path );
        gem::RoutingService().calculateRoute(routes, waypoints, gem::RoutePreferences().setTransportMode(gem::RTM_Car).setRouteType(gem::RT_Fastest).setAlternativesSchema(gem::AS_Never), &routeListener);
        // Wait until route calculation finished & check success
        if (WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener), 30000) && routeListener.GetError() == gem::KNoError
            && !routes.empty())
        {
            // Draw first resulting route (at index 0) on map view
            mapView->preferences().routes().add(routes[0]);
            mapView->centerOnRoute(routes[0], gem::Rect(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
            // Instantiate a navigation event listener for either navigation (real or playback) or simulation
            auto navListener = gem::StrongPointerFactory<MyNavigationListener>();
            srcNMEAPath = gem::FileSystem().makePath(dstCacheResPath, "strasbourg.nmea");
            // Start playback navigation along the route - first create and set the previously recorded navigation playback datasource
            auto dataSource = gem::sense::DataSourceFactory::produceLog(srcNMEAPath);
            bool isNavigationPlayback = true;
            if (isNavigationPlayback) // pre-recorded navigation playback
            {
                gem::PositionService().setDataSource(dataSource);
                gem::NavigationService().startNavigation(routes[0], navListener, gem::ProgressListener());
            }
            else // simulation
            {
                gem::NavigationService().startSimulation(routes[0], navListener, gem::ProgressListener());
            }
            // Start follow GPS positions ( generated by the simulation ) - camera follows the position along the route
            mapView->startFollowingPosition();
        }
    }

    std::thread myNavThread(navThread, "navThread", mapView);

    WAIT_UNTIL_WINDOW_CLOSE();

    navThreadStop = true;
    myNavThread.join();

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
