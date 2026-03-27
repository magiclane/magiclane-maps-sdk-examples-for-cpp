// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"
#include "BitmapImpl.h"

#include <API/GEM_MapView.h>
#include <API/GEM_Weather.h>
#include <API/GEM_Debug.h>

#include <sstream>
#include <imgui.h>

#define PIXELS_WEATHER_ICON_SIZE 48

// Derive from the standard touch event handler class which makes the map view interactive

class MyTouchEventListener : public CTouchEventListener
{
private:
    std::vector<std::string> strvecCurrent;
    std::vector<std::string> strvecHourly;
    std::vector<std::string> strvecDaily;
    std::vector<gem::Image> imgvecCurrent;
    std::vector<gem::Image> imgvecHourly;
    std::vector<gem::Image> imgvecDaily;
    bool renderPanel;
    bool renderMenu;
    gem::Coordinates selectedCoordinates;

public:
    enum class WeatherType
    {
        Current = 0,
        HourlyForecast,
        DailyForecast,
        None
    };
    std::vector<std::string> getStrVecOutput(MyTouchEventListener::WeatherType weatherType = MyTouchEventListener::WeatherType::Current)
    {
        return weatherType == MyTouchEventListener::WeatherType::Current ? strvecCurrent
            : weatherType == MyTouchEventListener::WeatherType::HourlyForecast ? strvecHourly : strvecDaily;
    }
    std::vector<gem::Image> getImageVecOutput(MyTouchEventListener::WeatherType weatherType = MyTouchEventListener::WeatherType::Current)
    {
        return weatherType == MyTouchEventListener::WeatherType::Current ? imgvecCurrent
            : weatherType == MyTouchEventListener::WeatherType::HourlyForecast ? imgvecHourly : imgvecDaily;
    }
    void setRenderPanel(bool isrender) { renderPanel = isrender; }
    bool getRenderPanel() { return renderPanel; }
    void setRenderMenu(bool isrender) { renderMenu = isrender; }
    bool getRenderMenu() { return renderMenu; }

    // This function from the standard touch event handler for the map view is
    // overridden to add our own processing - enabling drawing a route by dragging
    // after a single or double click on the map.

    void handleTouchEvent(int eventType, int pointerId, int x, int y)
    {
        auto mapView = getMapViewPointer();
        setCursorPosition(x, y);
        gem::Xy mousePos(x, y);
        if (mapView.get() != nullptr)
        {
            mapView->getScreen()->handleTouchEvent((gem::ETouchEvent)eventType, pointerId, mousePos);
        }
        else
        {
            GEM_LOGE("null mapView!");
        }
        auto coord = mapView->transformScreenToWgs(mousePos);
        GEM_LOGE("xy( %d, %d ) mapcoord lon,lat( %f, %f )\n", x, y, coord.getLongitude(), coord.getLatitude());

        if (eventType == gem::ETouchEvent::TE_Down)
        {
            gem::CoordinatesList coordinatesList;
            coordinatesList.push_back(coord);
            selectedCoordinates = coord;
            setRenderPanel(true);
            setRenderMenu(true);

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

            strstream.str("");
            strstream << "Lon, Lat " << coord.getLongitude() << ", " << coord.getLatitude();
            strvecCurrent.push_back(strstream.str());
            strvecHourly.push_back(strstream.str());
            strvecDaily.push_back(strstream.str());

            int errorn = gem::weather::Service().getCurrent(coordinatesList, locationForecastListResultCurrentWeatherConditions, &weatherListenerCurrent);
            if (errorn != gem::KNoError)
            {
                GEM_LOGE("error( %d ) current weather\n", errorn);
            }
            else
            {
                auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &weatherListenerCurrent), 15000);
                int index = 0;
                GEM_LOGE("CURRENT weather vec size main %d forecast front %d", (int)locationForecastListResultCurrentWeatherConditions.size(),
                    (int)locationForecastListResultCurrentWeatherConditions.front().forecast.size());
                strvecCurrent.push_back("Current Weather Conditions");
                for (auto& e : locationForecastListResultCurrentWeatherConditions.front().forecast.front().params)
                {
                    GEM_LOGE("Got CURRENT Weather [ %d ] ( %s, %s )( %f )\n",
                        index,
                        e.name.toStdString().c_str(),
                        std::string("°") == e.unit.toStdString() ? "degrees" : std::string("°C") == e.unit.toStdString() ? "deg C" : e.unit.toStdString().c_str(),
                        e.value
                    );
                    strstream.str("");
                    strstream << "   " << e.name.toStdString() << " "
                        << (std::string("°") == e.unit.toStdString() ? "degrees" : std::string("°C") == e.unit.toStdString() ? "deg C"
                            //: std::string("Sunrise") == e.unit.toStdString() || std::string("Sunset") == e.unit.toStdString() ? e.value. 
                            : e.unit.toStdString().c_str())
                        << " " << e.value;
                    strvecCurrent.push_back(strstream.str());
                    index++;
                }
                imgvecCurrent.push_back(locationForecastListResultCurrentWeatherConditions.front().forecast.front().image);
            }
            int hours = 36;
            errorn = gem::weather::Service().getHourlyForecast(hours, coordinatesList, locationForecastListResultHourlyWeatherConditions, &weatherListenerHourly);
            if (errorn != gem::KNoError)
            {
                GEM_LOGE("error( %d ) hourly weather\n", errorn);
            }
            else
            {
                auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &weatherListenerHourly), 15000);
                int index = 0;
                GEM_LOGE("HOURLY weather vec size main %d forecast front %d", (int)locationForecastListResultHourlyWeatherConditions.size(),
                    (int)locationForecastListResultHourlyWeatherConditions.front().forecast.size());
                strvecHourly.push_back("Hourly Weather Forecast");
                for (auto& e : locationForecastListResultHourlyWeatherConditions.front().forecast.front().params)
                {
                    GEM_LOGE("Got HOURLY Weather [ %d ] ( %s, %s )( %.1f )\n",
                        index,
                        e.name.toStdString().c_str(),
                        std::string("°") == e.unit.toStdString() ? "degrees" : std::string("°C") == e.unit.toStdString() ? "deg C" : e.unit.toStdString().c_str(),
                        e.value
                    );
                    strstream.str("");
                    strstream << "   " << e.name.toStdString() << " "
                        << (std::string("°") == e.unit.toStdString() ? "degrees" : std::string("°C") == e.unit.toStdString() ? "deg C" : e.unit.toStdString().c_str())
                        << " " << e.value;
                    strvecHourly.push_back(strstream.str());
                    index++;
                }
                imgvecHourly.push_back(locationForecastListResultCurrentWeatherConditions.front().forecast.front().image);
            }
            errorn = gem::weather::Service().getDailyForecast(10, coordinatesList, locationForecastListResultDailyWeatherConditions, &weatherListenerDaily);
            if (errorn != gem::KNoError)
            {
                GEM_LOGE("error( %d ) daily weather\n", errorn);
            }
            else
            {
                auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &weatherListenerDaily), 15000);
                int index = 0;
                strvecDaily.push_back("Daily Weather Forecast");
                for (auto& e : locationForecastListResultDailyWeatherConditions.front().forecast.front().params)
                {
                    GEM_LOGE("Got DAILY Weather [ %d ] ( %s, %s )( %.1f )\n",
                        index,
                        e.name.toStdString().c_str(),
                        std::string("°") == e.unit.toStdString() ? "degrees" : std::string("°C") == e.unit.toStdString() ? "deg C" : e.unit.toStdString().c_str(),
                        e.value
                    );
                    strstream.str("");
                    strstream << "   " << e.name.toStdString() << " "
                        << (std::string("°") == e.unit.toStdString() ? "degrees" : std::string("°C") == e.unit.toStdString() ? "deg C" : e.unit.toStdString().c_str())
                        << " " << e.value;
                    strvecDaily.push_back(strstream.str());
                    index++;
                }
                imgvecDaily.push_back(locationForecastListResultCurrentWeatherConditions.front().forecast.front().image);
            }
        }
    }
};

// Create an interactive map view
MyTouchEventListener pTouchEventListener;
MyTouchEventListener::WeatherType weatherType = MyTouchEventListener::WeatherType::None;

auto getUiRender()
{
    return std::bind([](gem::StrongPointer<gem::MapView> mapView)
        {
            ImGuiIO& io = ImGui::GetIO();
            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 12), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(0, 0));
            if (pTouchEventListener.getRenderMenu())
            {
                ImGui::SetNextWindowBgAlpha(1.0f);
                ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->Size.x * 0.7, main_viewport->WorkPos.y + 0), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(0, 0));// main_viewport->WorkSize.x, 108));
                ImGui::GetStyle().WindowRounding = 0.0f;
                ImGui::Begin("panel2", nullptr, ImGuiWindowFlags_NoMove
                    | ImGuiWindowFlags_NoDecoration
                    | ImGuiWindowFlags_AlwaysAutoResize
                    | ImGuiWindowFlags_NoBackground
                    | ImGuiWindowFlags_NoSavedSettings);
                if (ImGui::Button("Current Weather"))
                {
                    weatherType = MyTouchEventListener::WeatherType::Current;
                    pTouchEventListener.setRenderPanel(true);
                }
                if (ImGui::Button("Hourly Weather Forecast"))
                {
                    weatherType = MyTouchEventListener::WeatherType::HourlyForecast;
                    pTouchEventListener.setRenderPanel(true);
                }
                if (ImGui::Button("Daily Weather Forecast"))
                {
                    weatherType = MyTouchEventListener::WeatherType::DailyForecast;
                    pTouchEventListener.setRenderPanel(true);
                }
                if (ImGui::Button("Close"))
                {
                    weatherType = MyTouchEventListener::WeatherType::None;
                    pTouchEventListener.setRenderPanel(false);
                    pTouchEventListener.setRenderMenu(false);
                }
                ImGui::End();
            }
            if (pTouchEventListener.getRenderPanel())
            {
                int index = 0;
                switch (weatherType)
                {
                case MyTouchEventListener::WeatherType::Current:
                {
                    auto strvec = pTouchEventListener.getStrVecOutput();
                    ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                        | ImGuiWindowFlags_NoDecoration
                        | ImGuiWindowFlags_AlwaysAutoResize
                        | ImGuiWindowFlags_NoCollapse
                        | ImGuiWindowFlags_NoSavedSettings);
                    if (ImGui::BeginTable("weather_panel", 2))
                    {
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, PIXELS_WEATHER_ICON_SIZE + 12);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

                        index = 0;
                        for (auto& s : strvec)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);

                            //////////////////////////////////////////////////////////
                            // WEATHER ICON
                            //////////////////////////////////////////////////////////
                            if ( index == 1 ) // for current weather, show icon only for the title element at index 1
                            {
                                const ImVec2 weatherIconSize{ PIXELS_WEATHER_ICON_SIZE, PIXELS_WEATHER_ICON_SIZE };
                                gem::Rgba color(255, 0, 255, 255);
                                gem::AbstractGeometryImageRenderSettings settings(gem::Rgba::white(), gem::Rgba::black(), color);
                                auto bitmap = gem::StrongPointerFactory<BitmapImpl>(70, 70);
                                auto imgvec = pTouchEventListener.getImageVecOutput();
                                imgvec.front().render(*bitmap);
                                unsigned int textureId = BitmapImpl::LoadTextureIntoGPU(bitmap->size().width, bitmap->size().height, bitmap->begin());
                                ImGui::Image(textureId, weatherIconSize);
                            }
                            index++;

                            ImGui::TableNextColumn();
                            ImGui::Text("%s", s.c_str());
                        }
                        ImGui::EndTable();
                    }
                    ImGui::End();
                    break;
                }
                case MyTouchEventListener::WeatherType::HourlyForecast:
                {
                    auto strvec = pTouchEventListener.getStrVecOutput(MyTouchEventListener::WeatherType::HourlyForecast);
                    ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                        | ImGuiWindowFlags_NoDecoration
                        | ImGuiWindowFlags_AlwaysAutoResize
                        | ImGuiWindowFlags_NoCollapse
                        | ImGuiWindowFlags_NoSavedSettings);
                    if (ImGui::BeginTable("weather_panel", 2))
                    {
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, PIXELS_WEATHER_ICON_SIZE + 12);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

                        index = 0;
                        for (auto& s : strvec)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);

                            //////////////////////////////////////////////////////////
                            // WEATHER ICON
                            //////////////////////////////////////////////////////////
                            if (index == 1)
                            {
                                const ImVec2 weatherIconSize{ PIXELS_WEATHER_ICON_SIZE, PIXELS_WEATHER_ICON_SIZE };
                                gem::Rgba color(255, 0, 255, 255);
                                gem::AbstractGeometryImageRenderSettings settings(gem::Rgba::white(), gem::Rgba::black(), color);
                                auto bitmap = gem::StrongPointerFactory<BitmapImpl>(70, 70);
                                auto imgvec = pTouchEventListener.getImageVecOutput(MyTouchEventListener::WeatherType::HourlyForecast);
                                imgvec.front().render(*bitmap);
                                unsigned int textureId = BitmapImpl::LoadTextureIntoGPU(bitmap->size().width, bitmap->size().height, bitmap->begin());
                                ImGui::Image(textureId, weatherIconSize);
                            }
                            index++;

                            ImGui::TableNextColumn();
                            ImGui::Text("%s", s.c_str());
                        }
                        ImGui::EndTable();
                    }
                    ImGui::End();
                    break;
                }
                case MyTouchEventListener::WeatherType::DailyForecast:
                {
                    auto strvec = pTouchEventListener.getStrVecOutput(MyTouchEventListener::WeatherType::DailyForecast);
                    ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                        | ImGuiWindowFlags_NoDecoration
                        | ImGuiWindowFlags_AlwaysAutoResize
                        | ImGuiWindowFlags_NoCollapse
                        | ImGuiWindowFlags_NoSavedSettings);
                    if (ImGui::BeginTable("weather_panel", 2))
                    {
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, PIXELS_WEATHER_ICON_SIZE + 12);
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

                        index = 0;
                        for (auto& s : strvec)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);

                            //////////////////////////////////////////////////////////
                            // WEATHER ICON
                            //////////////////////////////////////////////////////////
                            if (index == 0)
                            {
                                const ImVec2 weatherIconSize{ PIXELS_WEATHER_ICON_SIZE, PIXELS_WEATHER_ICON_SIZE };
                                gem::Rgba color(255, 0, 255, 255);
                                gem::AbstractGeometryImageRenderSettings settings(gem::Rgba::white(), gem::Rgba::black(), color);
                                auto bitmap = gem::StrongPointerFactory<BitmapImpl>(70, 70);
                                auto imgvec = pTouchEventListener.getImageVecOutput(MyTouchEventListener::WeatherType::DailyForecast);
                                imgvec.front().render(*bitmap);
                                unsigned int textureId = BitmapImpl::LoadTextureIntoGPU(bitmap->size().width, bitmap->size().height, bitmap->begin());
                                ImGui::Image(textureId, weatherIconSize);
                            }
                            index++;

                            ImGui::TableNextColumn();
                            ImGui::Text("%s", s.c_str());
                        }
                        ImGui::EndTable();
                    }
                    ImGui::End();
                    break;
                }
                case MyTouchEventListener::WeatherType::None:
                default:
                {
                    auto strvec = pTouchEventListener.getStrVecOutput();
                    if (!strvec.empty())
                    {
                        ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                            | ImGuiWindowFlags_NoDecoration
                            | ImGuiWindowFlags_AlwaysAutoResize
                            | ImGuiWindowFlags_NoCollapse
                            | ImGuiWindowFlags_NoSavedSettings);
                        for (auto& s : strvec)
                        {
                            ImGui::Text("%s", s.c_str());
                            break;
                        }
                        ImGui::End();
                    }
                    break;
                }
                } //switch
                // do a zoom to the current zoom level (no-op) to trigger a refresh;
                mapView.get()->setZoomLevel(mapView.get()->getZoomLevel());
            }
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


    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce( session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "Weather", &pTouchEventListener, getUiRender() ) );
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
