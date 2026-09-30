// SPDX-FileCopyrightText: 2024-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"
#include "BitmapImpl.h"

#include <API/GEM_MapView.h>
#include <API/GEM_Weather.h>
#include <API/GEM_Debug.h>

#include <ctime>
#include <sstream>
#include <imgui.h>

#define PIXELS_WEATHER_ICON_SIZE 48

// Derive from the standard touch event handler class which makes the map view interactive

// Sunrise and sunset are UTC timestamps (seconds since 1970): show them as a time of day.
static std::string FormatParameterValue( const gem::weather::Parameter& parameter )
{
    const std::string type = parameter.type.toStdString();
    if( type == "Sunrise" || type == "Sunset" )
    {
        const std::time_t time = static_cast<std::time_t>( parameter.value );
        std::tm utc {};
#if defined( _WIN32 )
        gmtime_s( &utc, &time );
#else
        gmtime_r( &time, &utc );
#endif
        char text[16];
        std::strftime( text, sizeof( text ), "%H:%M UTC", &utc );
        return text;
    }
    std::ostringstream stream;
    stream << parameter.value;
    return stream.str();
}

class MyTouchEventListener : public CTouchEventListener
{
private:
    // Screen rectangles (left, top, right, bottom) of the ImGui windows drawn in the last frame, see handleTouchEvent()
    std::vector<ImVec4> m_uiRects;
    bool m_pressOnUi = false;

    std::vector<std::string> strvecCurrent;
    std::vector<std::string> strvecHourly;
    std::vector<std::string> strvecDaily;
    std::vector<gem::Image> imgvecCurrent;
    std::vector<gem::Image> imgvecHourly;
    std::vector<gem::Image> imgvecDaily;
    bool renderPanel = false;
    bool renderMenu = false;

    // Click-vs-pan detection: a press that moves more than this many pixels is a map pan.
    static constexpr int KClickMoveTolerancePx = 5;
    int m_downX = 0;
    int m_downY = 0;
    bool m_dragging = false;

public:
    enum class WeatherType
    {
        Current = 0,
        HourlyForecast,
        DailyForecast,
        None
    };
    std::vector<std::string> getStrVecOutput( MyTouchEventListener::WeatherType weatherType = MyTouchEventListener::WeatherType::Current )
    {
        return weatherType == MyTouchEventListener::WeatherType::Current          ? strvecCurrent
               : weatherType == MyTouchEventListener::WeatherType::HourlyForecast ? strvecHourly
                                                                                  : strvecDaily;
    }
    std::vector<gem::Image> getImageVecOutput( MyTouchEventListener::WeatherType weatherType = MyTouchEventListener::WeatherType::Current )
    {
        return weatherType == MyTouchEventListener::WeatherType::Current          ? imgvecCurrent
               : weatherType == MyTouchEventListener::WeatherType::HourlyForecast ? imgvecHourly
                                                                                  : imgvecDaily;
    }
    void setRenderPanel( bool isrender )
    {
        renderPanel = isrender;
    }
    bool getRenderPanel()
    {
        return renderPanel;
    }
    void setRenderMenu( bool isrender )
    {
        renderMenu = isrender;
    }
    bool getRenderMenu()
    {
        return renderMenu;
    }

    void clearUiRects()
    {
        m_uiRects.clear();
    }

    // Call between ImGui::Begin() and ImGui::End(): remembers the rectangle of the current window.
    void addUiRect()
    {
        const ImVec2 pos = ImGui::GetWindowPos();
        const ImVec2 size = ImGui::GetWindowSize();
        m_uiRects.push_back( ImVec4( pos.x, pos.y, pos.x + size.x, pos.y + size.y ) );
    }

    bool isOverUi( int x, int y ) const
    {
        for( const auto& r : m_uiRects )
            if( x >= r.x && x < r.z && y >= r.y && y < r.w )
                return true;
        return false;
    }

    // This function from the standard touch event handler for the map view is
    // overridden to add our own processing - enabling drawing a route by dragging
    // after a single or double click on the map.

    void handleTouchEvent( int eventType, int pointerId, int x, int y ) override
    {
        auto mapView = getMapViewPointer();
        setCursorPosition( x, y );
        gem::Xy mousePos( x, y );
        if( mapView.get() == nullptr )
        {
            GEM_LOGE( "null mapView!" );
            return;
        }
        // A press on one of the ImGui windows belongs to the UI, not to the map. ImGui's WantCaptureMouse (which filters the
        // events) is updated one frame late: a tap without prior hover (touch screen) would also reach the map and request
        // the weather at the button's position.
        if( eventType == gem::ETouchEvent::TE_Down )
            m_pressOnUi = isOverUi( x, y );
        if( m_pressOnUi )
        {
            if( eventType == gem::ETouchEvent::TE_Up )
                m_pressOnUi = false;
            return;
        }

        mapView->getScreen()->handleTouchEvent( ( gem::ETouchEvent ) eventType, pointerId, mousePos );

        // Distinguish a clean click from a map pan: the weather is fetched only on a
        // press & release without significant movement - dragging just pans the map
        // and does not update the selected location.
        if( eventType == gem::ETouchEvent::TE_Down )
        {
            m_downX = x;
            m_downY = y;
            m_dragging = false;
            return;
        }
        if( eventType == gem::ETouchEvent::TE_Move )
        {
            if( std::abs( x - m_downX ) > KClickMoveTolerancePx || std::abs( y - m_downY ) > KClickMoveTolerancePx )
                m_dragging = true;
            return;
        }
        if( eventType != gem::ETouchEvent::TE_Up || m_dragging )
            return;

        auto coord = mapView->transformScreenToWgs( mousePos );
        GEM_LOGE( "xy( %d, %d ) mapcoord lon,lat( %f, %f )\n", x, y, coord.getLongitude(), coord.getLatitude() );

        {
            gem::CoordinatesList coordinatesList;
            coordinatesList.push_back( coord );
            setRenderPanel( true );
            setRenderMenu( true );

            std::ostringstream strstream;
            strvecCurrent.clear();
            strvecHourly.clear();
            strvecDaily.clear();
            imgvecCurrent.clear();
            imgvecHourly.clear();
            imgvecDaily.clear();

            ProgressListener weatherListenerCurrent;
            ProgressListener weatherListenerHourly;
            ProgressListener weatherListenerDaily;

            gem::weather::LocationForecastList locationForecastListResultCurrentWeatherConditions;
            gem::weather::LocationForecastList locationForecastListResultHourlyWeatherConditions;
            gem::weather::LocationForecastList locationForecastListResultDailyWeatherConditions;

            strstream.str( "" );
            strstream << "Lon, Lat " << coord.getLongitude() << ", " << coord.getLatitude();
            strvecCurrent.push_back( strstream.str() );
            strvecHourly.push_back( strstream.str() );
            strvecDaily.push_back( strstream.str() );

            int errorn = gem::weather::Service().getCurrent( coordinatesList, locationForecastListResultCurrentWeatherConditions, &weatherListenerCurrent );
            if( errorn != gem::KNoError )
            {
                GEM_LOGE( "error( %d ) current weather\n", errorn );
            }
            else if( !WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &weatherListenerCurrent ), 15000 ) || weatherListenerCurrent.GetError() != gem::KNoError ||
                     locationForecastListResultCurrentWeatherConditions.empty() || locationForecastListResultCurrentWeatherConditions.front().forecast.empty() )
            {
                // Timed out, failed, or returned no forecast - leave the vectors empty
                // (the render callback tolerates empty vectors, see the icon guards).
                GEM_LOGE( "current weather request failed (err=%d)\n", weatherListenerCurrent.GetError() );
            }
            else
            {
                int index = 0;
                GEM_LOGE( "CURRENT weather vec size main %d forecast front %d", ( int ) locationForecastListResultCurrentWeatherConditions.size(),
                          ( int ) locationForecastListResultCurrentWeatherConditions.front().forecast.size() );
                strvecCurrent.push_back( "Current Weather Conditions" );
                for( auto& e : locationForecastListResultCurrentWeatherConditions.front().forecast.front().params )
                {
                    GEM_LOGE( "Got CURRENT Weather [ %d ] ( %s, %s )( %f )\n", index, e.name.toStdString().c_str(),
                              std::string( "°" ) == e.unit.toStdString()    ? "degrees"
                              : std::string( "°C" ) == e.unit.toStdString() ? "deg C"
                                                                            : e.unit.toStdString().c_str(),
                              e.value );
                    strstream.str( "" );
                    strstream << "   " << e.name.toStdString() << " "
                              << ( std::string( "°" ) == e.unit.toStdString() ? "degrees"
                                   : std::string( "°C" ) == e.unit.toStdString()
                                       ? "deg C"
                                       : e.unit.toStdString().c_str() )
                              << " " << FormatParameterValue( e );
                    strvecCurrent.push_back( strstream.str() );
                    index++;
                }
                imgvecCurrent.push_back( locationForecastListResultCurrentWeatherConditions.front().forecast.front().image );
            }
            int hours = 36;
            errorn = gem::weather::Service().getHourlyForecast( hours, coordinatesList, locationForecastListResultHourlyWeatherConditions, &weatherListenerHourly );
            if( errorn != gem::KNoError )
            {
                GEM_LOGE( "error( %d ) hourly weather\n", errorn );
            }
            else if( !WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &weatherListenerHourly ), 15000 ) || weatherListenerHourly.GetError() != gem::KNoError ||
                     locationForecastListResultHourlyWeatherConditions.empty() || locationForecastListResultHourlyWeatherConditions.front().forecast.empty() )
            {
                GEM_LOGE( "hourly weather request failed (err=%d)\n", weatherListenerHourly.GetError() );
            }
            else
            {
                int index = 0;
                GEM_LOGE( "HOURLY weather vec size main %d forecast front %d", ( int ) locationForecastListResultHourlyWeatherConditions.size(),
                          ( int ) locationForecastListResultHourlyWeatherConditions.front().forecast.size() );
                strvecHourly.push_back( "Hourly Weather Forecast" );
                for( auto& e : locationForecastListResultHourlyWeatherConditions.front().forecast.front().params )
                {
                    GEM_LOGE( "Got HOURLY Weather [ %d ] ( %s, %s )( %.1f )\n", index, e.name.toStdString().c_str(),
                              std::string( "°" ) == e.unit.toStdString()    ? "degrees"
                              : std::string( "°C" ) == e.unit.toStdString() ? "deg C"
                                                                            : e.unit.toStdString().c_str(),
                              e.value );
                    strstream.str( "" );
                    strstream << "   " << e.name.toStdString() << " "
                              << ( std::string( "°" ) == e.unit.toStdString()    ? "degrees"
                                   : std::string( "°C" ) == e.unit.toStdString() ? "deg C"
                                                                                 : e.unit.toStdString().c_str() )
                              << " " << FormatParameterValue( e );
                    strvecHourly.push_back( strstream.str() );
                    index++;
                }
                imgvecHourly.push_back( locationForecastListResultHourlyWeatherConditions.front().forecast.front().image );
            }
            errorn = gem::weather::Service().getDailyForecast( 10, coordinatesList, locationForecastListResultDailyWeatherConditions, &weatherListenerDaily );
            if( errorn != gem::KNoError )
            {
                GEM_LOGE( "error( %d ) daily weather\n", errorn );
            }
            else if( !WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &weatherListenerDaily ), 15000 ) || weatherListenerDaily.GetError() != gem::KNoError ||
                     locationForecastListResultDailyWeatherConditions.empty() || locationForecastListResultDailyWeatherConditions.front().forecast.empty() )
            {
                GEM_LOGE( "daily weather request failed (err=%d)\n", weatherListenerDaily.GetError() );
            }
            else
            {
                int index = 0;
                strvecDaily.push_back( "Daily Weather Forecast" );
                for( auto& e : locationForecastListResultDailyWeatherConditions.front().forecast.front().params )
                {
                    GEM_LOGE( "Got DAILY Weather [ %d ] ( %s, %s )( %.1f )\n", index, e.name.toStdString().c_str(),
                              std::string( "°" ) == e.unit.toStdString()    ? "degrees"
                              : std::string( "°C" ) == e.unit.toStdString() ? "deg C"
                                                                            : e.unit.toStdString().c_str(),
                              e.value );
                    strstream.str( "" );
                    strstream << "   " << e.name.toStdString() << " "
                              << ( std::string( "°" ) == e.unit.toStdString()    ? "degrees"
                                   : std::string( "°C" ) == e.unit.toStdString() ? "deg C"
                                                                                 : e.unit.toStdString().c_str() )
                              << " " << FormatParameterValue( e );
                    strvecDaily.push_back( strstream.str() );
                    index++;
                }
                imgvecDaily.push_back( locationForecastListResultDailyWeatherConditions.front().forecast.front().image );
            }
        }
    }
};

// The touch listener is passed in from main(), where it must be declared AFTER the
// SdkSession: locals are destroyed in reverse order, so the listener releases its
// SDK objects (map view, weather icon images) before the session shuts the SDK down.
// A listener with static / global storage would outlive the SDK shutdown and its
// objects would be reported as memory leaks.
namespace
{
    auto getUiRender( MyTouchEventListener& touchEventListener )
    {
        return std::bind(
            [&touchEventListener]( gem::StrongPointer<gem::MapView> mapView )
            {
                touchEventListener.clearUiRects();

                // Selected weather panel (chosen via the menu buttons).
                static auto weatherType = MyTouchEventListener::WeatherType::None;

                // Bumped when the panel is closed so the next panel gets a fresh ImGui window ID.
                // ImGui keeps a hidden window's last rect, and an AlwaysAutoResize window's first
                // re-appearing frame is sized from its stale (large) content - a fresh ID starts
                // from scratch and fits the actual content immediately, avoiding a one-frame flash.
                static int panelGeneration = 0;

                const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos( ImVec2( main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 12 ), ImGuiCond_FirstUseEver );
                ImGui::SetNextWindowSize( ImVec2( 0, 0 ) );

                // Show an instructions panel whenever no other UI is visible - at startup
                // (before the first click) and after the UI is hidden via the Close button -
                // so the app never shows a bare map with no visible UI.
                if( !touchEventListener.getRenderMenu() && !touchEventListener.getRenderPanel() )
                {
                    ImGui::Begin( "panel_hint", nullptr,
                                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings );
                    ImGui::TextUnformatted( "Click on the map to get the weather at that position." );
                    touchEventListener.addUiRect();
                    ImGui::End();
                }

                if( touchEventListener.getRenderMenu() )
                {
                    ImGui::SetNextWindowBgAlpha( 1.0f );
                    ImGui::SetNextWindowPos( ImVec2( main_viewport->WorkPos.x + main_viewport->Size.x * 0.7f, main_viewport->WorkPos.y + 0 ), ImGuiCond_FirstUseEver );
                    ImGui::SetNextWindowSize( ImVec2( 0, 0 ) ); // main_viewport->WorkSize.x, 108));
                    ImGui::GetStyle().WindowRounding = 0.0f;
                    ImGui::Begin( "panel2", nullptr,
                                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |
                                      ImGuiWindowFlags_NoSavedSettings );
                    if( ImGui::Button( "Current Weather" ) )
                    {
                        weatherType = MyTouchEventListener::WeatherType::Current;
                        touchEventListener.setRenderPanel( true );
                    }
                    if( ImGui::Button( "Hourly Weather Forecast" ) )
                    {
                        weatherType = MyTouchEventListener::WeatherType::HourlyForecast;
                        touchEventListener.setRenderPanel( true );
                    }
                    if( ImGui::Button( "Daily Weather Forecast" ) )
                    {
                        weatherType = MyTouchEventListener::WeatherType::DailyForecast;
                        touchEventListener.setRenderPanel( true );
                    }
                    if( ImGui::Button( "Close" ) )
                    {
                        weatherType = MyTouchEventListener::WeatherType::None;
                        touchEventListener.setRenderPanel( false );
                        touchEventListener.setRenderMenu( false );
                        panelGeneration++; // next panel gets a fresh window - see panelGeneration
                    }
                    touchEventListener.addUiRect();
                    ImGui::End();
                }
                if( touchEventListener.getRenderPanel() )
                {
                    // Same visible title, generation-suffixed ID (the part after ## is ID-only).
                    char panelName[32];
                    std::snprintf( panelName, sizeof( panelName ), "panel##g%d", panelGeneration );

                    int index = 0;
                    switch( weatherType )
                    {
                        case MyTouchEventListener::WeatherType::Current:
                        {
                            auto strvec = touchEventListener.getStrVecOutput();
                            ImGui::Begin( panelName, nullptr,
                                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
                                              ImGuiWindowFlags_NoSavedSettings );
                            if( ImGui::BeginTable( "weather_panel", 2 ) )
                            {
                                ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed, PIXELS_WEATHER_ICON_SIZE + 12 );
                                ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthStretch );

                                index = 0;
                                for( auto& s : strvec )
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex( 0 );

                                    //////////////////////////////////////////////////////////
                                    // WEATHER ICON
                                    //////////////////////////////////////////////////////////
                                    // The icon vector may still be empty: the touch handler waits for the weather
                                    // requests with WAIT_UNTIL, which pumps render frames, so this callback can run
                                    // while the vectors are only partially populated (or after a failed request).
                                    auto imgvec = touchEventListener.getImageVecOutput();
                                    if( index == 1 && !imgvec.empty() ) // for current weather, show icon only for the title element at index 1
                                    {
                                        const ImVec2 weatherIconSize { PIXELS_WEATHER_ICON_SIZE, PIXELS_WEATHER_ICON_SIZE };
                                        gem::Rgba color( 255, 0, 255, 255 );
                                        gem::AbstractGeometryImageRenderSettings settings( gem::Rgba::white(), gem::Rgba::black(), color );
                                        auto bitmap = gem::StrongPointerFactory<BitmapImpl>( 70, 70 );
                                        imgvec.front().render( *bitmap );
                                        unsigned int textureId = BitmapImpl::LoadTextureIntoGPU( bitmap->size().width, bitmap->size().height, bitmap->begin() );
                                        ImGui::Image( textureId, weatherIconSize );
                                    }
                                    index++;

                                    ImGui::TableNextColumn();
                                    ImGui::Text( "%s", s.c_str() );
                                }
                                ImGui::EndTable();
                            }
                            touchEventListener.addUiRect();
                            ImGui::End();
                            break;
                        }
                        case MyTouchEventListener::WeatherType::HourlyForecast:
                        {
                            auto strvec = touchEventListener.getStrVecOutput( MyTouchEventListener::WeatherType::HourlyForecast );
                            ImGui::Begin( panelName, nullptr,
                                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
                                              ImGuiWindowFlags_NoSavedSettings );
                            if( ImGui::BeginTable( "weather_panel", 2 ) )
                            {
                                ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed, PIXELS_WEATHER_ICON_SIZE + 12 );
                                ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthStretch );

                                index = 0;
                                for( auto& s : strvec )
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex( 0 );

                                    //////////////////////////////////////////////////////////
                                    // WEATHER ICON
                                    //////////////////////////////////////////////////////////
                                    // Guard against an empty icon vector - see the comment in the Current case.
                                    auto imgvec = touchEventListener.getImageVecOutput( MyTouchEventListener::WeatherType::HourlyForecast );
                                    if( index == 1 && !imgvec.empty() )
                                    {
                                        const ImVec2 weatherIconSize { PIXELS_WEATHER_ICON_SIZE, PIXELS_WEATHER_ICON_SIZE };
                                        gem::Rgba color( 255, 0, 255, 255 );
                                        gem::AbstractGeometryImageRenderSettings settings( gem::Rgba::white(), gem::Rgba::black(), color );
                                        auto bitmap = gem::StrongPointerFactory<BitmapImpl>( 70, 70 );
                                        imgvec.front().render( *bitmap );
                                        unsigned int textureId = BitmapImpl::LoadTextureIntoGPU( bitmap->size().width, bitmap->size().height, bitmap->begin() );
                                        ImGui::Image( textureId, weatherIconSize );
                                    }
                                    index++;

                                    ImGui::TableNextColumn();
                                    ImGui::Text( "%s", s.c_str() );
                                }
                                ImGui::EndTable();
                            }
                            touchEventListener.addUiRect();
                            ImGui::End();
                            break;
                        }
                        case MyTouchEventListener::WeatherType::DailyForecast:
                        {
                            auto strvec = touchEventListener.getStrVecOutput( MyTouchEventListener::WeatherType::DailyForecast );
                            ImGui::Begin( panelName, nullptr,
                                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
                                              ImGuiWindowFlags_NoSavedSettings );
                            if( ImGui::BeginTable( "weather_panel", 2 ) )
                            {
                                ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed, PIXELS_WEATHER_ICON_SIZE + 12 );
                                ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthStretch );

                                index = 0;
                                for( auto& s : strvec )
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex( 0 );

                                    //////////////////////////////////////////////////////////
                                    // WEATHER ICON
                                    //////////////////////////////////////////////////////////
                                    // Guard against an empty icon vector - see the comment in the Current case.
                                    auto imgvec = touchEventListener.getImageVecOutput( MyTouchEventListener::WeatherType::DailyForecast );
                                    if( index == 0 && !imgvec.empty() )
                                    {
                                        const ImVec2 weatherIconSize { PIXELS_WEATHER_ICON_SIZE, PIXELS_WEATHER_ICON_SIZE };
                                        gem::Rgba color( 255, 0, 255, 255 );
                                        gem::AbstractGeometryImageRenderSettings settings( gem::Rgba::white(), gem::Rgba::black(), color );
                                        auto bitmap = gem::StrongPointerFactory<BitmapImpl>( 70, 70 );
                                        imgvec.front().render( *bitmap );
                                        unsigned int textureId = BitmapImpl::LoadTextureIntoGPU( bitmap->size().width, bitmap->size().height, bitmap->begin() );
                                        ImGui::Image( textureId, weatherIconSize );
                                    }
                                    index++;

                                    ImGui::TableNextColumn();
                                    ImGui::Text( "%s", s.c_str() );
                                }
                                ImGui::EndTable();
                            }
                            touchEventListener.addUiRect();
                            ImGui::End();
                            break;
                        }
                        case MyTouchEventListener::WeatherType::None:
                        default:
                        {
                            auto strvec = touchEventListener.getStrVecOutput();
                            if( !strvec.empty() )
                            {
                                ImGui::Begin( panelName, nullptr,
                                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse |
                                                  ImGuiWindowFlags_NoSavedSettings );
                                for( auto& s : strvec )
                                {
                                    ImGui::Text( "%s", s.c_str() );
                                    break;
                                }
                                touchEventListener.addUiRect();
                                ImGui::End();
                            }
                            break;
                        }
                    } //switch
                }
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

    // Create an interactive map view.
    // The listener is deliberately a local declared AFTER the session: locals are
    // destroyed in reverse order, so the listener releases its SDK objects (map view,
    // weather icon images) before SdkSession uninitializes the SDK. A static / global
    // listener would outlive the SDK shutdown and its objects would be reported as
    // memory leaks.
    MyTouchEventListener pTouchEventListener;

    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "Weather", &pTouchEventListener, getUiRender( pTouchEventListener ) ) );
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
