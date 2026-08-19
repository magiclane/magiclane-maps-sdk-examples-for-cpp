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
    Environment::HandleHelpOption( argc, argv );

    std::string projectApiToken = Environment::ResolveApiToken( argc, argv );

    // Sdk objects can be created & used below this line
    Environment::SdkSession session( projectApiToken, { argc > 1 && argv[1][0] != '-' ? argv[1] : "" } ); // SDK API debug logging path

    if( GEM_GET_API_ERROR() != gem::KNoError ) // check for errors after session creation
        return GEM_GET_API_ERROR();

    {
        gem::vrp::Territory territory;
        territory.setName( "Polygon territory" );
        territory.setType( gem::vrp::ETerritoryType::TT_Polygon );
        territory.setColor( gem::Rgba( 255, 42, 0, 50 ) );

        // A 5-corner territory over central Paris. It has to sit where the ORDERS are: the orders moved to
        // Paris, so a territory left in Poitiers matches nothing and the example looks broken when it is
        // actually working. This one contains the 1er / 3e / 5e / 6e stops and excludes the other eight, so
        // a correct result is visibly a subset rather than everything or nothing.
        gem::CoordinatesList data;
        gem::Coordinates point1 = { 48.870000, 2.330000 };
        gem::Coordinates point2 = { 48.868000, 2.372000 };
        gem::Coordinates point3 = { 48.848000, 2.370000 };
        gem::Coordinates point4 = { 48.838000, 2.345000 };
        gem::Coordinates point5 = { 48.852000, 2.325000 };
        data.push_back( point1 );
        data.push_back( point2 );
        data.push_back( point3 );
        data.push_back( point4 );
        data.push_back( point5 );
        territory.setData( data );

        ProgressListener listener;
        gem::vrp::Service serv;
        int res = serv.addTerritory( &listener, territory );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        if( listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError )
        {
            std::cout << "Territory added successfully and has the id = " << territory.getId() << "." << std::endl;

            gem::CoordinatesList territoryCoords;
            for( int i = 0; i < territory.getData().size(); i++ )
                territoryCoords.push_back( territory.getData()[i] );

            MapViewListenerImpl mapListener;
            auto oglContext = session.produceOpenGLContext( Environment::WindowFrameworks::Available, "PolygonTerritory" );
            gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce( oglContext, &mapListener );

            auto col = gem::MarkerCollection( gem::EMarkerType::MT_Polygon, "Territory" );
            col.add( gem::Marker( territoryCoords ) );

            gem::MarkerCollectionRenderSettings markerCollDisplaySettings;
            markerCollDisplaySettings.polygonFillColor = gem::Rgba( territory.getColor() );

            mapView->preferences().markers().add( col, markerCollDisplaySettings );
            mapView->centerOnArea( col.getArea() );
            WAIT_UNTIL( std::bind( &MapViewListenerImpl::IsFinished, &mapListener ), 15000 );

            WAIT_UNTIL_WINDOW_CLOSE();
        }
        else
            std::cout << "Territory couldn't be added" << std::endl;

        // The customers returned by the method territory.getCustomers() are only the ones that were previously saved using the method serv.addCustomer(), not the customers that were used in optimizations.
    }

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
