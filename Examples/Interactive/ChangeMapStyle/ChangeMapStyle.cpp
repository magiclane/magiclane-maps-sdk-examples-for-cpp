// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_ContentStore.h>

#include <imgui.h>

auto getUiRender()
{
    return std::bind([]( gem::StrongPointer<gem::MapView> mapView )
        {
            static bool isNightStyle = false;
            ImGuiIO& io = ImGui::GetIO();
            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_AlwaysAutoResize
                | ImGuiWindowFlags_NoSavedSettings);
            if ( ImGui::Button( "Change map style" ) )
            {
                ImGui::SameLine();
                ImGui::Text("Getting map style...");
                isNightStyle = !isNightStyle;

                ProgressListener listener;

                auto searchAndApply = [&]()
                {
                    // Get available map styles from the content store

                    auto styles = gem::ContentStore().getStoreContentList( gem::EContentType::CT_ViewStyleLowRes ).first;

                    gem::ContentStoreItem nightStyle;
                    gem::ContentStoreItem dayStyle;

                    // Find first night map style

                    for ( auto style : styles )
                    {
                        auto backGroundColor = style.getContentParameters().findParameter( "Background-Color" );
                        if ( backGroundColor && !gem::Rgba( backGroundColor.getValue<int>() ).isLight() )
                        {
                            // Night map style found - download if needed

                            if ( style.getStatus() == gem::EContentStoreItemStatus::CIS_Unavailable )
                            {
                                listener.Reset();
                                style.asyncDownload( &listener );

                                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), INT_MAX );
                            }
                            nightStyle = style;
                            break;
                        }
                    }

                    // Find first day map style

                    for ( auto style : styles )
                    {
                        auto backGroundColor = style.getContentParameters().findParameter( "Background-Color" );
                        if ( backGroundColor && gem::Rgba( backGroundColor.getValue<int>() ).isLight() )
                        {
                            // Day map style found - download if needed

                            if ( style.getStatus() == gem::EContentStoreItemStatus::CIS_Unavailable )
                            {
                                listener.Reset();
                                style.asyncDownload( &listener );

                                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), INT_MAX );
                            }
                            dayStyle = style;
                            break;
                        }
                    }

                    if ( isNightStyle && nightStyle && nightStyle.isCompleted() )
                        return GEM_TEST_NOEXCEPT(mapView->preferences().setMapStyle( nightStyle )) == gem::KNoError;
                    else if ( !isNightStyle && dayStyle && dayStyle.isCompleted() )
                        return GEM_TEST_NOEXCEPT(mapView->preferences().setMapStyle( dayStyle )) == gem::KNoError;
                    else
                        return false;
                };

                // Try with local list

                if ( !searchAndApply() )
                {
                    // Get available content store map styles

                    gem::ContentStore().asyncGetStoreContentList( gem::EContentType::CT_ViewStyleLowRes, &listener );

                    WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), INT_MAX );

                    searchAndApply();
                }

            }
            ImGui::SameLine();
            ImGui::Text("%s", isNightStyle ? "Night map style set" : "Day map style set");
            //ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
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
	if ( value != nullptr )
		projectApiToken = value;
#endif

	// Sdk objects can be created & used below this line
	Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


	// Create an interactive map view
	CTouchEventListener pTouchEventListener;

	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce( session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "ChangeMapStyle", &pTouchEventListener, getUiRender()));
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
