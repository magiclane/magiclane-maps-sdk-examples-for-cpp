// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_SenseDataTypes.h>
#include <API/GEM_SenseDataSource.h>
#include <API/GEM_Debug.h>
#include <API/Extensions/GEM_Recorder.h>
#include <API/Extensions/GEM_Utils.h>

#include <imgui.h>

class RecorderListener : public gem::IProgressListener
{
public:
    RecorderListener( const gem::StringRef dstGPXTracksPath )
    {
        gpxRecorderBookmarks = gem::RecorderBookmarks::produce( dstGPXTracksPath );
    }

    void notifyStart( bool hasProgress ) override
    {
    }

    void notifyComplete( int reason, gem::String recordPath ) override
    {
        GEM_LOGI( "Recorder stopped with code = %d", reason );

        if( reason == gem::KNoError )
        {
            if( auto error = gpxRecorderBookmarks->exportLog( recordPath, gem::EFileType::Gpx ) == gem::KNoError )
            {
                GEM_LOGI( "Successfully exported log %s to gpx.", recordPath);
            }
            else
            {
                GEM_LOGE( "Could not export log %s to gpx. Error code = %d", recordPath, error );
            }
        }
    }

private:
    gem::StrongPointer<gem::RecorderBookmarks> gpxRecorderBookmarks;
};

auto getUiRender()
{
    auto sdkExamplesPath = Environment::GetInstance().GetSDKExamplesPath();
    auto sdkCachePath = Environment::GetInstance().GetCachePath();

    auto dstCacheResPath = gem::FileSystem().makePath(sdkCachePath.c_str(), u"Data", u"Res/");
    gem::FileSystem().createFolder(dstCacheResPath, true);

    auto srcNMEAPath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"Interactive", u"SaveGPX", u"strasbourg.nmea");
    int ret;
    if ((ret = gem::FileSystem().copyFile(srcNMEAPath, dstCacheResPath)) != gem::KNoError)
    {
        GEM_LOGE("Error copy NMEA resource (%d)", GEM_GET_API_ERROR());
    }
    srcNMEAPath = gem::FileSystem().makePath(dstCacheResPath, "strasbourg.nmea");

    auto dstGPXTracksPath = gem::FileSystem().makePath(sdkCachePath, u"Data", u"Tracks/");
    gem::FileSystem().createFolder(dstGPXTracksPath, true);

    auto dstLogsPath = gem::FileSystem().makePath(dstGPXTracksPath, "GPSLogs/");
    gem::FileSystem().createFolder(dstLogsPath, true);

    auto dataSource = gem::sense::DataSourceFactory::produceLog(srcNMEAPath);
    gem::PositionService().setDataSource( dataSource );

    gem::sense::DataTypeList datatypes;
    datatypes.push_back(gem::sense::EDataType::Position);
    datatypes.push_back(gem::sense::EDataType::ImprovedPosition);

    gem::RecorderConfigurationPtr gpxRecorderConfigs = gem::StrongPointerFactory<gem::RecorderConfiguration>();
    gpxRecorderConfigs->logsDir = dstLogsPath;
    gpxRecorderConfigs->dataSource = dataSource;
    gpxRecorderConfigs->recordedTypes = datatypes;
    gpxRecorderConfigs->minDurationSeconds = 10;
    gpxRecorderConfigs->chunkDurationSeconds = 60;
    gpxRecorderConfigs->bContinuousRecording = true;
    gpxRecorderConfigs->deleteOlderThanKeepMin = false;
    gpxRecorderConfigs->keepMinSeconds = 3600;

    gem::StrongPointer<RecorderListener> recorderListener = gem::StrongPointerFactory<RecorderListener>(dstGPXTracksPath);

    gem::StrongPointer<gem::Recorder> gpxRecorder = gem::Recorder::produce(gpxRecorderConfigs);
    if( gpxRecorder )
    {
        gpxRecorder->addListener( recorderListener );
    }

    return std::bind([dataSource, gpxRecorder](gem::StrongPointer<gem::MapView> mapView)
    {
        ImGuiIO& io = ImGui::GetIO();
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoSavedSettings);
        if (gpxRecorder)
        {
            auto recstat = gpxRecorder.get()->getStatus();
            char* str = (char*)"UNDEFINED";
            switch (recstat)
            {
            case gem::ERecorderStatus::Restarting:
                str = (char*)"gem::ERecorderStatus::Restarting";
                break;
            case gem::ERecorderStatus::Starting:
                str = (char*)"gem::ERecorderStatus::Starting";
                break;
            case gem::ERecorderStatus::Recording:
                str = (char*)"gem::ERecorderStatus::Recording";
                break;
            case gem::ERecorderStatus::Stopped:
                str = (char*)"gem::ERecorderStatus::Stopped";
                break;
            case gem::ERecorderStatus::Stopping:
                str = (char*)"gem::ERecorderStatus::Stopping";
                break;
            }
            GEM_LOGE("Recorder status: %s ( %d )", str, recstat);

            bool recStopped = gem::ERecorderStatus::Stopped == gpxRecorder.get()->getStatus();
            ImGui::Spacing();
            ImGui::BeginDisabled(!recStopped);
            if (ImGui::Button("Start recording"))
            {
                gpxRecorder.get()->startRecording();
                mapView->startFollowingPosition();
            }
            ImGui::EndDisabled();

            bool recRecording = gem::ERecorderStatus::Recording == gpxRecorder.get()->getStatus();
            ImGui::Spacing();
            ImGui::BeginDisabled(!recRecording);
            if (ImGui::Button("Stop recording"))
            {
                gpxRecorder.get()->stopRecording();
            }
            ImGui::EndDisabled();
        }

        bool shouldFollowPosition = gem::PositionService().getDataSource() ? !mapView->isFollowingPosition() : false;
        ImGui::Spacing();
        ImGui::BeginDisabled(!shouldFollowPosition);
        if (ImGui::Button("Follow position"))
        {
            mapView->startFollowingPosition();
        }
        ImGui::EndDisabled();

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
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::ImGUI, "SaveGPX", &pTouchEventListener, getUiRender()));
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
