// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>

#include <API/GEM_SenseDataTypes.h>
#include <API/GEM_SenseDataSource.h>
#include <API/GEM_NavigationService.h>
#include <API/GEM_NavigationListener.h>
#include <API/GEM_Debug.h>
#include <API/Extensions/GEM_Utils.h>

#include <imgui.h>

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

bool playbackStarted = false;
auto getUiRender()
{
    auto sdkExamplesPath = Environment::GetInstance().GetSDKExamplesPath();
    auto sdkCachePath = Environment::GetInstance().GetCachePath();

    auto dstCacheResPath = gem::FileSystem().makePath(sdkCachePath.c_str(), u"Data", u"Res/");
    gem::FileSystem().createFolder(dstCacheResPath, true);

    auto srcGPXPath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"Interactive", u"GpxNmeaPlayback", u"strasbourg_points.gpx");
    auto srcNMEAPath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"Interactive", u"GpxNmeaPlayback", u"strasbourg.nmea");

    int ret;
    if ((ret = gem::FileSystem().copyFile(srcGPXPath, dstCacheResPath)) != gem::KNoError)
    {
        GEM_LOGE("Error copy GPX resource (%d)", GEM_GET_API_ERROR());
    }
    if ((ret = gem::FileSystem().copyFile(srcNMEAPath, dstCacheResPath)) != gem::KNoError)
    {
        GEM_LOGE("Error copy NMEA resource (%d)", GEM_GET_API_ERROR());
    }

    srcGPXPath = gem::FileSystem().makePath(dstCacheResPath, "strasbourg_points.gpx");
    srcNMEAPath = gem::FileSystem().makePath(dstCacheResPath, "strasbourg.nmea");

    ////////////////////////////////////
    // read waypoints from gpx
    ////////////////////////////////////
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

    auto dataSourceNMEA = gem::sense::DataSourceFactory::produceLog(srcNMEAPath);
    dataSourceNMEA.get()->stop();
    gem::PositionService().setDataSource(dataSourceNMEA);

    return std::bind([gpxDataBuf, dataSourceNMEA](gem::StrongPointer<gem::MapView> mapView)
    {
        ImGuiIO& io = ImGui::GetIO();
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoSavedSettings);
        gem::RouteList routes;
        auto navListener = gem::StrongPointerFactory<MyNavigationListener>();
        ImGui::Spacing();
        if (ImGui::Button("Calculate route over GPX track"))
        {
            playbackStarted = false;
            if (!dataSourceNMEA.get()->isStopped())
            {
                dataSourceNMEA.get()->stop();
            }
            ////////////////////////////////////
            // calculate and render GPX route
            ////////////////////////////////////
            gem::Path path(gpxDataBuf, gem::PFF_Gpx);
            if (!path.getWayPoints().empty() && !path.getCoordinates().empty())
            {
                ProgressListener routeListener;
                gem::Landmark lmk;
                gem::LandmarkList waypoints;
                waypoints.emplace_back();
                gem::RouteBookmarks::setWaypointTrackData(waypoints.back_nc(), path);
                gem::RoutingService().calculateRoute(routes, waypoints, gem::RoutePreferences().setTransportMode(gem::RTM_Car).setRouteType(gem::RT_Fastest).setAlternativesSchema(gem::AS_Never), &routeListener);
                // Wait until route calculation finished & check success
                if (WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener), 30000) && routeListener.GetError() == gem::KNoError
                    && !routes.empty())
                {
                    // Draw first resulting route (at index 0) on map view
                    mapView->preferences().routes().add(routes[0]);
                    mapView->centerOnRoute(routes[0], gem::Rect(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
                }
            }
        }
        if (mapView->preferences().routes().size() > 0)
        {
            if (dataSourceNMEA.get()->isStopped())
            {
                ImGui::Spacing();
                if (ImGui::Button("Start NMEA pre-recorded navigation playback"))
                {
                    playbackStarted = true;
                    ////////////////////////////////////
                    // play back pre-recorded NMEA position log matching the GPX route
                    ////////////////////////////////////
                    if (dataSourceNMEA)
                    {
                        dataSourceNMEA.get()->start();
                    }
                    gem::NavigationService().startNavigation(mapView->preferences().routes().getMainRoute(), navListener, gem::ProgressListener());
                }
            }
            else
            {
                ImGui::BeginDisabled(true);
                ImGui::Spacing();
                ImGui::Button("Start NMEA pre-recorded navigation playback");
                ImGui::EndDisabled();
            }
            if (dataSourceNMEA.get()->isStopped())
            {
                ImGui::BeginDisabled(true);
                ImGui::Spacing();
                ImGui::Button("Resume playback");
                ImGui::EndDisabled();
            }
            else
            {
                if (dataSourceNMEA.get()->getPlayback()->getState() == gem::sense::EPlayingStatus::Paused)
                {
                    ImGui::Spacing();
                    if (ImGui::Button("Resume playback"))
                    {
                        if (dataSourceNMEA)
                        {
                            dataSourceNMEA.get()->getPlayback()->resume();
                        }
                    }
                }
                else if (dataSourceNMEA.get()->getPlayback()->getState() == gem::sense::EPlayingStatus::Playing)
                {
                    ImGui::Spacing();
                    if (ImGui::Button("Pause playback"))
                    {
                        if (dataSourceNMEA)
                        {
                            dataSourceNMEA.get()->getPlayback()->pause();
                        }
                    }
                }
                else
                {
                    ImGui::BeginDisabled(true);
                    ImGui::Spacing();
                    ImGui::Button("Resume playback");
                    ImGui::EndDisabled();
                }
            }
            if (dataSourceNMEA.get()->isStopped())
            {
                ImGui::BeginDisabled(true);
                ImGui::Spacing();
                ImGui::Button("Stop playback");
                ImGui::EndDisabled();
            }
            else
            {
                ImGui::Spacing();
                if (ImGui::Button("Stop playback"))
                {
                    if (dataSourceNMEA)
                    {
                        playbackStarted = false;
                        dataSourceNMEA.get()->stop();
                    }
                }
            }
        }
        else
        {
            ImGui::BeginDisabled(true);
            ImGui::Spacing();
            ImGui::Button("Start NMEA pre-recorded navigation playback");
            ImGui::Spacing();
            ImGui::Button("Resume playback");
            ImGui::Spacing();
            ImGui::Button("Stop playback");
            ImGui::EndDisabled();
        }
        if (playbackStarted && gem::PositionService().getDataSource())
        {

            if (mapView->isFollowingPosition())
            {
                ImGui::BeginDisabled(true);
                ImGui::Spacing();
                ImGui::Button("Follow position");
                ImGui::EndDisabled();
            }
            else
            {
                if (gem::PositionService().getDataSource().get()->isStopped())
                {
                    ImGui::BeginDisabled(true);
                    ImGui::Spacing();
                    ImGui::Button("Follow position");
                    ImGui::EndDisabled();
                }
                else
                {
                    ImGui::Spacing();
                    if (ImGui::Button("Follow position"))
                    {
                        mapView->startFollowingPosition();
                    }
                }
            }
        }
        else
        {
            ImGui::BeginDisabled(true);
            ImGui::Spacing();
            ImGui::Button("Follow position");
            ImGui::EndDisabled();
        }
        ImGui::End();
    }
    , std::placeholders::_1);
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
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::ImGUI, "GpxNmeaPlayback", &pTouchEventListener, getUiRender()));
	if ( !mapView )
	{
		GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
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
