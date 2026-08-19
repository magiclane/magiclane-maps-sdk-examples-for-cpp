// SPDX-FileCopyrightText: 2024-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_SearchService.h>

#include "Listeners.h"

#include <imgui.h>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace
{
    // One reverse geocoding request. Each click gets its own query object, so starting
    // a new search never touches the results currently displayed - the displayed list
    // is only swapped when a query completes. This keeps the previous results (and the
    // map highlight) visible while a new query is in flight, avoiding UI flicker.
    struct ReverseGeocodingQuery
    {
        gem::Coordinates m_coords;
        gem::LandmarkList m_results;
        ProgressListener m_listener;
    };

    // Keep state alive across frames (ImGui render callback is executed every frame).
    struct ReverseGeocodingUiState
    {
        // Query currently in flight (the newest click).
        std::shared_ptr<ReverseGeocodingQuery> m_activeQuery;

        // Superseded queries: canceled, but kept alive until the engine reports them
        // finished (the engine holds raw pointers to their listener & result list).
        std::vector<std::shared_ptr<ReverseGeocodingQuery>> m_retiredQueries;

        // Last completed query - what the panel and the map highlight display.
        gem::Coordinates m_clickedCoords;
        gem::LandmarkList m_results;
        gem::String m_lastHint;
        int m_lastError = gem::KNoError;
        bool m_hasResult = false;
    };

    static std::string FormatDistance( double meters )
    {
        char buf[32];
        if( meters >= 1000. )
            std::snprintf( buf, sizeof( buf ), "%.2f km", meters / 1000. );
        else
            std::snprintf( buf, sizeof( buf ), "%.0f m", meters );
        return buf;
    }
}

// Derive from the standard touch event handler class which makes the map view interactive.
// On a clean click / tap (press & release without dragging) we reverse geocode the clicked
// position: transform the screen coordinates to WGS and search for the nearest addresses
// & POIs around that point. Dragging the map only pans it - no search is triggered.
class ReverseGeocodingTouchListener : public CTouchEventListener
{
    // A press that moves more than this many pixels is a map pan, not a click.
    static constexpr int KClickMoveTolerancePx = 5;

public:
    explicit ReverseGeocodingTouchListener( std::shared_ptr<ReverseGeocodingUiState> state )
        : m_state( std::move( state ) )
    {
    }

    void handleTouchEvent( int eventType, int pointerId, int x, int y ) override
    {
        // Keep the default behavior so the map stays interactive (pan / zoom).
        CTouchEventListener::handleTouchEvent( eventType, pointerId, x, y );

        if( !m_state )
            return;

        switch( eventType )
        {
            case gem::ETouchEvent::TE_Down:
            {
                m_downX = x;
                m_downY = y;
                m_dragging = false;
                return;
            }
            case gem::ETouchEvent::TE_Move:
            {
                if( std::abs( x - m_downX ) > KClickMoveTolerancePx || std::abs( y - m_downY ) > KClickMoveTolerancePx )
                    m_dragging = true;
                return;
            }
            case gem::ETouchEvent::TE_Up:
            {
                if( m_dragging )
                    return; // the user was panning the map - don't reverse geocode
                break;      // clean click - fall through and search
            }
            default:
                return;
        }

        auto mapView = getMapViewPointer();
        if( mapView.get() == nullptr )
            return;

        // The position the user clicked, as WGS coordinates
        auto coords = mapView->transformScreenToWgs( gem::Xy( x, y ) );

        // Supersede any query still in flight - the newest click wins. The old query
        // is canceled but kept alive until the engine reports it finished.
        if( m_state->m_activeQuery )
        {
            gem::SearchService().cancelSearch( &m_state->m_activeQuery->m_listener );
            m_state->m_retiredQueries.push_back( m_state->m_activeQuery );
        }

        // Reverse geocoding = search around a position with no text filter.
        // Results are addresses & map POIs near the given coordinates, closest first.
        // Note: the currently displayed results are deliberately left untouched here;
        // they are replaced only when this query completes (see getUiRender).
        auto query = std::make_shared<ReverseGeocodingQuery>();
        query->m_coords = coords;
        gem::SearchService().searchAroundPosition( query->m_results, &query->m_listener, coords, gem::String(),
                                                   gem::SearchPreferences().setSearchAddresses( true ).setSearchMapPOIs( true ).setMaxMatches( 10 ).setThresholdDistance( 500 ) );

        m_state->m_activeQuery = query;
    }

private:
    std::shared_ptr<ReverseGeocodingUiState> m_state;
    int m_downX = 0;
    int m_downY = 0;
    bool m_dragging = false;
};

static std::function<void( gem::StrongPointer<gem::MapView> )> getUiRender( const std::shared_ptr<ReverseGeocodingUiState>& state )
{
    return std::bind(
        [state]( gem::StrongPointer<gem::MapView> mapView )
        {
            if( !state )
                return;

            // Drop superseded queries once the engine has released them.
            auto& retired = state->m_retiredQueries;
            retired.erase( std::remove_if( retired.begin(), retired.end(),
                                           []( const std::shared_ptr<ReverseGeocodingQuery>& q )
                                           {
                                               return q->m_listener.IsFinished();
                                           } ),
                           retired.end() );

            // Poll the async search. The previous results stay on screen while a query
            // is in flight; the displayed state is swapped only on completion.
            if( state->m_activeQuery && state->m_activeQuery->m_listener.IsFinished() )
            {
                auto query = state->m_activeQuery;
                state->m_activeQuery.reset();

                state->m_hasResult = true;
                state->m_lastError = query->m_listener.GetError();
                state->m_lastHint = query->m_listener.GetHint();
                state->m_clickedCoords = query->m_coords;
                state->m_results = query->m_results;

                mapView->deactivateHighlight();
                if( state->m_lastError == gem::KNoError && !state->m_results.empty() )
                    mapView->activateHighlight( state->m_results );
            }

            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos( ImVec2( main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20 ), ImGuiCond_FirstUseEver );
            ImGui::SetNextWindowSize( ImVec2( 460, 0 ), ImGuiCond_FirstUseEver );
            ImGui::Begin( "panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings );

            ImGui::TextUnformatted( "Click on the map to reverse geocode that position." );

            // A query in flight is announced without hiding the previous results.
            if( state->m_activeQuery )
                ImGui::Text( "Searching around %.6f, %.6f ...", state->m_activeQuery->m_coords.getLatitude(), state->m_activeQuery->m_coords.getLongitude() );

            if( state->m_hasResult )
            {
                ImGui::Separator();
                ImGui::Text( "Position: %.6f, %.6f", state->m_clickedCoords.getLatitude(), state->m_clickedCoords.getLongitude() );

                if( state->m_lastError != gem::KNoError )
                {
                    ImGui::Text( "Search failed (err=%d): %s", state->m_lastError, state->m_lastHint.toStdString().c_str() );
                }
                else if( state->m_results.empty() )
                {
                    ImGui::TextUnformatted( "No address found around this position." );
                }
                else
                {
                    ImGui::Text( "Nearest addresses & places: %d", ( int ) state->m_results.size() );
                    ImGui::Separator();

                    for( int i = 0; i < ( int ) state->m_results.size(); ++i )
                    {
                        auto lmk = state->m_results[( size_t ) i];

                        const double distMeters = lmk.getCoordinates().getDistance( state->m_clickedCoords );

                        std::string name = lmk.getName().toStdString();
                        if( name.empty() )
                            name = "<unnamed>";

                        // Prefer the landmark description; fall back to the formatted address.
                        std::string details = lmk.getDescription().toStdString();
                        if( details.empty() )
                            details = lmk.getAddress().format().toStdString();

                        ImGui::Text( "%d. %s (%s)", i + 1, name.c_str(), FormatDistance( distMeters ).c_str() );
                        if( !details.empty() )
                        {
                            ImGui::Indent();
                            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.7f, 0.7f, 0.7f, 1.f ) );
                            ImGui::TextWrapped( "%s", details.c_str() );
                            ImGui::PopStyleColor( 1 );
                            ImGui::Unindent();
                        }
                    }
                }
            }

            ImGui::End();
        },
        std::placeholders::_1 );
}

int main( int argc, char** argv )
{
    // Get new project API token from:
    // https://developer.magiclane.com/api/projects
    Environment::HandleHelpOption( argc, argv );

    std::string projectApiToken = Environment::ResolveApiToken( argc, argv );

    // Sdk objects can be created & used below this line
    Environment::SdkSession session( projectApiToken, { argc > 1 && argv[1][0] != '-' ? argv[1] : "" } ); // SDK API debug logging path

    if( GEM_GET_API_ERROR() != gem::KNoError ) // check for errors after session creation
        return GEM_GET_API_ERROR();

    auto uiState = std::make_shared<ReverseGeocodingUiState>();

    // Create an interactive map view
    ReverseGeocodingTouchListener pTouchEventListener( uiState );

    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "ReverseGeocoding", &pTouchEventListener, getUiRender( uiState ) ) );
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    WAIT_UNTIL_WINDOW_CLOSE();

    // Ensure SDK objects are destroyed before SdkSession uninitializes the SDK.
    uiState.reset();
    mapView = nullptr;

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
