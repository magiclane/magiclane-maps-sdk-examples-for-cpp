// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"
#include "FleetUtils.h"

#include <API/GEM_VRP.h>
#include <API/GEM_MapView.h>
#include <API/GEM_Markers.h>
#include<iomanip>

int main( int argc, char** argv )
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
    {
        ProgressListener listener;
        gem::vrp::Service serv;

        std::string input = R"(
        {
          "configurationParameters": {
            "name": "Berlin - test optimization",
            "ignoreTimeWindow": false,
            "allowDroppingOrders": false,
            "groupingOrders": false,
            "balancedRoutes": 0,
            "optimizationCriterion": 0,
            "arrangeCriterion": 0,
            "optimizationQuality": 2,
            "maxTimeToOptimize": 300,
            "maxWaitTime": 18000,
            "routeType": 1,
            "restrictions": 0,
            "distanceUnit": 0
          },
          "orders": [
            {"alias": "Customer 1", "location": [52.540008, 13.404954], "type":0,"numberOfPackages":0,"weight":0,"cube":0,"timeWindow":[420,600],"serviceTime":0,"revenue":0,"priority":0},
            {"alias": "Customer 2", "location": [52.507874, 13.357096], "type":0,"numberOfPackages":0,"weight":0,"cube":0,"timeWindow":[480,660],"serviceTime":0,"revenue":0,"priority":0},
            {"alias": "Customer 3", "location": [52.535126, 13.378906], "type":0,"numberOfPackages":3,"weight":5.1,"cube":2.5,"timeWindow":[540,720],"serviceTime":0,"revenue":12,"priority":0},
            {"alias": "Customer 4", "location": [52.495041, 13.424119], "type":1,"numberOfPackages":8,"weight":15.4,"cube":5.8,"timeWindow":[600,780],"serviceTime":0,"revenue":2.7,"priority":0},
            {"alias": "Customer 5", "location": [52.544308, 13.351266], "type":0,"numberOfPackages":9,"weight":16.2,"cube":8.4,"timeWindow":[660,840],"serviceTime":0,"revenue":7.5,"priority":0},
            {"alias": "Customer 6", "location": [52.548926, 13.425293], "type":1,"numberOfPackages":9,"weight":16.5,"cube":9.7,"timeWindow":[720,900],"serviceTime":0,"revenue":10,"priority":0},
            {"alias": "Customer 7", "location": [52.515697, 13.329315], "type":0,"numberOfPackages":5,"weight":7.5,"cube":2.6,"timeWindow":[780,960],"serviceTime":0,"revenue":13.4,"priority":0},
            {"alias": "Customer 8", "location": [52.485413, 13.398438], "type":1,"numberOfPackages":11,"weight":12.1,"cube":4.6,"timeWindow":[840,1020],"serviceTime":0,"revenue":14.7,"priority":0},
            {"alias": "Customer 9", "location": [52.537384, 13.341675], "type":1,"numberOfPackages":6,"weight":5.2,"cube":1.5,"timeWindow":[900,1080],"serviceTime":0,"revenue":2.9,"priority":0},
            {"alias": "Customer 10", "location": [52.543286, 13.364258], "type":0,"numberOfPackages":13,"weight":6.3,"cube":7.5,"timeWindow":[960,1140],"serviceTime":0,"revenue":8.1,"priority":0},
            {"alias": "Customer 11", "location": [52.518623, 13.376465], "type":0,"numberOfPackages":10,"weight":8,"cube":4.1,"timeWindow":[1020,1200],"serviceTime":0,"revenue":6.2,"priority":0},
            {"alias": "Customer 12", "location": [52.542847, 13.385315], "type":0,"numberOfPackages":6,"weight":4.8,"cube":11.5,"timeWindow":[1080,1260],"serviceTime":0,"revenue":2.3,"priority":0}
          ],
          "departures": [
            {"alias": "Depot 1", "location":[52.520000, 13.405000]},
            {"alias": "Depot 2", "location":[52.486674, 13.357300]}
          ],
          "destinations": [],
          "vehicles": [
            {"name":"Car vehicle 1","type":0,"maxLoadWeight":60,"maxLoadCube":50,"startTime":420,"endTime":1439,"licensePlate":"AA-123-AA","consumption":6.5},
            {"name":"Car vehicle 2","type":0,"maxLoadWeight":60,"maxLoadCube":50,"startTime":420,"endTime":1439,"licensePlate":"AA-124-AA","consumption":6.5}
          ],
          "vehiclesConstraints": [
            {"startDate":1596758400000,"maxNumberOfPackages":53,"minNumberOfOrders":0,"maxNumberOfOrders":99999999,"minDistance":0,"maxDistance":99999999,"maxRevenue":85}
          ],
          "matrixBuildType": 1
        }
    )";

        // map initialization
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "OptimizeFullDetails");
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(oglContext, &mapListener);

        gem::LandmarkList lmks;
        gem::CoordinatesList coordsList;

        // parse input
        JsonParser parser(input);
        JsonValue object = parser.parse();

        auto addLandmarks = [&](JsonValue list, gem::Icon::Core icon) {
            for (auto elem : list.asArray()) {
                JsonArray loc = elem["location"].asArray();
                gem::Coordinates coords(loc[0].asNumber(), loc[1].asNumber(), 0.0);
                gem::Landmark lmk;
                lmk.setName(elem["alias"].asString());
                lmk.setCoordinates(coords);
                lmk.setImage(icon);
                lmks.push_back(lmk);
                coordsList.push_back(coords);
            }
            };

        // create landmarks for departures, orders and destinations
        addLandmarks(object["departures"], gem::Icon::Core::GreenBall);
        addLandmarks(object["orders"], gem::Icon::Core::BlueBall);
        addLandmarks(object["destinations"], gem::Icon::Core::RedBall);

        // display landmarks on the map
        mapView->activateHighlight(lmks, gem::HO_ShowLandmark | gem::HO_NoFading);
        gem::PolygonGeographicArea polyArea(coordsList);
        mapView->centerOnArea(polyArea, 55);
        int ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

        // create optimize request
        gem::vrp::Request request;
        ret = serv.optimize(&listener, input, request);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 60000);

        WAIT_UNTIL([&]() {
            serv.getRequest(&listener, request, request.id);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 7000);
            return request.status == gem::vrp::ERequestStatus::eFinished;
            }, 120000);

        gem::LargeInteger solutionId = request.entityId;
        // retrieve solution 
        std::shared_ptr<std::string> output = std::make_shared<std::string>();
        ret = serv.getSolutionJson(&listener, solutionId, output);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        // check if response was succesful and display routes on the map
        if (listener.IsFinished() && listener.GetError() == gem::KNoError && ret == gem::KNoError)
        {
            std::cout << "Problem optimized successfully" << std::endl;

            JsonParser parser2(*output);
            JsonValue object2 = parser2.parse();
            JsonArray routesList = object2["routes"].asArray();

            // Draw each route with different color 
            std::vector<gem::Rgba> baseColors = {
                gem::Rgba(255, 0, 0, 255),     // red
                gem::Rgba(0, 255, 0, 255),     // green
                gem::Rgba(0, 0, 255, 255),     // blue
                gem::Rgba(255, 255, 0, 255),   // yellow
                gem::Rgba(255, 0, 255, 255),   // magenta
                gem::Rgba(0, 255, 255, 255)    // cyan
            };

            gem::CoordinatesList allShapes;
            for (size_t i = 0; i < routesList.size(); ++i) {
                gem::CoordinatesList shape = Utils::decodePolyline(routesList[i]["shape"].asString());
                auto col = gem::MarkerCollection(gem::EMarkerType::MT_Polyline, "route" + std::to_string(i + 1));
                col.add(gem::Marker(shape));
                gem::MarkerCollectionRenderSettings settings;
                settings.polylineInnerColor = baseColors[i % baseColors.size()];
                mapView->preferences().markers().add(col, settings);

                allShapes.insert(allShapes.end(), shape.begin(), shape.end());
            }

            ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

            gem::PolygonGeographicArea routeArea(allShapes);
            mapView->centerOnArea(routeArea, 55);
            WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);
            WAIT_UNTIL_WINDOW_CLOSE();
        }
        else
        {
            std::cout << "Problem couldn't be optimized" << std::endl;
        }
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
