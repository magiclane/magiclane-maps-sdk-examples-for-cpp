// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"
#include "BitmapImpl.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>
#include <API/GEM_NavigationListener.h>
#include <API/GEM_NavigationService.h>

#include <imgui.h>

namespace
{

////////////////////////////////////////////////////////////////////////
class MyNavigationListener : public gem::INavigationListener
{
private:
    const ImVec2 INSTRUCTION_ICON_SIZE{70, 70};

    void onNavigationStarted() override
    {
        GEM_LOGI( "Simulation started" );
    }
    void onNavigationInstructionUpdated( const gem::NavigationInstruction &navinstruct ) override
    {
        GEM_LOGE("NAV INSTRUCTION ( E N T R Y P O I N T ) %%%%%%%%%%%%%%%%%");
        gem::String instructionText = navinstruct.getNextTurnInstruction();
        instructionText.fallbackToLegacyUnicode();

        if (navinstruct && !navinstruct.isDefault())
        {
            GEM_LOGE("NAV INSTRUCTION ( %s ) %%%%%%%%%%%%%%%%%", instructionText.toStdString().c_str());
        }
        else if (navinstruct && navinstruct.isDefault())
        {
            GEM_LOGE("NAV INSTRUCTION ( D E F A U L T ) %%%%%%%%%%%%%%%%%");
        }
        else if (!navinstruct)
        {
            GEM_LOGE("NAV INSTRUCTION ( N U L L ) %%%%%%%%%%%%%%%%%");
        }
        else
        {
            GEM_LOGE("NAV INSTRUCTION ( O T H E R ) %%%%%%%%%%%%%%%%%");
        }
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

#define NEXT_TURN_ICON_SIZE_PIXELS 80

auto getUiRender()
{
    return std::bind([](gem::StrongPointer<gem::MapView> mapView)
        {
            ImGuiIO& io = ImGui::GetIO();
            static bool iskm = true;
            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 120), ImGuiCond_FirstUseEver);
            ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_AlwaysAutoResize
                | ImGuiWindowFlags_NoSavedSettings);
            auto navListener = gem::StrongPointerFactory<MyNavigationListener>();
            ImGui::PushFont(io.Fonts->Fonts[0]);
            if (!gem::NavigationService().isNavigationActive()
                && !gem::NavigationService().isSimulationActive())
            {
                ImGui::Spacing();
                if (ImGui::Button("Simulate navigation on a route"))
                {
                    // At least 2 waypoints define the route, the first is the departure position, and the last is the destination.
                    // There can be zero or more intermediate waypoints through which the route passes in the order they are listed.
                    // The coordinates are {latitude,longitude} in degrees; the landmark name is optional and can be an empty string.
                    gem::LandmarkList waypoints({ { "San Francisco", { 37.77903, -122.41991 } }, { "San Jose", { 37.33619, -121.89058 } } });

                    // Compute route using these preferences: car / fastest / without alternatives in result
                    gem::RouteList routes;
                    ProgressListener routeListener;
                    gem::RoutingService().calculateRoute(routes, waypoints, gem::RoutePreferences().setTransportMode(gem::RTM_Car).setRouteType(gem::RT_Fastest).setAlternativesSchema(gem::AS_Never), &routeListener);

                    // Wait until route calculation finished & check success
                    if (WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &routeListener), 30000) && routeListener.GetError() == gem::KNoError && !routes.empty())
                    {
                        // Add the first resulting route (at index 0) to map view
                        mapView->preferences().routes().add(routes[0]);

                        // Start simulated navigation along the route
                        gem::NavigationService().startSimulation(routes[0], navListener, gem::ProgressListener());

                        // Start follow GPS positions ( generated by the simulation ) - camera follows the position along the route
                        mapView->startFollowingPosition();
                    }
                }
            }
            if (!mapView->isFollowingPosition())
            {
                ImGui::Spacing();
                if (ImGui::Button("Follow position"))
                {
                    mapView->startFollowingPosition();
                }
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 1.f, 1.f));
                ImGui::PushFont(io.Fonts->Fonts[1]);
                ImGui::Text("Following position!");
                ImGui::PopFont();
                ImGui::PopStyleColor(1);
            }
            if (iskm)
            {
                ImGui::Spacing();
                if (ImGui::Button("Switch to miles"))
                {
                    iskm = false;
                }
            }
            else
            {
                ImGui::Spacing();
                if (ImGui::Button("Switch to kilometers"))
                {
                    iskm = true;
                }
            }
            if (gem::NavigationService().isNavigationActive()
                || gem::NavigationService().isSimulationActive())
            {
                ImGui::Spacing();
                if (ImGui::Button("Stop navigation"))
                    gem::NavigationService().cancelNavigation(navListener);
            }
            ImGui::PopFont();
            ImGui::End();

            //////////////////////////////////////////////////////////
            // TOP STATUS PANEL
            //////////////////////////////////////////////////////////
            if (gem::NavigationService().isNavigationActive()
                || gem::NavigationService().isSimulationActive())
            {
                ImGui::SetNextWindowBgAlpha(1.0f);
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 0), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, 108));
                ImGui::GetStyle().WindowRounding = 0.0f;
                ImGui::Begin("panel2", nullptr, ImGuiWindowFlags_NoMove
                    | ImGuiWindowFlags_NoDecoration
                    | ImGuiWindowFlags_AlwaysAutoResize
                    //| ImGuiWindowFlags_NoBackground
                    | ImGuiWindowFlags_NoSavedSettings);
                const ImU32 colorblack = ImGui::GetColorU32(ImVec4(0, 0, 0, 1));
                ImVec2 wPos = ImGui::GetWindowPos();
                ImVec2 wSize = ImGui::GetWindowSize();
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(wPos.x + 0, wPos.y + 0), ImVec2(wSize.x, wSize.y), colorblack);
                auto navinstruct = gem::NavigationService().getNavigationInstruction();
                gem::String instructionText = navinstruct.getNextTurnInstruction();
                instructionText.fallbackToLegacyUnicode();

                if (ImGui::BeginTable("top_instruction_panel", 2))
                {
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, NEXT_TURN_ICON_SIZE_PIXELS + 12);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, wSize.x - 12 - NEXT_TURN_ICON_SIZE_PIXELS);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);

                    //////////////////////////////////////////////////////////
                    // TURN ICON
                    //////////////////////////////////////////////////////////
                    {
                        const ImVec2 nextTurnIconSize{ NEXT_TURN_ICON_SIZE_PIXELS, NEXT_TURN_ICON_SIZE_PIXELS };
                        gem::Rgba color(255, 0, 255, 255);
                        gem::AbstractGeometryImageRenderSettings settings(gem::Rgba::white(), gem::Rgba::black(), color);
                        auto bitmap = gem::StrongPointerFactory<BitmapImpl>(70, 70);
                        navinstruct.getNextTurnDetails().getAbstractGeometryImage().render(*bitmap, settings);
                        unsigned int textureId = BitmapImpl::LoadTextureIntoGPU(bitmap->size().width, bitmap->size().height, bitmap->begin());
                        ImGui::Image(textureId, nextTurnIconSize);
                    }

                    ImGui::TableSetColumnIndex(1);

                    //////////////////////////////////////////////////////////
                    // NEXT STREET NAME
                    //////////////////////////////////////////////////////////
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::PushFont(io.Fonts->Fonts[2]);
                    ImGui::Text("%s", navinstruct.getNextStreetName().toStdString().c_str());
                    ImGui::PopFont();
                    ImGui::PopStyleColor(1);

                    //////////////////////////////////////////////////////////
                    // NEXT TURN INSTRUCTION TEXT
                    //////////////////////////////////////////////////////////
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
                    ImGui::PushFont(io.Fonts->Fonts[1]);
                    ImGui::Text("%s", instructionText.toStdString().c_str());

                    auto remainingDistanceToNextTurn = navinstruct.getTimeDistanceToNextTurn().getTotalDistance();
                    auto remainingTimeToNextTurn = navinstruct.getTimeDistanceToNextTurn().getTotalTime();

                    ImGui::SameLine();
                    ImGui::Text("   ");
                    ImGui::SameLine();

                    //////////////////////////////////////////////////////////
                    // NEXT TURN INSTRUCTION DISTANCE
                    //////////////////////////////////////////////////////////
                    if (iskm)
                    {
                        if (remainingDistanceToNextTurn > 1000)
                        {
                            ImGui::Text("%.1f ", (float)(remainingDistanceToNextTurn * .001));
                            ImGui::SameLine();
                            ImGui::Text("km ");
                        }
                        else
                        {
                            ImGui::Text("%d ", (int)(remainingDistanceToNextTurn));
                            ImGui::SameLine();
                            ImGui::Text("m ");
                        }
                        ImGui::SameLine();
                    }
                    else
                    {
                        if ((remainingDistanceToNextTurn / 1609.344 * 5280) > 5280)
                        {
                            ImGui::Text("%.1f ", (float)((remainingDistanceToNextTurn / 1609.344 * 5280)/5280.0));
                            ImGui::SameLine();
                            ImGui::Text("mi ");
                        }
                        else
                        {
                            ImGui::Text("%d ", (int)(remainingDistanceToNextTurn / 1609.344 * 5280));
                            ImGui::SameLine();
                            ImGui::Text("ft ");
                        }
                        ImGui::SameLine();
                    }
                    ImGui::PopFont();
                    ImGui::PopStyleColor(1);

                    ImGui::EndTable();

                } // end table

                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                ImGui::End();

                //////////////////////////////////////////////////////////
                // BOTTOM STATUS PANEL
                //////////////////////////////////////////////////////////
                {
                    ImGui::SetNextWindowBgAlpha(1.0f);
                    ImGui::SetNextWindowPos(ImVec2(main_viewport->Pos.x + 20, main_viewport->Pos.y + main_viewport->WorkSize.y - 72));
                    ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x - 40, 64));
                    ImGui::GetStyle().WindowRounding = 12.0f;
                    ImGui::Begin("panel4", nullptr, ImGuiWindowFlags_NoMove
                        | ImGuiWindowFlags_NoDecoration
                        //| ImGuiWindowFlags_AlwaysAutoResize
                        //| ImGuiWindowFlags_NoBackground
                        | ImGuiWindowFlags_NoSavedSettings);
                    ImVec4 mycolor0{ 0,0,0,255 };
                    ImVec4 mycolor1{ 255,0,0,255 };
                    ImVec4 mycolor2{ 0,255,0,255 };
                    ImVec4 mycolor3{ 0,0,255,255 };
                    ImVec4 mycolor7{ 255,255,255,255 };

                    auto remainingTravelTime = navinstruct.getRemainingTravelTimeDistance().getTotalTime();
                    auto remainingTravelDistance = navinstruct.getRemainingTravelTimeDistance().getTotalDistance();

                    time_t currentTimeSec = (time_t)(gem::Time::getLocalTime().asInt() / 1000);
                    time_t eta = (time_t)(currentTimeSec + remainingTravelTime);
                    struct tm tmcurrent, etatm;
                    struct tm* tmptr = gmtime((const time_t*)&(currentTimeSec));
                    memcpy(&tmcurrent, tmptr, sizeof(struct tm));
                    tmptr = gmtime((const time_t*)&(eta));
                    memcpy(&etatm, tmptr, sizeof(struct tm));
                    /*
                    //////////////////////////////////////////////////////////
                    // CURRENT TIME
                    //////////////////////////////////////////////////////////
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 1.f, 1.f));
                    //ImGui::SetWindowFontScale(2);
                    //auto rpos = ImGui::GetCursorScreenPos();
                    ImGui::PushFont(io.Fonts->Fonts[1]);
                    ImGui::Text("%02d:%02d    ", tmcurrent.tm_hour, tmcurrent.tm_min);
                    ImGui::PopFont();
                    ImGui::SameLine();
                    ImGui::PopStyleColor(1);
                    */

                    //////////////////////////////////////////////////////////
                    // REMAINING TRAVEL HOURS, MIN, SEC: PRINT ONLY HOURS:MIN IF >= 1H OR MIN:SEC IF < 1H
                    //////////////////////////////////////////////////////////
                    //ImGui::Text("Time %d:%02d:%02d", (int)(remainingTravelTime / 3600), (int)((remainingTravelTime % 3600) / 60), (int)(remainingTravelTime % 60));
                    int remainingTravelHours = (int)(remainingTravelTime / 3600);
                    int remainingTravelMin = (int)((remainingTravelTime % 3600) / 60);
                    int remainingTravelSec = (int)(remainingTravelTime % 60);

                    //////////////////////////////////////////////////////////
                    // COMPUTE BOTTOM STAT STR LENGTH TO ENABLE CENTERING STR HORIZONTALLY AND VERTICALLY!
                    //////////////////////////////////////////////////////////
                    ImGui::PushFont(io.Fonts->Fonts[0]);
                    float font_size1 = ImGui::GetFontSize();
                    auto textWidth1 = ImGui::CalcTextSize("ETAminkm").x;
                    ImGui::PopFont();
                    ImGui::PushFont(io.Fonts->Fonts[1]);
                    float font_size2 = ImGui::GetFontSize();
                    auto textWidth2 = ImGui::CalcTextSize("00:00   00").x;
                    auto textHeight2 = ImGui::CalcTextSize("00").y;
                    ImGui::PopFont();
                    if (remainingTravelHours > 0)
                    {
                        int digitsInHours = (int)(log10(remainingTravelHours)) + 1;
                        ImGui::PushFont(io.Fonts->Fonts[1]);
                        textWidth2 += digitsInHours * ImGui::CalcTextSize("0").x;
                        ImGui::PopFont();
                        ImGui::PushFont(io.Fonts->Fonts[0]);
                        textWidth1 += ImGui::CalcTextSize("h ").x;
                        ImGui::PopFont();

                    }
                    else
                    {
                        ImGui::PushFont(io.Fonts->Fonts[1]);
                        textWidth2 += ImGui::CalcTextSize(" 00").x;
                        ImGui::PopFont();
                    }
                    int digitsInMeters = (int)(log10(remainingTravelDistance)) + 1;
                    if (remainingTravelDistance > 1000)
                    {
                        digitsInMeters -= 1;//printed %.1f
                    }
                    ImGui::PushFont(io.Fonts->Fonts[1]);
                    textWidth2 += digitsInMeters * ImGui::CalcTextSize("0").x;
                    ImGui::PopFont();
                    ImGui::PushFont(io.Fonts->Fonts[0]);
                    textWidth1 += ImGui::CalcTextSize("km").x;
                    ImGui::PopFont();

                    ImGui::SetCursorScreenPos(ImVec2((ImGui::GetWindowSize().x - textWidth2 - textWidth1) * 0.4f + ImGui::GetWindowPos().x,
                        (ImGui::GetWindowSize().y - textHeight2) * 0.5f + ImGui::GetWindowPos().y));

                    //////////////////////////////////////////////////////////
                    // ETA
                    //////////////////////////////////////////////////////////
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
                    //auto rpos = ImGui::GetCursorScreenPos();
                    //ImGui::SetCursorScreenPos(ImVec2(rpos.x, rpos.y + 12));
                    ImGui::PushFont(io.Fonts->Fonts[0]);
                    ImGui::Text("ETA");
                    ImGui::PopFont();
                    ImGui::SameLine();
                    ImGui::PushFont(io.Fonts->Fonts[1]);
                    ImGui::Text("%02d:%02d   ", etatm.tm_hour, etatm.tm_min);
                    ImGui::PopFont();
                    ImGui::SameLine();
                    ImGui::PopStyleColor(1);

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, .5f, 0.f, 1.f));
                    if (remainingTravelHours > 0)
                    {
                        ImGui::PushFont(io.Fonts->Fonts[1]);
                        ImGui::Text("%d", remainingTravelHours);
                        ImGui::PopFont();
                        ImGui::SameLine();
                        ImGui::PushFont(io.Fonts->Fonts[0]);
                        ImGui::Text("h ");
                        ImGui::PopFont();
                        ImGui::SameLine();
                    }
                    {
                        ImGui::PushFont(io.Fonts->Fonts[1]);
                        ImGui::Text("%02d", remainingTravelMin);
                        ImGui::PopFont();
                        ImGui::SameLine();
                        ImGui::PushFont(io.Fonts->Fonts[0]);
                        ImGui::Text("min");
                        ImGui::PopFont();
                        ImGui::SameLine();
                    }
                    if (remainingTravelHours == 0)
                    {
                        ImGui::PushFont(io.Fonts->Fonts[1]);
                        ImGui::Text(" %02d", remainingTravelSec);
                        ImGui::PopFont();
                        ImGui::SameLine();
                    }
                    ImGui::PopStyleColor(1);

                    ImGui::PushFont(io.Fonts->Fonts[1]);
                    ImGui::Text("   ");
                    ImGui::PopFont();
                    ImGui::SameLine();

                    //////////////////////////////////////////////////////////
                    // REMAINING TRAVEL DISTANCE KM
                    //////////////////////////////////////////////////////////
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 1.f, 1.f));
                    if (iskm)
                    {
                        if (remainingTravelDistance > 1000)
                        {
                            ImGui::PushFont(io.Fonts->Fonts[1]);
                            ImGui::Text("%.1f", (float)(remainingTravelDistance * .001));
                            ImGui::PopFont();
                            ImGui::SameLine();
                            ImGui::PushFont(io.Fonts->Fonts[0]);
                            ImGui::Text("km");
                            ImGui::PopFont();
                        }
                        else
                        {
                            ImGui::PushFont(io.Fonts->Fonts[1]);
                            ImGui::Text("%.1f", (float)(remainingTravelDistance));
                            ImGui::PopFont();
                            ImGui::SameLine();
                            ImGui::PushFont(io.Fonts->Fonts[0]);
                            ImGui::Text("m");
                            ImGui::PopFont();
                        }
                    }
                    else
                    {
                        if (remainingTravelDistance > 1000)
                        {
                            ImGui::PushFont(io.Fonts->Fonts[1]);
                            ImGui::Text("%.1f", (float)(remainingTravelDistance / 1609.344));
                            ImGui::PopFont();
                            ImGui::SameLine();
                            ImGui::PushFont(io.Fonts->Fonts[0]);
                            ImGui::Text("mi");
                            ImGui::PopFont();
                        }
                        else
                        {
                            ImGui::PushFont(io.Fonts->Fonts[1]);
                            ImGui::Text("%.1f", (float)(remainingTravelDistance / 1609.344 * 5280));
                            ImGui::PopFont();
                            ImGui::SameLine();
                            ImGui::PushFont(io.Fonts->Fonts[0]);
                            ImGui::Text("ft");
                            ImGui::PopFont();
                        }
                    }
                    ImGui::PopStyleColor(1);

                    colors[ImGuiCol_WindowBg] = mycolor7;
                    ImGui::End();
                }
            }
        }
    , std::placeholders::_1);
}

} //namespace

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
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::ImGUI, "SimulateNavigation", &pTouchEventListener, getUiRender()));
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
