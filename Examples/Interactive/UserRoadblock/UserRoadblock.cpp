// SPDX-FileCopyrightText: 2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

// Define a multi-point user roadblock interactively on the map.
//
// The interaction is:
//   1. arm the definition from the UI panel
//   2. double click on the map to drop the roadblock start point
//   3. move the mouse: TrafficService::getPersistentRoadblockPathPreview() snaps the
//      cursor onto the road network and returns the path from the last confirmed point
//      to it, which is drawn as a polyline sketch - a rubber band
//   4. double click again to confirm the previewed point ( the confirmed coordinate is
//      the road match returned by the preview, not the raw screen position )
//   5. finish the definition: all confirmed points are committed as ONE persistent user
//      roadblock through TrafficService::addPersistentRoadblock()
//
// The demo route is recalculated after every change so the effect of the roadblock on
// routing is visible: block a street the route uses and watch it go around.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>
#include <API/GEM_Traffic.h>

#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>

namespace
{
    // Demo route, San Francisco: Embarcadero -> Castro. Dense street grid, so there is
    // always a way around a blocked street.
    constexpr double kDepartureLat = 37.79331;
    constexpr double kDepartureLon = -122.39381;
    constexpr double kDestinationLat = 37.76575;
    constexpr double kDestinationLon = -122.43486;

    // Transport modes offered by the panel.
    const char* const kTransportModeNames[] = { "Car", "Lorry", "Pedestrian", "Bicycle" };
    const gem::ERouteTransportMode kTransportModes[] = { gem::RTM_Car, gem::RTM_Lorry, gem::RTM_Pedestrian, gem::RTM_Bicycle };
    constexpr int kTransportModesCount = int( sizeof( kTransportModes ) / sizeof( kTransportModes[0] ) );

    // A hand-drawn roadblock is valid from now and expires one day later.
    std::pair<gem::Time, gem::Time> GetRoadblockLifetime()
    {
        gem::Time start, expire;
        start.setUniversalTime();
        expire.setUniversalTime();
        expire += 86400 * 1000; // one day, in milliseconds

        return { start, expire };
    }

    // Interactive multi-point user roadblock definition.
    class CUserRoadblockDefinition
    {
    public:
        struct TRoadblockInfo
        {
            std::string id;
            int points = 0;
        };

        bool IsArmed() const
        {
            return m_armed;
        }
        bool IsDefining() const
        {
            return m_defining;
        }
        bool IsActive() const
        {
            return m_armed || m_defining;
        }
        int GetPointsCount() const
        {
            return int( m_coords.size() );
        }
        const std::string& GetStatus() const
        {
            return m_status;
        }
        const std::vector<TRoadblockInfo>& GetRoadblocks() const
        {
            return m_roadblocks;
        }
        int GetTransportModeIndex() const
        {
            return m_transportModeIdx;
        }
        void SetTransportModeIndex( int index )
        {
            // The transport mode is part of the roadblock definition, so it is frozen
            // while a definition is in progress.
            if( !m_defining && index >= 0 && index < kTransportModesCount )
                m_transportModeIdx = index;
        }

        // Arm the definition: the next double click on the map drops the start point.
        void Arm()
        {
            // User roadblocks are part of the navigation capability.
            if( !( gem::Sdk::getCapabilities() & gem::SC_Navigation ) )
            {
                m_status = "Missing navigation capability - user roadblocks unavailable";
                return;
            }

            m_armed = true;
            m_status = "Double click on the map to place the roadblock start point";
        }

        // Every double click on the map is routed here while a definition is active.
        void OnMapDoubleClick( std::shared_ptr<gem::MapView> mapView, const gem::Xy& point )
        {
            if( m_defining )
                ConfirmPoint( mapView, point );
            else if( m_armed )
                Start( mapView, point );
        }

        // The mouse moved over the map: refresh the road-snapped preview between the
        // last confirmed point and the cursor.
        void OnMapMouseMove( std::shared_ptr<gem::MapView> mapView, const gem::Xy& point )
        {
            if( !m_defining )
                return;

            // The traffic service matches the free cursor position onto the road network
            // and returns: the path from the previous match to it, and the match info to
            // be used as the start of the next preview segment.
            auto preview = gem::TrafficService().getPersistentRoadblockPathPreview( m_lastMatch, mapView->transformScreenToWgs( point ), TransportMode() );
            if( std::get<2>( preview ) != gem::KNoError )
                return; // nothing matched under the cursor - keep the previous preview

            // Keep the already confirmed part of the sketch, [0, m_lastMatchIdx), and
            // replace the tail with the fresh preview.
            m_previewSketch.delRange( m_lastMatchIdx, -1 );
            m_previewSketch.add( std::get<0>( preview ) );
            m_currentMatch = std::get<1>( preview );
        }

        // Commit the confirmed points as one persistent user roadblock.
        void Finish( std::shared_ptr<gem::MapView> mapView )
        {
            const int points = GetPointsCount();
            const bool hasPath = points > 1;

            gem::TrafficEvent roadblock;
            int error = gem::KNoError;
            std::string id;

            if( hasPath )
            {
                auto lifetime = GetRoadblockLifetime();

                // The id has to be unique: roadblocks are persistent, so an id used by a
                // previous run of the example is still in use ( addPersistentRoadblock
                // would return error::KInUse ) - hence it is derived from the roadblock
                // start time.
                id = "example_roadblock_" + std::to_string( lifetime.first.asInt() ) + "_" + std::to_string( ++m_idCounter );

                // A single coordinate defines a point roadblock, which may block the
                // matched road both ways; two or more coordinates define a path
                // roadblock, blocking the first -> last direction only.
                auto result = gem::TrafficService().addPersistentRoadblock( m_coords, lifetime.first, lifetime.second, TransportMode(), gem::String( id ) );
                roadblock = result.first;
                error = result.second;
            }

            Reset( mapView );

            if( !hasPath )
            {
                m_status = "At least 2 points are needed for a roadblock path - nothing added";
                return;
            }
            if( error != gem::KNoError )
            {
                // error::KNoRoute means the points cannot be matched to a road path,
                // error::KExist / error::KInUse mean the roadblock or its id already exists.
                m_status = "addPersistentRoadblock failed (err=" + std::to_string( error ) + ")";
                GEM_LOGE( "addPersistentRoadblock failed (err=%d)", error );
                return;
            }

            m_roadblocks.push_back( { id, points } );
            m_status = "Roadblock " + id + " added (" + std::to_string( points ) + " points)";

            mapView->centerOnArea( roadblock.getBoundingBox(), -1, gem::Xy(), gem::Animation( gem::AnimationLinear, gem::ProgressListener(), 1000 ) );
        }

        // Drop the definition without committing anything.
        void Cancel( std::shared_ptr<gem::MapView> mapView )
        {
            Reset( mapView );
            m_status = "Roadblock definition cancelled";
        }

        void Remove( int index )
        {
            if( index < 0 || index >= int( m_roadblocks.size() ) )
                return;

            gem::TrafficService().removePersistentRoadblock( gem::String( m_roadblocks[index].id ) );
            m_status = "Roadblock " + m_roadblocks[index].id + " removed";
            m_roadblocks.erase( m_roadblocks.begin() + index );
        }

        void RemoveAll()
        {
            gem::TrafficService().removeAllPersistentRoadblocks();
            m_roadblocks.clear();
            m_status = "All user roadblocks removed";
        }

    private:
        int TransportMode() const
        {
            return int( kTransportModes[m_transportModeIdx] );
        }

        // First point of the roadblock path: create the polyline sketch which carries
        // the geometry while the roadblock is being defined.
        void Start( std::shared_ptr<gem::MapView> mapView, const gem::Xy& point )
        {
            const gem::Coordinates anchor = mapView->transformScreenToWgs( point );

            m_previewSketch.delPart( 0 ); // drop any leftover geometry
            m_previewSketch.add( anchor, -1, 0 );

            // Sketches are markers with individual render settings, drawn on top of all
            // other marker collections - ideal for transient UI geometry like this.
            mapView->preferences()
                .markers()
                .sketches( gem::MT_Polyline )
                .add( m_previewSketch, gem::MarkerRenderSettings().setPolylineInnerColor( gem::Rgba( 0, 0, 200, 255 ) ).setPolylineInnerSize( 1.5 ) );

            // A double click adds a roadblock point, it must not zoom the map in. The
            // point-adding press itself is not forwarded to the map ( see
            // MyTouchEventListener ), but the gesture detector must not pick up the
            // preceding single click either, so the zoom gesture is disabled for the
            // duration of the definition. Everything else - pan, pinch, rotate - keeps
            // working.
            mapView->preferences().enableTouchGestures( gem::TG_OnDoubleTouch, false );

            m_coords.clear();
            m_coords.push_back( anchor );

            m_lastMatch = gem::UserRoadblockPathPreviewCoordinate();
            m_lastMatch.coord = anchor;
            m_currentMatch = gem::UserRoadblockPathPreviewCoordinate();
            m_lastMatchIdx = 0;

            m_armed = false;
            m_defining = true;
            m_status = "Move the mouse to preview, double click to add a point";
        }

        // Confirm the currently previewed point.
        void ConfirmPoint( std::shared_ptr<gem::MapView> mapView, const gem::Xy& point )
        {
            m_coords.push_back( mapView->transformScreenToWgs( point ) );

            if( m_currentMatch.coord )
            {
                // Use the road match from the preview instead of the raw screen position:
                // a coordinate which is not on a road cannot define a roadblock path.
                m_coords.back_nc() = m_currentMatch.coord;
                m_lastMatch = m_currentMatch;
                m_currentMatch = gem::UserRoadblockPathPreviewCoordinate();

                // The previewed segment is now part of the confirmed sketch, the next
                // preview replaces everything after it.
                m_lastMatchIdx = m_previewSketch.getCoordinatesCount( 0 );
            }

            m_status = std::to_string( GetPointsCount() ) + " point(s) - double click to add more, or finish the roadblock";
        }

        // Reset the definition state and remove the preview sketch from the map.
        void Reset( std::shared_ptr<gem::MapView> mapView )
        {
            const int index = mapView->preferences().markers().sketches( gem::MT_Polyline ).indexOf( m_previewSketch );
            if( index >= 0 )
                mapView->preferences().markers().sketches( gem::MT_Polyline ).del( index );

            mapView->preferences().enableTouchGestures( gem::TG_OnDoubleTouch, true );

            m_coords.clear();
            m_lastMatch = gem::UserRoadblockPathPreviewCoordinate();
            m_currentMatch = gem::UserRoadblockPathPreviewCoordinate();
            m_lastMatchIdx = -1;
            m_armed = false;
            m_defining = false;
        }

        bool m_armed = false;    // waiting for the first double click
        bool m_defining = false; // the roadblock path is being defined

        gem::CoordinatesList m_coords; // confirmed roadblock path points
        gem::Marker m_previewSketch;   // polyline sketch: confirmed part + live preview
        int m_lastMatchIdx = -1;       // sketch coordinate index where the preview starts

        gem::UserRoadblockPathPreviewCoordinate m_lastMatch;    // last confirmed road match
        gem::UserRoadblockPathPreviewCoordinate m_currentMatch; // road match under the cursor

        int m_transportModeIdx = 0;
        int m_idCounter = 0;
        std::string m_status = "Idle";
        std::vector<TRoadblockInfo> m_roadblocks;
    };

    // Demo route, recalculated on demand to show the roadblock being avoided.
    class CRouteDemo
    {
    public:
        void Calculate( std::shared_ptr<gem::MapView> mapView, int transportModeIdx )
        {
            gem::LandmarkList waypoints( { { "departure", { kDepartureLat, kDepartureLon } }, { "destination", { kDestinationLat, kDestinationLon } } } );

            gem::RouteList routes;
            ProgressListener listener;

            // ETrafficAvoidance::TA_Roadblocks makes the routing engine avoid roadblock
            // traffic events, which includes the user roadblocks defined above.
            gem::RoutingService().calculateRoute( routes, waypoints,
                                                  gem::RoutePreferences()
                                                      .setTransportMode( kTransportModes[transportModeIdx] )
                                                      .setRouteType( gem::RT_Fastest )
                                                      .setAlternativesSchema( gem::AS_Never )
                                                      .setAvoidTraffic( gem::TA_Roadblocks ),
                                                  &listener );

            // A failure leaves the previously calculated route on the map and keeps the
            // before / after summaries intact - only m_error is updated.
            if( !WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 30000 ) )
            {
                m_error = "route calculation did not finish (timed out or window closed)";
                GEM_LOGE( "Route calculation did not finish" );
                return;
            }
            if( listener.GetError() != gem::KNoError || routes.empty() )
            {
                m_error = "route calculation failed (err=" + std::to_string( listener.GetError() ) + ")";
                GEM_LOGE( "Route calculation failed (err=%d)", listener.GetError() );
                return;
            }

            m_error.clear();
            m_previousSummary = m_summary;
            m_routes = routes;

            mapView->preferences().routes().clear();
            mapView->preferences().routes().add( m_routes[0], true );
            mapView->centerOnRoute( m_routes[0], gem::Rect(), gem::Animation( gem::AnimationLinear, gem::ProgressListener(), 1000 ) );

            auto timeDistance = m_routes[0].getTimeDistance( false );
            m_summary = std::to_string( timeDistance.getTotalDistance() / 1000 ) + "." + std::to_string( ( timeDistance.getTotalDistance() % 1000 ) / 100 ) + " km, " +
                        std::to_string( timeDistance.getTotalTime() / 60 ) + " min";
        }

        bool HasRoute() const
        {
            return !m_routes.empty();
        }
        const std::string& GetSummary() const
        {
            return m_summary;
        }
        const std::string& GetPreviousSummary() const
        {
            return m_previousSummary;
        }
        const std::string& GetError() const
        {
            return m_error;
        }

    private:
        gem::RouteList m_routes;
        std::string m_summary;
        std::string m_previousSummary;
        std::string m_error;
    };

    // Derive from the standard touch event handler class which makes the map view
    // interactive, and add double-click detection on top of it.
    class MyTouchEventListener : public CTouchEventListener
    {
    public:
        explicit MyTouchEventListener( CUserRoadblockDefinition& definition )
            : m_definition( definition )
        {
        }

        void handleTouchEvent( int eventType, int pointerId, int x, int y ) override
        {
            auto mapView = getMapViewPointer();
            setCursorPosition( x, y );

            if( mapView.get() == nullptr )
            {
                GEM_LOGE( "null mapView!" );
                return;
            }

            const gem::Xy mousePos( x, y );

            if( eventType == gem::ETouchEvent::TE_Down )
            {
                // A release which never reached us ( released over the UI panel, pointer
                // left the window ) must not make us swallow a later, unrelated release.
                m_swallowUp = false;

                const long long now = gem::Time::getUniversalTime().asInt();
                const bool isDoubleClick = m_lastDownTime > 0 && now - m_lastDownTime <= KDoubleClickIntervalMs && std::abs( x - m_lastDownX ) <= KClickMoveTolerancePx &&
                                           std::abs( y - m_lastDownY ) <= KClickMoveTolerancePx;

                m_lastDownTime = now;
                m_lastDownX = x;
                m_lastDownY = y;

                if( isDoubleClick && m_definition.IsActive() )
                {
                    // Swallow this press and the matching release: the map must not zoom
                    // in on the double click which adds a roadblock point.
                    m_lastDownTime = 0;
                    m_swallowUp = true;
                    m_definition.OnMapDoubleClick( mapView, mousePos );
                    return;
                }
            }
            else if( eventType == gem::ETouchEvent::TE_Up && m_swallowUp )
            {
                m_swallowUp = false;
                return;
            }

            // Everything else is forwarded, so the map stays pannable and zoomable while
            // the roadblock is being defined.
            mapView->getScreen()->handleTouchEvent( ( gem::ETouchEvent ) eventType, pointerId, mousePos );

            // The preview follows the cursor, pressed or not.
            if( eventType == gem::ETouchEvent::TE_Move )
                m_definition.OnMapMouseMove( mapView, mousePos );
        }

    private:
        static constexpr long long KDoubleClickIntervalMs = 400;
        static constexpr int KClickMoveTolerancePx = 6;

        CUserRoadblockDefinition& m_definition;
        long long m_lastDownTime = 0;
        int m_lastDownX = 0;
        int m_lastDownY = 0;
        bool m_swallowUp = false;
    };

    auto getUiRender( CUserRoadblockDefinition& definition, CRouteDemo& route )
    {
        return std::bind(
            [&definition, &route]( gem::StrongPointer<gem::MapView> mapView )
            {
                const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos( ImVec2( mainViewport->WorkPos.x + 0, mainViewport->WorkPos.y + 20 ), ImGuiCond_FirstUseEver );
                ImGui::Begin( "panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings );

                ImGui::TextUnformatted( "User roadblock" );
                ImGui::Separator();

                // Transport mode - part of both the roadblock and the route definition
                int transportModeIdx = definition.GetTransportModeIndex();
                ImGui::BeginDisabled( definition.IsDefining() );
                if( ImGui::Combo( "transport", &transportModeIdx, kTransportModeNames, kTransportModesCount ) )
                    definition.SetTransportModeIndex( transportModeIdx );
                ImGui::EndDisabled();

                ImGui::Spacing();

                if( !definition.IsActive() )
                {
                    if( ImGui::Button( "Define roadblock from points" ) )
                        definition.Arm();
                }
                else
                {
                    ImGui::TextUnformatted( "Double click on the map to add points" );
                    ImGui::Text( "points: %d", definition.GetPointsCount() );

                    ImGui::BeginDisabled( definition.GetPointsCount() < 2 );
                    if( ImGui::Button( "Finish roadblock" ) )
                    {
                        definition.Finish( mapView );
                        if( route.HasRoute() )
                            route.Calculate( mapView, definition.GetTransportModeIndex() );
                    }
                    ImGui::EndDisabled();

                    ImGui::SameLine();
                    if( ImGui::Button( "Cancel" ) )
                        definition.Cancel( mapView );
                }

                ImGui::Spacing();
                ImGui::TextUnformatted( definition.GetStatus().c_str() );

                const auto& roadblocks = definition.GetRoadblocks();
                if( !roadblocks.empty() )
                {
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text( "active user roadblocks: %d", ( int ) roadblocks.size() );

                    int removeIndex = -1;
                    for( int i = 0; i < ( int ) roadblocks.size(); ++i )
                    {
                        ImGui::Text( "%s (%d pts)", roadblocks[i].id.c_str(), roadblocks[i].points );
                        ImGui::SameLine();
                        ImGui::PushID( i );
                        if( ImGui::SmallButton( "remove" ) )
                            removeIndex = i;
                        ImGui::PopID();
                    }

                    if( ImGui::Button( "Remove all roadblocks" ) )
                    {
                        definition.RemoveAll();
                        if( route.HasRoute() )
                            route.Calculate( mapView, definition.GetTransportModeIndex() );
                    }
                    else if( removeIndex >= 0 )
                    {
                        definition.Remove( removeIndex );
                        if( route.HasRoute() )
                            route.Calculate( mapView, definition.GetTransportModeIndex() );
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextUnformatted( "Demo route" );
                if( ImGui::Button( route.HasRoute() ? "Recalculate route" : "Calculate route" ) )
                    route.Calculate( mapView, definition.GetTransportModeIndex() );
                if( !route.GetSummary().empty() )
                    ImGui::Text( "now:      %s", route.GetSummary().c_str() );
                if( !route.GetPreviousSummary().empty() )
                    ImGui::Text( "previous: %s", route.GetPreviousSummary().c_str() );
                if( !route.GetError().empty() )
                    ImGui::Text( "error:    %s", route.GetError().c_str() );

                ImGui::End();
            },
            std::placeholders::_1 );
    }
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

    // Declared after the session: locals are destroyed in reverse order, so these
    // release their SDK objects before SdkSession uninitializes the SDK.
    CUserRoadblockDefinition roadblockDefinition;
    CRouteDemo routeDemo;

    // Create an interactive map view with double-click handling for roadblock points
    MyTouchEventListener touchEventListener( roadblockDefinition );

    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "UserRoadblock", &touchEventListener, getUiRender( roadblockDefinition, routeDemo ) ) );
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
        return GEM_GET_API_ERROR();
    }

    // User roadblocks are rendered by the traffic layer of the map style
    if( mapView->preferences().setTrafficVisibility( true ) != gem::KNoError )
        GEM_LOGW( "Current map style has no traffic layer - roadblocks will not be rendered" );

    // Show the demo route so there is something to block
    routeDemo.Calculate( mapView, roadblockDefinition.GetTransportModeIndex() );

    WAIT_UNTIL_WINDOW_CLOSE();

    // Leave no user roadblocks behind - they are persistent across SDK sessions
    gem::TrafficService().removeAllPersistentRoadblocks();

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
