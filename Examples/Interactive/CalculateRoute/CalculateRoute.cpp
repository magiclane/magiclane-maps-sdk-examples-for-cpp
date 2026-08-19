// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"
#include "ExampleArgs.h"
#include "LvglTheme.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>

#include <imgui.h>

#include <atomic>
#include <mutex>
#include <string>
#include <cstring>

namespace
{
    // Selected backend (set from argv in main)
    Environment::WindowFrameworks g_uiBackend = Environment::WindowFrameworks::LVGL;

    // Function to perform route calculation and add result to map view, used by both ImGui and LVGL UIs.
    void DoCalculateRouteOnMap( gem::StrongPointer<gem::MapView> mapView )
    {
        // At least 2 waypoints define the route, the first is the departure position, and the last is the destination.
        // There can be zero or more intermediate waypoints through which the route passes in the order they are listed.
        // The coordinates are {latitude,longitude} in degrees; the landmark name is optional and can be an empty string.
        gem::LandmarkList waypoints( { { "San Francisco", { 37.77903, -122.41991 } }, { "San Jose", { 37.33619, -121.89058 } } } );

        gem::RouteList routes;
        ProgressListener routeListener;

        // Calculate route - car / fastest / without alternatives in result
        gem::RoutingService().calculateRoute(
            routes, waypoints, gem::RoutePreferences().setTransportMode( gem::RTM_Car ).setRouteType( gem::RT_Fastest ).setAlternativesSchema( gem::AS_Never ), &routeListener );

        // Wait until route calculation finished & check success
        if( WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &routeListener ), 30000 ) && routeListener.GetError() == gem::KNoError && !routes.empty() )
        {
            GEM_LOGI( "Route calculated successfully (%d route(s)), rendering the first one", ( int ) routes.size() );

            // Add resulting route to map view
            mapView->preferences().routes().add( routes[0] );
            mapView->centerOnRoute( routes[0], gem::Rect(), gem::Animation( gem::AnimationLinear, gem::ProgressListener(), 2000 ) );
        }
        else
        {
            GEM_LOGE( "Route calculation failed (err=%d) - nothing to render", routeListener.GetError() );
        }
    }


    // Shared by both UIs, so they report the same thing
    constexpr const char* kStatusIdle = "Status: idle";
    constexpr const char* kStatusBusy = "Status: calculating...";
    constexpr const char* kStatusDone = "Status: done";
    const char* g_status = kStatusIdle;

    // ImGui UI callbacks
    auto createImGuiUiCallbacks()
    {
        return std::bind(
            []( gem::StrongPointer<gem::MapView> mapView )
            {
                const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos( ImVec2( main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20 ), ImGuiCond_FirstUseEver );
                ImGui::Begin( "panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings );
                if( ImGui::Button( "Calculate route" ) )
                {
                    // NOTE: this blocks the render thread; the status label
                    // below is what the LVGL UI shows too
                    g_status = kStatusBusy;
                    DoCalculateRouteOnMap( mapView );
                    g_status = kStatusDone;
                }
                ImGui::TextUnformatted( g_status );
                ImGui::End();
            },
            std::placeholders::_1 );
    }

    // LVGL UI objects
    lv_obj_t* g_panel = nullptr;
    lv_obj_t* g_btnCalc = nullptr;
    lv_obj_t* g_lblBtn = nullptr;
    lv_obj_t* g_lblStatus = nullptr;

    // Route request state (set by LVGL event, consumed from UI update callback)
    std::atomic<bool> g_calcRequested { false };

    void btn_calc_event_cb( lv_event_t* e )
    {
        if( lv_event_get_code( e ) != LV_EVENT_CLICKED )
            return;

        // Keep the callback minimal/fast; do work from the per-frame callback.
        g_calcRequested.store( true );
    }

    bool InitLvglUi()
    {
        // Dark Magic Lane look, shared with the other LVGL examples
        LvglTheme::Apply();

        lv_obj_t* scr = lv_screen_active();

        // Floating panel; the height follows the content, like the auto-sized
        // ImGui window above
        g_panel = lv_obj_create( scr );
        lv_obj_set_size( g_panel, 232, LV_SIZE_CONTENT );
        lv_obj_set_pos( g_panel, 10, 10 );
        LvglTheme::StylePanel( g_panel );

        g_btnCalc = lv_button_create( g_panel );
        lv_obj_set_size( g_btnCalc, LV_PCT( 100 ), 40 );
        lv_obj_set_pos( g_btnCalc, 0, 0 );
        LvglTheme::StylePrimaryWidget( g_btnCalc );
        lv_obj_add_event_cb( g_btnCalc, btn_calc_event_cb, LV_EVENT_CLICKED, nullptr );

        g_lblBtn = lv_label_create( g_btnCalc );
        lv_label_set_text( g_lblBtn, "Calculate route" );
        lv_obj_center( g_lblBtn );

        g_lblStatus = lv_label_create( g_panel );
        lv_label_set_text( g_lblStatus, kStatusIdle );
        lv_obj_set_pos( g_lblStatus, 0, 52 );

        return true;
    }

    void UpdateLvglUi( gem::StrongPointer<gem::MapView> mapView )
    {
        // Handle click-triggered route calculation
        if( g_calcRequested.exchange( false ) )
        {
            g_status = kStatusBusy;
            if( g_lblStatus )
                lv_label_set_text( g_lblStatus, g_status );

            // NOTE: this blocks the render thread.
            // If you want non-blocking UI, move this to a worker thread and just update labels from UpdateLvglUi().
            DoCalculateRouteOnMap( mapView );

            g_status = kStatusDone;
            if( g_lblStatus )
                lv_label_set_text( g_lblStatus, g_status );

            // Ensure LVGL redraws the changed labels
            lv_obj_invalidate( lv_screen_active() );
        }
    }

    // LVGL UI callbacks
    UICallbacks createLvglUiCallbacks()
    {
        return UICallbacks(
            []() -> bool
            {
                return InitLvglUi();
            },
            []( gem::StrongPointer<gem::MapView> mapView )
            {
                UpdateLvglUi( mapView );
            } );
    }

    int g_rotation = 0;

    gem::OpenGLContext ProduceContext( Environment::SdkSession& session, const char* title, CTouchEventListener* touchListener )
    {
        switch( g_uiBackend )
        {
            case Environment::WindowFrameworks::ImGUI:
                // IMPORTANT: ensure the overload you call matches what your Environment provides for ImGUI
                // (some codebases use a render functor instead of UICallbacks).
                return session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, title, touchListener, createImGuiUiCallbacks(), gem::Size( 0, 0 ), g_rotation );

            case Environment::WindowFrameworks::LVGL:
            default:
                return session.produceOpenGLContext( Environment::WindowFrameworks::LVGL, title, touchListener, createLvglUiCallbacks(), gem::Size( 0, 0 ), g_rotation );
        }
    }
}

int main( int argc, char** argv )
{
    // Parse shared args
    g_uiBackend = ExampleArgs::ParseUiBackend( argc, argv, Environment::WindowFrameworks::LVGL );
    g_rotation = ExampleArgs::ParseRotation( argc, argv, 0 );

    // Get new project API token from:
    // https://developer.magiclane.com/api/projects
    Environment::HandleHelpOption( argc, argv );

    std::string projectApiToken = Environment::ResolveApiToken( argc, argv );

    // Sdk objects can be created & used below this line
    Environment::SdkSession session( projectApiToken, { argc > 1 && argv[1][0] != '-' ? argv[1] : "" } ); // SDK API debug logging path

    if( GEM_GET_API_ERROR() != gem::KNoError ) // check for errors after session creation
        return GEM_GET_API_ERROR();

    // Create an interactive map view
    CTouchEventListener touchEventListener;

    gem::OpenGLContext glContext = ProduceContext( session, "CalculateRoute", &touchEventListener );

    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce( glContext );

    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    WAIT_UNTIL_WINDOW_CLOSE();

    return 0;
}

#if ( defined( _WIN32 ) || defined( _WIN64 ) ) && !defined( __MINGW32__ ) && !defined( __MINGW64__ )

int WINAPI WinMain( HINSTANCE hInstance,     // Instance
                    HINSTANCE hPrevInstance, // Previous Instance
                    LPSTR lpCmdLine,         // Command Line Parameters
                    int nCmdShow )
{
    main( 0, nullptr );

    return 0;
}

#endif
