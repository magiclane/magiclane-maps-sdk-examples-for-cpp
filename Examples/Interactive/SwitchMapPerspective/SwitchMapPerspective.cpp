// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>

#include <imgui.h>

auto getUiRender()
{
    return std::bind([](gem::StrongPointer<gem::MapView> mapView)
    {
            static bool is3DmapPerspective = false;
            ImGuiIO& io = ImGui::GetIO();
            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_AlwaysAutoResize
                | ImGuiWindowFlags_NoSavedSettings);
            if (ImGui::Button("Change map perspective"))
            {
                is3DmapPerspective = !is3DmapPerspective;
                mapView->preferences().setMapViewPerspective(is3DmapPerspective ? gem::MVP_3D
                    : gem::MVP_2D, gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
            }
            ImGui::SameLine();
            ImGui::Text("%s", is3DmapPerspective ? "3D map perspective set" : "2D map perspective set");
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

	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::ImGUI, "MapPerspective", &pTouchEventListener, getUiRender()));
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
