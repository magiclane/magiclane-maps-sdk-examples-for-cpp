// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_SearchService.h>

#include <imgui.h>
#include "MultiSearchViewModel.h"

void pushButtonStyle() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.0f, .0f, 1.f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.5f, .0f, 1.f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, .0f, 1.f, 1.f));
}

auto renderUI(gem::StrongPointer<gem::MapView> mapView, MultiSearchViewModel* vm)
{

    ImGuiIO& io = ImGui::GetIO();
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
    ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings);

    // Position around which to search for POIs
    auto aroundPositionSearch = MultiSearchViewModel::EViewPurpose::AROUND_POSITION_SEARCH;
    auto position1 = vm->getTargetPos(aroundPositionSearch);
    auto lat1 = position1.getLatitude(), lon1 = position1.getLongitude();
    ImGui::PushItemWidth(128);
    if (ImGui::InputDouble("Lat", &lat1, 0., 0., "%f")) {
        vm->setTargetPosition(aroundPositionSearch, lat1, lon1);
    };
    if (ImGui::InputDouble("Lon", &lon1, 0., 0., "%f")) {
        vm->setTargetPosition(aroundPositionSearch, lat1, lon1);
    };
    ImGui::PopItemWidth();
    pushButtonStyle();
    ImGui::Spacing();
    if (ImGui::Button("Search POIs around position"))
    {
        // Perform the search
        gem::LandmarkList results;

        ProgressListener searchListener;

        // Position around which to search, given as lat,lon coordinates in degrees.
        gem::SearchService().searchAroundPosition(results, &searchListener, vm->getTargetPos(MultiSearchViewModel::EViewPurpose::AROUND_POSITION_SEARCH));

        auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &searchListener), 15000);

        if (results.size() > 0)
        {
            mapView->centerOnCoordinates(results[0].getCoordinates(), 80);
            mapView->activateHighlight(results);
        }
    }
    ImGui::PopStyleColor(4);

    // Position around which to search for text input below

    auto textSearch = MultiSearchViewModel::EViewPurpose::TEXT_SEARCH;
    auto position2 = vm->getTargetPos(textSearch);
    auto lat2 = position2.getLatitude(), lon2 = position2.getLongitude();
    ImGui::PushItemWidth(128);
    if (ImGui::InputDouble("Lat position", &lat2, 0., 0., "%f")) {
        vm->setTargetPosition(textSearch, lat2, lon2);
    }
    if (ImGui::InputDouble("Lon position", &lon2, 0., 0., "%f")) {
        vm->setTargetPosition(textSearch, lat2, lon2);
    };
    ImGui::PopItemWidth();

    // Text to search for around above position
    auto searchText = vm->getSearchText(textSearch);
    if (ImGui::InputTextWithHint("Search text", "Enter search text here", searchText, vm->getSearchSize())) {
        vm->setSearchText(textSearch, searchText);
    };
    pushButtonStyle();
    ImGui::Spacing();
    if (ImGui::Button("Search free text around position"))
    {
        // Perform the search
        gem::LandmarkList results;

        ProgressListener searchListener;

        // Text to search for; position given as lat,lon coordinates in degrees
        gem::SearchService().search(results, &searchListener, vm->getSearchText(textSearch), vm->getTargetPos(textSearch));

        auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &searchListener), 15000);

        if (results.size() > 0)
        {
            mapView->centerOnCoordinates(results[0].getCoordinates(), 85, gem::Xy(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 2000));
            mapView->activateHighlight(results);
        }
    }
    ImGui::PopStyleColor(4);

    // Rectangular area within which to search for text input below
    // Upper-left,lower-right bounding box given as lat,lon coordinate pairs in degrees.
    gem::RectangleGeographicArea searchArea = vm->getSearchArea();
    auto topLeftLat = searchArea.getTopLeft().getLatitude(), topLeftLon = searchArea.getTopLeft().getLongitude();
    auto bottomRightLat = searchArea.getBottomRight().getLatitude(), bottomRightLon = searchArea.getBottomRight().getLongitude();
    ImGui::PushItemWidth(128);

    if (ImGui::InputDouble("Lat Upper left", &topLeftLat, 0., 0., "%f")) {
        vm->setSearchAreaTopLeft(topLeftLat, topLeftLon);
    };
    if (ImGui::InputDouble("Lon Upper left", &topLeftLon, 0., 0., "%f")) {
        vm->setSearchAreaTopLeft(topLeftLat, topLeftLon);
    };
    if (ImGui::InputDouble("Lat Lower right", &bottomRightLat, 0., 0., "%f")) {
        vm->setSearchAreaBottomRight(bottomRightLat, bottomRightLon);
    };
    if (ImGui::InputDouble("Lon Lower right", &bottomRightLon, 0., 0., "%f")) {
        vm->setSearchAreaBottomRight(bottomRightLat, bottomRightLon);
    };

    auto byAreaSearch = MultiSearchViewModel::EViewPurpose::BY_AREA_SEARCH;
    auto position3 = vm->getTargetPos(byAreaSearch);
    auto lat3 = position3.getLatitude(), lon3 = position3.getLongitude();
    if (ImGui::InputDouble("Lat search around", &lat3, 0., 0., "%f")) {
        vm->setTargetPosition(byAreaSearch, lat3, lon3);
    };
    if (ImGui::InputDouble("Lon search around", &lon3, 0., 0., "%f")) {
        vm->setTargetPosition(byAreaSearch, lat3, lon3);
    };
    ImGui::PopItemWidth();

    // Text to search for within above rectangular area
    auto searchTextByArea = vm->getSearchText(byAreaSearch);
    if (ImGui::InputTextWithHint("Area search text", "Enter search text here", searchTextByArea, vm->getSearchSize())) {
        vm->setSearchText(byAreaSearch, searchTextByArea);
    };
    pushButtonStyle();
    ImGui::Spacing();
    if (ImGui::Button("Search free text within rectangular area"))
    {
        // Perform the search
        gem::LandmarkList results;

        ProgressListener searchListener;

        // Text to search for
        gem::SearchService().search(results, &searchListener, vm->getSearchText(byAreaSearch), position3, gem::SearchPreferences(), searchArea);

        auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &searchListener), 15000);

        if (results.size() > 0)
        {
            mapView->centerOnCoordinates(results[0].getCoordinates());
            mapView->activateHighlight(results);
        }
    }
    ImGui::PopStyleColor(4);
    ImGui::End();
}

auto getUIRender(gem::StrongPointer<gem::MapView> mapView) {
    MultiSearchViewModel* vm = MultiSearchViewModel::getInstance();
    renderUI(mapView, vm);
}

int main(int argc, char** argv)
{
    // Get new project API token from:
    // https://developer.magiclane.com/api/projects
    std::string projectApiToken = "";

#if defined(API_TOKEN)
    projectApiToken = std::string(API_TOKEN);
#else
    auto value = std::getenv("GEM_TOKEN");
    if (value != nullptr)
        projectApiToken = value;
#endif

    // Sdk objects can be created & used below this line
    Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


    // Create an interactive map view
    CTouchEventListener pTouchEventListener;
    auto binding = std::bind(getUIRender, std::placeholders::_1);
    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::ImGUI, "MultiSearch", &pTouchEventListener, binding));
    if (!mapView)
    {
        GEM_LOGE("Error creating gem::MapView: %d", GEM_GET_API_ERROR());
    }

    WAIT_UNTIL_WINDOW_CLOSE();

    return 0;
}

#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")

int WINAPI WinMain(HINSTANCE hInstance, // Instance
    HINSTANCE hPrevInstance, // Previous Instance
    LPSTR lpCmdLine, // Command Line Parameters
    int nCmdShow)
{
    main(0, nullptr);

    return 0;
}

#endif
