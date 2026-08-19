// SPDX-FileCopyrightText: 2025-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"
#include "FleetUtils.h"

#include <API/GEM_VRP.h>
#include <API/GEM_MapView.h>
#include <API/GEM_Markers.h>
#include <iomanip>

int main( int argc, char** argv )
{
    // Get new project API token from:
    // https://developer.magiclane.com/api/projects
    Environment::HandleHelpOption( argc, argv );

    std::string projectApiToken = Environment::ResolveApiToken( argc, argv );

    // Sdk objects can be created & used below this line
    Environment::SdkSession session( projectApiToken, { argc > 1 && argv[1][0] != '-' ? argv[1] : "" } ); // SDK API debug logging path

    if( GEM_GET_API_ERROR() != gem::KNoError )
        return GEM_GET_API_ERROR();

    {
        ProgressListener listener;
        gem::vrp::Service serv;

        // This input is modelled on AddOptimWithFixedOrdersSequences, the fixed-sequence example that solves,
        // and on OptimizeSingleVehicle, the JSON example with the same shape (one vehicle, one departure, one
        // destination) that solves. Every value that differed from both of them has been brought into line:
        //
        //   sequenceOptions was 3 = OSO_InPairs. This example is about OSO_InFixedSequence, which is 2.
        //     InPairs is a different feature with a data contract this input cannot meet - each pair must be a
        //     pickup THEN a delivery, with the delivery's packages, weight and cube all >= the pickup's - and
        //     the pairs it declared were delivery -> delivery. OptimizeSequencePairs has the mirror image of
        //     this bug: 2 with 3-element groups where it wants 3 with 2-element ones. The values look swapped.
        //   startDate was the STRING "1596758400000". Every example that solves passes it as a number.
        //   The orders carried no revenue field, while maxRevenue was 100. Both working JSON examples set a
        //     revenue on every order and leave the cap far above their total.
        //   ignoreTimeWindow was true and balancedRoutes was 1. The reference sets neither.
        //   The capacities were the tightest of any example here; they now match OptimizeSingleVehicle's,
        //     which leaves them non-binding, exactly as zero-weight orders do in the reference.
        //
        // The five orders in the sequence carry the vehicle's full shift as their window, so nothing but the
        // adjacency itself constrains them. That matters: InFixedSequence means strictly consecutive stops,
        // and a narrow window on any member of the chain is what makes it unsatisfiable.
        std::string input = R"(
        {
          "configurationParameters": {
            "name": "Berlin - Optimize with fixed sequence",
            "ignoreTimeWindow": false,
            "allowDroppingOrders": false,
            "groupingOrders": false,
            "balancedRoutes": 0,
            "optimizationCriterion": 0,
            "arrangeCriterion": 0,
            "optimizationQuality": 2,
            "maxTimeToOptimize": 300,
            "maxWaitTime": 18000,
            "routeType": 2,
            "restrictions": 0,
            "distanceUnit": 0,
            "orderSequenceOptions": [
              {
                "sequenceOptions": 2,
                "ordersSequence": [[12, 2, 14, 8, 10]]
              }
            ]
          },
          "orders": [
            { "alias": "Order 1", "location": [52.5125, 13.3845], "type": 0, "numberOfPackages": 2, "weight": 2.8, "cube": 1.1, "timeWindow": [420, 780], "serviceTime": 5, "revenue": 8, "priority": 0 },
            { "alias": "Order 2", "location": [52.4987, 13.4123], "type": 1, "numberOfPackages": 1, "weight": 1.9, "cube": 0.7, "timeWindow": [480, 840], "serviceTime": 5, "revenue": 6, "priority": 0 },
            { "alias": "Order 3", "location": [52.5213, 13.3678], "type": 1, "numberOfPackages": 3, "weight": 3.4, "cube": 1.6, "timeWindow": [420, 1439], "serviceTime": 5, "revenue": 9, "priority": 0 },
            { "alias": "Order 4", "location": [52.4856, 13.4289], "type": 0, "numberOfPackages": 1, "weight": 1.5, "cube": 0.8, "timeWindow": [600, 960], "serviceTime": 5, "revenue": 5, "priority": 0 },
            { "alias": "Order 5", "location": [52.5367, 13.3912], "type": 0, "numberOfPackages": 2, "weight": 2.6, "cube": 1.3, "timeWindow": [660, 1020], "serviceTime": 5, "revenue": 7, "priority": 0 },
            { "alias": "Order 6", "location": [52.5045, 13.4356], "type": 1, "numberOfPackages": 1, "weight": 1.7, "cube": 0.9, "timeWindow": [720, 1080], "serviceTime": 5, "revenue": 5, "priority": 0 },
            { "alias": "Order 7", "location": [52.4912, 13.3734], "type": 0, "numberOfPackages": 2, "weight": 2.9, "cube": 1.2, "timeWindow": [780, 1140], "serviceTime": 5, "revenue": 8, "priority": 0 },
            { "alias": "Order 8", "location": [52.5189, 13.4201], "type": 1, "numberOfPackages": 1, "weight": 1.6, "cube": 0.6, "timeWindow": [840, 1200], "serviceTime": 5, "revenue": 6, "priority": 0 },
            { "alias": "Order 9", "location": [52.5298, 13.3567], "type": 0, "numberOfPackages": 3, "weight": 3.7, "cube": 1.8, "timeWindow": [420, 1439], "serviceTime": 5, "revenue": 10, "priority": 0 },
            { "alias": "Order 10", "location": [52.4867, 13.4089], "type": 1, "numberOfPackages": 1, "weight": 1.8, "cube": 0.8, "timeWindow": [960, 1320], "serviceTime": 5, "revenue": 6, "priority": 0 },
            { "alias": "Order 11", "location": [52.5412, 13.3823], "type": 0, "numberOfPackages": 2, "weight": 2.5, "cube": 1.4, "timeWindow": [420, 1439], "serviceTime": 5, "revenue": 7, "priority": 0 },
            { "alias": "Order 12", "location": [52.4967, 13.4445], "type": 1, "numberOfPackages": 1, "weight": 1.4, "cube": 0.7, "timeWindow": [450, 810], "serviceTime": 5, "revenue": 5, "priority": 0 },
            { "alias": "Order 13", "location": [52.5156, 13.3589], "type": 0, "numberOfPackages": 2, "weight": 3.1, "cube": 1.5, "timeWindow": [420, 1439], "serviceTime": 5, "revenue": 9, "priority": 0 },
            { "alias": "Order 14", "location": [52.4823, 13.4234], "type": 0, "numberOfPackages": 1, "weight": 1.9, "cube": 0.9, "timeWindow": [570, 930], "serviceTime": 5, "revenue": 6, "priority": 0 },
            { "alias": "Order 15", "location": [52.5278, 13.3712], "type": 1, "numberOfPackages": 3, "weight": 3.6, "cube": 1.7, "timeWindow": [420, 1439], "serviceTime": 5, "revenue": 10, "priority": 0 }
          ],
          "departures": [
            { "alias": "West Depot", "location": [52.5078, 13.3456] }
          ],
          "destinations": [
            { "alias": "East Terminal", "location": [52.5234, 13.4567] }
          ],
          "vehicles": [
            { "name": "Vehicle 1", "type": 0, "maxLoadWeight": 300, "maxLoadCube": 50, "startTime": 420, "endTime": 1439, "licensePlate": "AA-001-AA", "consumption": 6.5 }
          ],
          "vehiclesConstraints": [
            { "startDate": 1596758400000, "maxNumberOfPackages": 200, "minNumberOfOrders": 0, "maxNumberOfOrders": 99999999, "minDistance": 0, "maxDistance": 99999999, "maxRevenue": 9999 }
          ],
          "matrixBuildType": 1
        }
        )";

        // map initialization
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext( Environment::WindowFrameworks::Available, "OptimizeFixedSequence" );
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce( oglContext, &mapListener );

        gem::LandmarkList lmks;
        gem::CoordinatesList coordsList;

        JsonParser parser( input );
        JsonValue object = parser.parse();

        auto addLandmarks = [&]( JsonValue list, gem::Icon::Core icon )
        {
            for( auto elem : list.asArray() )
            {
                JsonArray loc = elem["location"].asArray();
                gem::Coordinates coords( loc[0].asNumber(), loc[1].asNumber(), 0.0 );
                gem::Landmark lmk;
                lmk.setName( elem["alias"].asString() );
                lmk.setCoordinates( coords );
                lmk.setImage( icon );
                lmks.push_back( lmk );
                coordsList.push_back( coords );
            }
        };

        // create landmarks for departures, orders and destinations
        addLandmarks( object["departures"], gem::Icon::Core::GreenBall );
        addLandmarks( object["orders"], gem::Icon::Core::BlueBall );
        addLandmarks( object["destinations"], gem::Icon::Core::RedBall );

        // display landmarks on the map
        mapView->activateHighlight( lmks, gem::HO_ShowLandmark | gem::HO_NoFading );
        gem::PolygonGeographicArea area( coordsList );
        mapView->centerOnArea( area, 55 );
        WAIT_UNTIL( std::bind( &MapViewListenerImpl::IsFinished, &mapListener ), 15000 );

        // create optimize request
        gem::vrp::Request request;
        listener.Reset();
        int ret = serv.optimize( &listener, input, request );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 60000 );

        gem::LargeInteger solutionId = request.entityId;

        WAIT_UNTIL(
            [&]()
            {
                listener.Reset();
                serv.getRequest( &listener, request, request.id );
                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 7000 );
                return request.status == gem::vrp::ERequestStatus::eFinished;
            },
            120000 );

        // retrieve solution
        std::shared_ptr<std::string> output = std::make_shared<std::string>();
        listener.Reset();
        ret = serv.getSolutionJson( &listener, solutionId, output );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        // check if response was succesful and display routes on the map
        // output must be non-empty: JsonParser throws std::runtime_error( "Unexpected end of input" ) on
        // an empty string, and nothing here catches it, so an empty solution would take down the example.
        if( listener.IsFinished() && listener.GetError() == gem::KNoError && ret == gem::KNoError && output && !output->empty() )
        {
            std::cout << "Problem optimized successfully" << std::endl;

            JsonParser parser2( *output );
            JsonValue object2 = parser2.parse();
            JsonArray routesList = object2["routes"].asArray();

            // Draw each route with different color
            std::vector<gem::Rgba> baseColors = {
                gem::Rgba( 255, 0, 0, 255 ),   // red
                gem::Rgba( 0, 255, 0, 255 ),   // green
                gem::Rgba( 0, 0, 255, 255 ),   // blue
                gem::Rgba( 255, 255, 0, 255 ), // yellow
                gem::Rgba( 255, 0, 255, 255 ), // magenta
                gem::Rgba( 0, 255, 255, 255 )  // cyan
            };

            gem::CoordinatesList allShapes;
            for( size_t i = 0; i < routesList.size(); ++i )
            {
                gem::CoordinatesList shape = Utils::decodePolyline( routesList[i]["shape"].asString() );
                auto col = gem::MarkerCollection( gem::EMarkerType::MT_Polyline, "route" + std::to_string( i + 1 ) );
                col.add( gem::Marker( shape ) );
                gem::MarkerCollectionRenderSettings settings;
                settings.polylineInnerColor = baseColors[i % baseColors.size()];
                mapView->preferences().markers().add( col, settings );

                allShapes.insert( allShapes.end(), shape.begin(), shape.end() );
            }

            ret = WAIT_UNTIL( std::bind( &MapViewListenerImpl::IsFinished, &mapListener ), 15000 );

            gem::PolygonGeographicArea routeArea( allShapes );
            mapView->centerOnArea( routeArea, 55 );
            WAIT_UNTIL( std::bind( &MapViewListenerImpl::IsFinished, &mapListener ), 15000 );
            WAIT_UNTIL_WINDOW_CLOSE();
        }
        else
        {
            std::cout << "Optimization failed." << std::endl;
        }
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
