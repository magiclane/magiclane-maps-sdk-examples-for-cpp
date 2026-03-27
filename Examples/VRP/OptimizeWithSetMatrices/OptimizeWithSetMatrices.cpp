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

    if (GEM_GET_API_ERROR() != gem::KNoError)
        return GEM_GET_API_ERROR();


    {
        ProgressListener listener;
        gem::vrp::Service serv;

        std::string input = R"(
          {
            "configurationParameters": {
              "name": "Paris - local delivery test",
              "ignoreTimeWindow": true,
              "allowDroppingOrders": false,
              "groupingOrders": false,
              "balancedRoutes": 0,
              "optimizationCriterion": 0,
              "arrangeCriterion": 0,
              "optimizationQuality": 2,
              "maxTimeToOptimize": 300,
              "maxWaitTime": 18000,
              "routeType": 0,
              "restrictions": 0,
              "distanceUnit": 0
            },
            "matrixBuildType": 0,
            "matrices": [
            {
            "vehicleType": 0,
            "distMatrix": [
              [0, 11847, 1289, 11234, 2156, 8934, 1678, 3456, 2789, 4123, 9876],
              [11847, 0, 12456, 4567, 9123, 2345, 11789, 8456, 7234, 8901, 5678],
              [1289, 12456, 0, 12345, 2567, 9456, 892, 4123, 3234, 4567, 10234],
              [11234, 4567, 12345, 0, 8456, 3789, 11456, 7234, 6789, 7456, 4890],
              [2156, 9123, 2567, 8456, 0, 6789, 2890, 1234, 1567, 2345, 7890],
              [8934, 2345, 9456, 3789, 6789, 0, 8567, 5678, 4567, 5890, 2456],
              [1678, 11789, 892, 11456, 2890, 8567, 0, 3789, 2456, 3567, 9567],
              [3456, 8456, 4123, 7234, 1234, 5678, 3789, 0, 2123, 1456, 6789],
              [2789, 7234, 3234, 6789, 1567, 4567, 2456, 2123, 0, 1789, 5234],
              [4123, 8901, 4567, 7456, 2345, 5890, 3567, 1456, 1789, 0, 6456],
              [9876, 5678, 10234, 4890, 7890, 2456, 9567, 6789, 5234, 6456, 0]
            ],
            "timeMatrix": [
              [0, 720, 89, 685, 165, 545, 125, 210, 170, 250, 600],
              [720, 0, 758, 278, 555, 143, 717, 515, 440, 542, 346],
              [89, 758, 0, 752, 156, 575, 65, 251, 197, 278, 623],
              [685, 278, 752, 0, 515, 231, 697, 441, 414, 454, 298],
              [165, 555, 156, 515, 0, 414, 176, 75, 95, 143, 481],
              [545, 143, 575, 231, 414, 0, 522, 346, 278, 359, 150],
              [125, 717, 65, 697, 176, 522, 0, 231, 150, 217, 582],
              [210, 515, 251, 441, 75, 346, 231, 0, 129, 89, 414],
              [170, 440, 197, 414, 95, 278, 150, 129, 0, 109, 319],
              [250, 542, 278, 454, 143, 359, 217, 89, 109, 0, 393],
              [600, 346, 623, 298, 481, 150, 582, 414, 319, 393, 0]
                ]
              }
            ],
            "orders": [
              {"alias": "Customer 1", "location": [48.88025, 2.299601], "type":0,"numberOfPackages":4,"weight":8.0,"cube":1.5,"timeWindow":[540,900],"serviceTime":5,"revenue":9,"priority":0},
              {"alias": "Customer 2", "location": [48.845198, 2.402896], "type":1,"numberOfPackages":5,"weight":9.4,"cube":1.2,"timeWindow":[600,960],"serviceTime":5,"revenue":7,"priority":0},
              {"alias": "Customer 3", "location": [48.897163, 2.292865], "type":0,"numberOfPackages":6,"weight":10.2,"cube":1.4,"timeWindow":[660,1020],"serviceTime":5,"revenue":6,"priority":0},
              {"alias": "Customer 4", "location": [48.90092, 2.400039], "type":1,"numberOfPackages":2,"weight":3.5,"cube":0.6,"timeWindow":[720,1080],"serviceTime":5,"revenue":5,"priority":0},
              {"alias": "Customer 5", "location": [48.860914, 2.310625], "type":0,"numberOfPackages":8,"weight":12.1,"cube":1.3,"timeWindow":[780,1140],"serviceTime":5,"revenue":8,"priority":0},
              {"alias": "Customer 6", "location": [48.827865, 2.379216], "type":1,"numberOfPackages":4,"weight":8.4,"cube":0.9,"timeWindow":[840,1200],"serviceTime":5,"revenue":7,"priority":0},
              {"alias": "Customer 7", "location": [48.887096, 2.283513], "type":1,"numberOfPackages":9,"weight":16.0,"cube":1.5,"timeWindow":[900,1260],"serviceTime":5,"revenue":9,"priority":0},
              {"alias": "Customer 8", "location": [48.896894, 2.321586], "type":0,"numberOfPackages":3,"weight":4.7,"cube":0.7,"timeWindow":[960,1320],"serviceTime":5,"revenue":4,"priority":0},
              {"alias": "Customer 9", "location": [48.870449, 2.342204], "type":0,"numberOfPackages":10,"weight":15.0,"cube":1.4,"timeWindow":[450,810],"serviceTime":5,"revenue":15,"priority":0},
              {"alias": "Customer 10", "location": [48.895658, 2.344043], "type":0,"numberOfPackages":4,"weight":7.8,"cube":1.0,"timeWindow":[510,870],"serviceTime":5,"revenue":8,"priority":0}
              ],
            "departures": [
              {"alias": "Depot 1", "location": [48.863247, 2.387459]}
             ],
            "destinations": [],
            "vehicles": [
              {
                "name": "Car vehicle 1",
                "type": 0,
                "maxLoadWeight": 60,
                "maxLoadCube": 50,
                "startTime": 420,
                "endTime": 1439,
                "licensePlate": "AA-123-AA",
                "consumption": 6.5
              }
            ],
            "vehiclesConstraints": [
              {"startDate":1596758400000,"maxNumberOfPackages":99999999,"minNumberOfOrders":0,"maxNumberOfOrders":99999999,"minDistance":0,"maxDistance":99999999,"maxRevenue":99999}
            ]
          }
          )";

        // map initialization
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "OptimizeSetMatrices");
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(oglContext, &mapListener);

        gem::LandmarkList lmks;
        gem::CoordinatesList coordsList;

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
        gem::PolygonGeographicArea area(coordsList);
        mapView->centerOnArea(area, 55);
        WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

        // create optimize request
        gem::vrp::Request request;
        int ret = serv.optimize(&listener, input, request);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 60000);

        gem::LargeInteger solutionId = request.entityId;

        WAIT_UNTIL([&]() {
            serv.getRequest(&listener, request, request.id);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 7000);
            return request.status == gem::vrp::ERequestStatus::eFinished;
            }, 120000);

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
        else {
            std::cout << "Optimization failed." << std::endl;
        }
    }

    return 0;
}

#if ( defined(_WIN32) || defined(_WIN64) ) && !defined(__MINGW32__) && !defined(__MINGW64__)

int WINAPI WinMain(HINSTANCE hInstance, // Instance
    HINSTANCE hPrevInstance, // Previous Instance
    LPSTR lpCmdLine, // Command Line Parameters
    int nCmdShow)
{
    main(0, nullptr);

    return 0;
}

#endif
