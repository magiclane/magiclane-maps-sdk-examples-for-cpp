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


    {
        gem::vrp::Territory territory;
        territory.setName("Polygon territory");
        territory.setType(gem::vrp::ETerritoryType::TT_Polygon);
        territory.setColor(gem::Rgba(255, 42, 0, 50));

        gem::CoordinatesList data;
        gem::Coordinates point1 = { 46.585964, 0.346915 };
        gem::Coordinates point2 = { 46.593774, 0.340877 };
        gem::Coordinates point3 = { 46.587993, 0.357528 };
        gem::Coordinates point4 = { 46.580442, 0.344825 };
        gem::Coordinates point5 = { 46.587344, 0.339847 };
        data.push_back(point1); data.push_back(point2);
        data.push_back(point3); data.push_back(point4);
        data.push_back(point5);
        territory.setData(data);

        ProgressListener listener;
        gem::vrp::Service serv;
        int res = serv.addTerritory(&listener, territory);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
        {
            std::cout << "Territory added successfully and has the id = " << territory.getId() << "." << std::endl;
            
            gem::CoordinatesList territoryCoords;
            for (int i = 0; i < territory.getData().size(); i++)
                territoryCoords.push_back(territory.getData()[i]);

            MapViewListenerImpl mapListener;
            auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "PolygonTerritory");
            gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(oglContext, &mapListener);

            auto col = gem::MarkerCollection(gem::EMarkerType::MT_Polygon, "Territory");
            col.add(gem::Marker(territoryCoords));

            gem::MarkerCollectionRenderSettings markerCollDisplaySettings;
            markerCollDisplaySettings.polygonFillColor = gem::Rgba(territory.getColor());

            mapView->preferences().markers().add(col, markerCollDisplaySettings);
            mapView->centerOnArea(col.getArea());
            auto ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

            WAIT_UNTIL_WINDOW_CLOSE();
        }
        else
            std::cout << "Territory couldn't be added" << std::endl;

        // The customers returned by the method territory.getCustomers() are only the ones that were previously saved using the method serv.addCustomer(), not the customers that were used in optimizations.
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
