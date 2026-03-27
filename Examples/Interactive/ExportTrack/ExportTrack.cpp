// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_SenseDataTypes.h>
#include <API/GEM_SenseDataSource.h>
#include <API/GEM_Debug.h>
#include <API/Extensions/GEM_Utils.h>

#include <sstream>
#include <iomanip>
#include <chrono>

#include <imgui.h>

// Create a filename for the track output file using the current date and time plus given extension
std::string generateFilename(const std::string& extension)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_c);
    std::ostringstream filenameStream;
    filenameStream << "track_"
        << std::put_time(&local_tm, "%Y-%m-%d_%H%M%S")
        << "." << extension;
    return filenameStream.str();
}

// Function to calculate the width of a text string
float getStringWidth(const std::string& str)
{
    float maxWidth = 0.0f;
    // Calculate the width of the item
    ImVec2 itemSize = ImGui::CalcTextSize(str.c_str());
    if (itemSize.x > maxWidth) {
        maxWidth = itemSize.x;
    }
    return maxWidth;
}

auto getUiRender()
{
    auto sdkExamplesPath = Environment::GetInstance().GetSDKExamplesPath();
    auto sdkCachePath = Environment::GetInstance().GetCachePath();

    auto dstCacheResPath = gem::FileSystem().makePath(sdkCachePath.c_str(), u"Data", u"Res/");
    gem::FileSystem().createFolder(dstCacheResPath, true);

    auto srcNMEAPath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"Interactive", u"ExportTrack", u"strasbourg_parc.nmea");
    int ret;
    if ((ret = gem::FileSystem().copyFile(srcNMEAPath, dstCacheResPath)) != gem::KNoError)
    {
        GEM_LOGE("Error copy NMEA resource (%d)", GEM_GET_API_ERROR());
    }
    srcNMEAPath = gem::FileSystem().makePath(dstCacheResPath, "strasbourg_parc.nmea");

    auto dstGPXTracksPath = gem::FileSystem().makePath(sdkCachePath, u"Data", u"Tracks/");
    gem::FileSystem().createFolder(dstGPXTracksPath, true);
    auto dstLogsPath = gem::FileSystem().makePath(dstGPXTracksPath, "GPSLogs/");
    gem::FileSystem().createFolder(dstLogsPath, true);

    auto dataSourceNMEA = gem::sense::DataSourceFactory::produceLog(srcNMEAPath);
    gem::PositionService().setDataSource(dataSourceNMEA);

    return std::bind([dataSourceNMEA, dstGPXTracksPath](gem::StrongPointer<gem::MapView> mapView)
    {
        ImGuiIO& io = ImGui::GetIO();
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoSavedSettings);

        static bool started = true;
        int millisecondFrequency = 300;
        if (started)
        {
            mapView->startFollowingPosition();
            started = false;
        }
        gem::MarkerCollectionRenderSettings markerCollectionRenderSettings;
        if (dataSourceNMEA.get()->isStopped())
        {
            ImGui::BeginDisabled(true);
            ImGui::Spacing();
            ImGui::Button("Start tracking");
            ImGui::EndDisabled();
        }
        else
        {
            static const char* outputTrackFileExtension[]{ "Gpx","Kml","Nmea","GeoJson","LatLonTxt","LonLatTxt" };
            static int selectedOutputTrackFileExtension = 0;
            float strwidth = getStringWidth(std::string("LatLonTxt"));
            ImGui::SetNextItemWidth(strwidth + ImGui::GetStyle().FramePadding.x * 2);
            std::string filename = generateFilename(outputTrackFileExtension[selectedOutputTrackFileExtension]);
            if (ImGui::Combo("Track", &selectedOutputTrackFileExtension, outputTrackFileExtension, IM_ARRAYSIZE(outputTrackFileExtension)))
            {
                filename = generateFilename(outputTrackFileExtension[selectedOutputTrackFileExtension]);
            }
            if (mapView->extensions().isTrackedPositions())
            {
                ImGui::Spacing();
                if (ImGui::Button("Stop tracking"))
                {
                    mapView->extensions().stopTrackPositions();
                }
                ImGui::Spacing();
                if (ImGui::Button("Export track"))
                {
                    auto path = mapView->extensions().getTrackedPositions();
                    if (path.second == gem::KNoError)
                    {
                        auto buff = gem::Path(path.first).exportAs(gem::EPathFileFormat(selectedOutputTrackFileExtension));
                        if (GEM_GET_API_ERROR() == gem::KNoError)
                        {
                            auto dstTrackFileName = gem::FileSystem().makePath(dstGPXTracksPath.toStdString(), filename);
                            // Open the output file in binary mode
                            std::ofstream outputFile(dstTrackFileName.toStdString(), std::ios::binary);
                            if (outputFile)
                            {
                                outputFile.write((const char*)buff.getBytes(), buff.size());
                                outputFile.close();
                            }
                        }
                    }
                }
            }
            else
            {
                ImGui::Spacing();
                if (ImGui::Button("Start tracking"))
                {
                    mapView->extensions().startTrackPositions(millisecondFrequency, markerCollectionRenderSettings);
                }
                ImGui::BeginDisabled(true);
                ImGui::Spacing();
                ImGui::Button("Export track");
                ImGui::EndDisabled();
            }
        }

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
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::ImGUI, "ExportTrack", &pTouchEventListener, getUiRender()));
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
