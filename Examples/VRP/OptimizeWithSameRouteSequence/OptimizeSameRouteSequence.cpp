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
        "name": "Paris - 30 orders optimization",
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
        "orderSequenceOptions":[
                    {
                    "sequenceOptions": 0,
                    "ordersSequence": [[14, 16, 6, 29]]
                    }
               ]
      },
      "orders": [
        {"alias": "Customer 1", "location": [48.870852, 2.356148], "type":0,"numberOfPackages":2,"weight":5.0,"cube":0.8,"timeWindow":[420,720],"serviceTime":5,"revenue":10,"priority":0},
        {"alias": "Customer 2", "location": [48.827327, 2.342267], "type":0,"numberOfPackages":3,"weight":6.0,"cube":1.1,"timeWindow":[480,840],"serviceTime":5,"revenue":12,"priority":0},
        {"alias": "Customer 3", "location": [48.88025, 2.299601], "type":0,"numberOfPackages":4,"weight":8.0,"cube":1.5,"timeWindow":[540,900],"serviceTime":5,"revenue":9,"priority":0},
        {"alias": "Customer 4", "location": [48.845198, 2.402896], "type":1,"numberOfPackages":5,"weight":9.4,"cube":1.2,"timeWindow":[600,960],"serviceTime":5,"revenue":7,"priority":0},
        {"alias": "Customer 5", "location": [48.897163, 2.292865], "type":0,"numberOfPackages":6,"weight":10.2,"cube":1.4,"timeWindow":[660,1020],"serviceTime":5,"revenue":6,"priority":0},
        {"alias": "Customer 6", "location": [48.90092, 2.400039], "type":1,"numberOfPackages":2,"weight":3.5,"cube":0.6,"timeWindow":[720,1080],"serviceTime":5,"revenue":5,"priority":0},
        {"alias": "Customer 7", "location": [48.860914, 2.310625], "type":0,"numberOfPackages":8,"weight":12.1,"cube":1.3,"timeWindow":[780,1140],"serviceTime":5,"revenue":8,"priority":0},
        {"alias": "Customer 8", "location": [48.827865, 2.379216], "type":1,"numberOfPackages":4,"weight":8.4,"cube":0.9,"timeWindow":[840,1200],"serviceTime":5,"revenue":7,"priority":0},
        {"alias": "Customer 9", "location": [48.887096, 2.283513], "type":1,"numberOfPackages":9,"weight":16.0,"cube":1.5,"timeWindow":[900,1260],"serviceTime":5,"revenue":9,"priority":0},
        {"alias": "Customer 10", "location": [48.896894, 2.321586], "type":0,"numberOfPackages":3,"weight":4.7,"cube":0.7,"timeWindow":[960,1320],"serviceTime":5,"revenue":4,"priority":0},
        {"alias": "Customer 11", "location": [48.870449, 2.342204], "type":0,"numberOfPackages":10,"weight":15.0,"cube":1.4,"timeWindow":[450,810],"serviceTime":5,"revenue":15,"priority":0},
        {"alias": "Customer 12", "location": [48.895658, 2.344043], "type":0,"numberOfPackages":4,"weight":7.8,"cube":1.0,"timeWindow":[510,870],"serviceTime":5,"revenue":8,"priority":0},
        {"alias": "Customer 13", "location": [48.882213, 2.331512], "type":0,"numberOfPackages":5,"weight":9.2,"cube":1.1,"timeWindow":[570,930],"serviceTime":5,"revenue":11,"priority":0},
        {"alias": "Customer 14", "location": [48.854327, 2.362874], "type":1,"numberOfPackages":7,"weight":13.1,"cube":1.3,"timeWindow":[630,990],"serviceTime":5,"revenue":10,"priority":0},
        {"alias": "Customer 15", "location": [48.881954, 2.293521], "type":1,"numberOfPackages":6,"weight":11.0,"cube":1.2,"timeWindow":[690,1050],"serviceTime":5,"revenue":9,"priority":0},
        {"alias": "Customer 16", "location": [48.889213, 2.312349], "type":0,"numberOfPackages":4,"weight":6.7,"cube":0.8,"timeWindow":[750,1110],"serviceTime":5,"revenue":6,"priority":0},
        {"alias": "Customer 17", "location": [48.874211, 2.303412], "type":1,"numberOfPackages":8,"weight":13.5,"cube":1.4,"timeWindow":[810,1170],"serviceTime":5,"revenue":7,"priority":0},
        {"alias": "Customer 18", "location": [48.859217, 2.372316], "type":0,"numberOfPackages":5,"weight":7.2,"cube":0.9,"timeWindow":[870,1230],"serviceTime":5,"revenue":8,"priority":0},
        {"alias": "Customer 19", "location": [48.868492, 2.343152], "type":0,"numberOfPackages":3,"weight":4.3,"cube":0.6,"timeWindow":[930,1290],"serviceTime":5,"revenue":4,"priority":0},
        {"alias": "Customer 20", "location": [48.872156, 2.326891], "type":1,"numberOfPackages":2,"weight":3.1,"cube":0.5,"timeWindow":[990,1350],"serviceTime":5,"revenue":3,"priority":0},
        {"alias": "Customer 21", "location": [48.885112, 2.341722], "type":0,"numberOfPackages":7,"weight":11.4,"cube":1.3,"timeWindow":[420,780],"serviceTime":5,"revenue":10,"priority":0},
        {"alias": "Customer 22", "location": [48.852317, 2.380911], "type":1,"numberOfPackages":9,"weight":15.2,"cube":1.5,"timeWindow":[480,840],"serviceTime":5,"revenue":11,"priority":0},
        {"alias": "Customer 23", "location": [48.877951, 2.352652], "type":0,"numberOfPackages":6,"weight":10.8,"cube":1.2,"timeWindow":[540,900],"serviceTime":5,"revenue":8,"priority":0},
        {"alias": "Customer 24", "location": [48.899223, 2.372101], "type":1,"numberOfPackages":3,"weight":5.3,"cube":0.7,"timeWindow":[600,960],"serviceTime":5,"revenue":6,"priority":0},
        {"alias": "Customer 25", "location": [48.861715, 2.334811], "type":0,"numberOfPackages":5,"weight":9.2,"cube":1.0,"timeWindow":[660,1020],"serviceTime":5,"revenue":9,"priority":0},
        {"alias": "Customer 26", "location": [48.866712, 2.394414], "type":1,"numberOfPackages":8,"weight":13.7,"cube":1.4,"timeWindow":[720,1080],"serviceTime":5,"revenue":10,"priority":0},
        {"alias": "Customer 27", "location": [48.879314, 2.361921], "type":0,"numberOfPackages":2,"weight":4.1,"cube":0.5,"timeWindow":[780,1140],"serviceTime":5,"revenue":5,"priority":0},
        {"alias": "Customer 28", "location": [48.884213, 2.321234], "type":1,"numberOfPackages":3,"weight":6.0,"cube":0.6,"timeWindow":[840,1200],"serviceTime":5,"revenue":6,"priority":0},
        {"alias": "Customer 29", "location": [48.891213, 2.352116], "type":0,"numberOfPackages":9,"weight":15.5,"cube":1.5,"timeWindow":[900,1260],"serviceTime":5,"revenue":12,"priority":0},
        {"alias": "Customer 30", "location": [48.853112, 2.341214], "type":1,"numberOfPackages":7,"weight":12.0,"cube":1.3,"timeWindow":[960,1320],"serviceTime":5,"revenue":9,"priority":0}
      ],
      "departures": [
        {"alias": "Depot 1", "location": [48.875432, 2.361789]},
        {"alias": "Depot 2", "location": [48.831256, 2.347823]}
      ],
      "destinations": [
        {"alias": "Destination 1", "location": [48.895687, 2.365432]},
        {"alias": "Destination 2", "location": [48.825789, 2.335147]}
      ],
      "vehicles": [
        {"name": "Car vehicle 1", "type":0,"maxLoadWeight":150,"maxLoadCube":50,"startTime":420,"endTime":1439,"licensePlate":"AA-123-AA","consumption":6.5},
        {"name": "Car vehicle 2", "type":0,"maxLoadWeight":150,"maxLoadCube":50,"startTime":420,"endTime":1439,"licensePlate":"AA-124-AA","consumption":6.5}
      ],
      "vehiclesConstraints": [
        {"startDate":1596758400000,"maxNumberOfPackages":80,"minNumberOfOrders":0,"maxNumberOfOrders":99999999,"minDistance":0,"maxDistance":99999999,"maxRevenue":9999}
      ],
      "matrixBuildType": 1
    }
    )";

        // map initialization
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "OptimizeWithSameRouteSequence");
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
