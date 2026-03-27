// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>
#include <API/GEM_MapView.h>
#include <API/GEM_Markers.h>

#include <iostream>

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
;

    {
        gem::CoordinatesList coords = { gem::Coordinates(47.592266,8.089206),
            gem::Coordinates(48.568867,9.122461),
            gem::Coordinates(50.487137,14.161686),
            gem::Coordinates(49.851322,20.489012),
            gem::Coordinates(49.471581,7.480935),
            gem::Coordinates(46.858112,21.328030),
            gem::Coordinates(51.427933,9.794261),
            gem::Coordinates(46.602039,10.907111),
            gem::Coordinates(48.667385,17.154114),
            gem::Coordinates(46.674530,25.333998),
            gem::Coordinates(48.248165,18.071995),
            gem::Coordinates(48.694458,19.731777),
            gem::Coordinates(51.356140,15.819277),
            gem::Coordinates(51.602428,27.123756),
            gem::Coordinates(51.611862,4.796660),
            gem::Coordinates(49.301449,23.506264),
            gem::Coordinates(49.621101,6.395834),
            gem::Coordinates(48.567471,17.284821),
            gem::Coordinates(49.604858,18.538603),
            gem::Coordinates(51.599285,7.911033),
            gem::Coordinates(46.873657,8.260807),
            gem::Coordinates(47.746155,20.826370),
            gem::Coordinates(52.115623,16.879551),
            gem::Coordinates(50.366787,7.117288),
            gem::Coordinates(46.785622,4.617757),
            gem::Coordinates(51.643478,6.422852),
            gem::Coordinates(47.214272,6.709096),
            gem::Coordinates(51.503738,26.488468),
            gem::Coordinates(51.522953,12.511396),
            gem::Coordinates(50.644344,13.998848),
            gem::Coordinates(46.965187,20.015100),
            gem::Coordinates(49.337257,16.126698),
            gem::Coordinates(52.110031,12.255091),
            gem::Coordinates(50.760151,22.095486),
            gem::Coordinates(50.960152,20.884787),
            gem::Coordinates(47.572002,14.907236) };

        MapViewListenerImpl mapListener;

        auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "Territories");
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(oglContext, &mapListener);

        auto col = gem::MarkerCollection(gem::EMarkerType::MT_Point, "Coordinates marker");
        col.add(coords);

        gem::MarkerCollectionRenderSettings markerCollDisplaySettings;
        markerCollDisplaySettings.pointsGroupingZoomLevel = 0; // don't group them

        mapView->preferences().markers().add(col, markerCollDisplaySettings);
        mapView->centerOnArea(col.getArea());
        auto ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

        ProgressListener listener;
        gem::vrp::Service serv;

        gem::vrp::TerritoryList territories;
        int territoriesNumber = 3;
        int res = serv.generateTerritories(&listener, territories, coords, territoriesNumber); // to generate one territory minimum 3 coordinates are needed, so to create 3 territories, allStops's size should be at least 9
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
        {
            std::cout << "Territories generated successfully" << std::endl;

            for (gem::vrp::Territory territory : territories)
            {
                std::cout << "Territory " << territory.getId() << " has the points:" << std::endl;

                gem::CoordinatesList territoryCoords;
                for (int i = 0; i < territory.getData().size(); i++)
                {
                    std::cout << territory.getData()[i].getLatitude() << "," << territory.getData()[i].getLongitude() << std::endl;
                    territoryCoords.push_back(territory.getData()[i]);
                }

                auto territoryCol = gem::MarkerCollection(gem::EMarkerType::MT_Polygon, territory.getName());
                territoryCol.add(territoryCoords);

                gem::MarkerCollectionRenderSettings markerCollDisplaySettingsTerritory;
                markerCollDisplaySettingsTerritory.polygonFillColor = territory.getColor();

                mapView->preferences().markers().add(territoryCol, markerCollDisplaySettingsTerritory);
            }

            ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

            WAIT_UNTIL_WINDOW_CLOSE();
        }
        else
            std::cout << "Territories couldn't be generated" << std::endl;
    }

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
