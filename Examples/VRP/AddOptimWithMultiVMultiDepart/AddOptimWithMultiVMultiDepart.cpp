// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>
#include <API/GEM_MapView.h>
#include <API/GEM_Markers.h>

#include <iostream>
#include <iomanip>

void PrintRouteOnConsole(gem::vrp::Route route)
{
    int totalTime = (route.getOrders().at(route.getOrders().size() - 1).getArrivalTime().asInt() - route.getOrders().at(0).getArrivalTime().asInt()) / 1000;
    std::cout << route.getConfigurationParameters().getName().toStdString() << " totalDistance=" << std::setprecision(2) << std::fixed << route.getTotalDistance() << " totalTime=" << totalTime << " cost=" << route.getCost() << std::endl;
    for (gem::vrp::RouteOrder routeOrder : route.getOrders())
    {
        gem::String type[2] = { "PickUp","Delivery" };
        std::cout << "id=" << routeOrder.getId() << " index=" << routeOrder.getIndexInRoute() << " index in opt=" << routeOrder.getIndexInOptimization() << "customerId=" <<routeOrder.getCustomer().getId() << " alias=" << routeOrder.getAlias().toStdString() << std::setprecision(6) << " coordinates=[" << routeOrder.getCoordinates().getLatitude() << "," << routeOrder.getCoordinates().getLongitude() << "] type=" << type[(int)routeOrder.getType()].toStdString() << " time-window=[";

        if (routeOrder.getTimeWindow().first > 0)
            std::cout << routeOrder.getTimeWindow().first << ";" << routeOrder.getTimeWindow().second;
        else
            std::cout << "not set";

        std::cout << "] serviceTime=" << routeOrder.getServiceTime() << " arrivalTime=" << std::setfill('0') << std::setw(2) << routeOrder.getArrivalTime().getHour() << ":" << std::setw(2) << routeOrder.getArrivalTime().getMinute() << ":" << std::setw(2) << routeOrder.getArrivalTime().getSecond() << " " << std::setw(2) << routeOrder.getArrivalTime().getDay() << "/" << std::setw(2) << routeOrder.getArrivalTime().getMonth() << "/" << routeOrder.getArrivalTime().getYear()
            << " timeToNextOrder=" << routeOrder.getTimeToNextOrder() << " waitTime=" << routeOrder.getWaitTime() << " numberOfPiecesAtArrival=" << routeOrder.getNumberOfPackagesAtArrival() << " numberOfPiecesCollected=" << routeOrder.getCollectedNumberOfPackages() << " numberOfPiecesDelivered=" << routeOrder.getDeliveredNumberOfPackages() << " weightAtArrival=" << routeOrder.getWeightAtArrival() << " weightCollected=" << routeOrder.getCollectedWeight() << " weightDelivered=" << routeOrder.getDeliveredWeight()
            << " cubeArrival=" << routeOrder.getCubeAtArrival() << " cubeCollected=" << routeOrder.getCollectedCube() << " cubeDelivered=" << routeOrder.getDeliveredCube() << " traveledDistance=" << routeOrder.getTraveledDistance() << " distanceToNextOrder=" << routeOrder.getDistanceToNextOrder() << std::endl;

    }
    std::cout << std::endl;
}

void PrintRoutesOnConsole(gem::vrp::RouteList routes)
{
    for (gem::vrp::Route route : routes)
        PrintRouteOnConsole(route);
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
	if( value != nullptr )
		projectApiToken = value;
#endif

	// Sdk objects can be created & used below this line
	Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


    {
        ProgressListener listener;
        gem::vrp::Service serv;

        gem::vrp::Customer c0;
        c0.setCoordinates(gem::Coordinates(48.234270, -2.133208));
        c0.setAlias("c0");
        c0.setPhoneNumber("+12312312");
        c0.setEmail("c0@yahoo.com");
        int ret = serv.addCustomer(&listener, c0);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c1;
        c1.setCoordinates(gem::Coordinates(45.854137, 2.853998));
        c1.setAlias("c1");
        c1.setEmail("c1@yahoo.com");
        c1.setPhoneNumber("+12312312");
        ret = serv.addCustomer(&listener, c1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c2(gem::Coordinates(46.199373, 0.069986));
        c2.setAlias("c2");
        c2.setPhoneNumber("+12312312");
        c2.setEmail("c2@yahoo.com");
        ret = serv.addCustomer(&listener, c2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c3(gem::Coordinates(48.052503, 0.119726));
        c3.setAlias("c3");
        c3.setPhoneNumber("+12312312");
        c3.setEmail("c3@yahoo.com");
        ret = serv.addCustomer(&listener, c3);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c4(gem::Coordinates(44.346051, 4.694878));
        c4.setAlias("c4");
        c4.setPhoneNumber("+12312312");
        c4.setEmail("c4@yahoo.com");
        ret = serv.addCustomer(&listener, c4);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c5(gem::Coordinates(44.464582, 2.455020));
        c5.setAlias("c5");
        c5.setPhoneNumber("+12312312");
        c5.setEmail("c5@yahoo.com");
        ret = serv.addCustomer(&listener, c5);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c6(gem::Coordinates(48.656644, 5.907131));
        c6.setAlias("c6");
        c6.setPhoneNumber("+12312312");
        c6.setEmail("c6@yahoo.com");
        ret = serv.addCustomer(&listener, c6);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c7(gem::Coordinates(49.161539, 0.500580));
        c7.setAlias("c7");
        c7.setPhoneNumber("+12312312");
        c7.setEmail("c7@yahoo.com");
        ret = serv.addCustomer(&listener, c7);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c8(gem::Coordinates(47.702421, 3.384226));
        c8.setAlias("c8");
        c8.setPhoneNumber("+12312312");
        c8.setEmail("c8@yahoo.com");
        ret = serv.addCustomer(&listener, c8);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c9(gem::Coordinates(47.198274, 4.630011));
        c9.setAlias("c9");
        c9.setPhoneNumber("+12312312");
        c9.setEmail("c9@yahoo.com");
        ret = serv.addCustomer(&listener, c9);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c10(gem::Coordinates(49.655296, 2.243181));
        c10.setAlias("c10");
        c10.setPhoneNumber("+12312312");
        c10.setEmail("c10@yahoo.com");
        ret = serv.addCustomer(&listener, c10);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::Customer c11(gem::Coordinates(50.719729, 2.160877));
        c11.setAlias("c11");
        c11.setPhoneNumber("+12312312");
        c11.setEmail("c11@yahoo.com");
        ret = serv.addCustomer(&listener, c11);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::OrderList orders;
        gem::vrp::Order order0(c0);
        ret = serv.addOrder(&listener, order0, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order0);
        gem::vrp::Order order1(c1);
        ret = serv.addOrder(&listener, order1, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order1);
        gem::vrp::Order order2(c2);
        ret = serv.addOrder(&listener, order2, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order2);
        gem::vrp::Order order3(c3);
        ret = serv.addOrder(&listener, order3, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order3);
        gem::vrp::Order order4(c4);
        ret = serv.addOrder(&listener, order4, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order4);
        gem::vrp::Order order5(c5);
        ret = serv.addOrder(&listener, order5, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order5);
        gem::vrp::Order order6(c6);
        ret = serv.addOrder(&listener, order6, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order6);
        gem::vrp::Order order7(c7);
        ret = serv.addOrder(&listener, order7, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order7);
        gem::vrp::Order order8(c8);
        ret = serv.addOrder(&listener, order8, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order8);
        gem::vrp::Order order9(c9);
        ret = serv.addOrder(&listener, order9, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order9);
        gem::vrp::Order order10(c10);
        ret = serv.addOrder(&listener, order10, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order10);
        gem::vrp::Order order11(c11);
        ret = serv.addOrder(&listener, order11, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orders.push_back(order11);

        gem::vrp::VehicleList vehicles;
        gem::vrp::Vehicle vehicle1;
        vehicle1.setName("Vehicle 1");
        vehicle1.setType(gem::vrp::EVehicleType::VT_Car);
        vehicle1.setStatus(gem::vrp::EVehicleStatus::VS_Available);
        vehicle1.setManufacturer("Kia");
        vehicle1.setModel("Ceed");
        vehicle1.setFuelType(gem::vrp::EFuelType::FT_GasolinePremium);
        vehicle1.setConsumption(6.5);
        vehicle1.setLicensePlate("BV01ASD");
        vehicle1.setMaxWeight(350);
        vehicle1.setMaxCube(15);
        vehicle1.setStartTime(540); //9:00 AM
        vehicle1.setEndTime(1800);  //6:00 AM next day

        ret = serv.addVehicle(&listener, vehicle1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        vehicles.push_back(vehicle1);

        gem::vrp::Vehicle vehicle2;
        vehicle2.setName("Vehicle 2");
        vehicle2.setType(gem::vrp::EVehicleType::VT_Car);
        vehicle2.setStatus(gem::vrp::EVehicleStatus::VS_Available);
        vehicle2.setManufacturer("Kia");
        vehicle2.setModel("Ceed");
        vehicle2.setFuelType(gem::vrp::EFuelType::FT_GasolinePremium);
        vehicle2.setConsumption(6.5);
        vehicle2.setLicensePlate("BV02ASD");
        vehicle2.setMaxWeight(350);
        vehicle2.setMaxCube(15);
        vehicle2.setStartTime(540); //9:00 AM
        vehicle2.setEndTime(1800);  //6:00 AM next day

        ret = serv.addVehicle(&listener, vehicle2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        vehicles.push_back(vehicle2);

        gem::vrp::VehicleConstraintsList vehConstraintsList;
        gem::vrp::VehicleConstraints vehConstr1;
        vehConstr1.setStartDate(gem::Time(2020, 8, 7));
        vehConstr1.setMaxDistance(2000);
        vehConstraintsList.push_back(vehConstr1);

        gem::vrp::Departure departure1;
        departure1.setAlias("departure 1");
        departure1.setCoordinates(gem::Coordinates(48.618893, -1.353635));

        gem::vrp::Departure departure2;
        departure2.setAlias("departure 2");
        departure2.setCoordinates(gem::Coordinates(46.213984, 1.693113));

        gem::vrp::Optimization optimization;
        optimization.setOrders(orders);
        optimization.setVehicles(vehicles);
        optimization.setDepartures({ departure1,departure2 });
        optimization.setVehiclesConstraints(vehConstraintsList);

        //display orders on map
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "AddOptimizationWithMultipleDepartures");
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(oglContext, &mapListener);

        gem::LandmarkList lmks;
        gem::CoordinatesList coords;

        for (int i = 0; i < optimization.getDepartures().size(); i++)
        {
            gem::Landmark landmark;
            landmark.setName(optimization.getDepartures()[i].getAlias());
            landmark.setCoordinates(optimization.getDepartures()[i].getCoordinates());
            landmark.setImage(gem::Icon::Core::GreenBall);

            lmks.push_back(landmark);
            coords.push_back(optimization.getDepartures()[i].getCoordinates());
        }

        for (int i = 0; i < orders.size(); i++)
        {
            gem::Landmark landmark;
            landmark.setName(orders[i].getAlias());
            landmark.setCoordinates(orders[i].getCoordinates());
            landmark.setImage(gem::Icon::Core::BlueBall);

            lmks.push_back(landmark);
            coords.push_back(orders[i].getCoordinates());
        }

        for (int i = 0; i < optimization.getDestinations().size(); i++)
        {
            gem::Landmark landmark;
            landmark.setName(optimization.getDestinations()[i].getAlias());
            landmark.setCoordinates(optimization.getDestinations()[i].getCoordinates());
            landmark.setImage(gem::Icon::Core::RedBall);

            lmks.push_back(landmark);
            coords.push_back(optimization.getDestinations()[i].getCoordinates());
        }

        mapView->activateHighlight(lmks);
        gem::PolygonGeographicArea polyArea(coords);
        mapView->centerOnArea(polyArea);

        ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

        //add optimization
        gem::vrp::Request request;
        ret = serv.addOptimization(&listener, optimization, request);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        WAIT_UNTIL([&]() {
            serv.getRequest(&listener, request, request.id);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 7000);
            return request.status == gem::vrp::ERequestStatus::eFinished;
            }, 40000);

        gem::vrp::RouteList routes;
        ret = optimization.getSolution(&listener, routes);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && ret == gem::KNoError)
        {
            std::cout << "Problem optimized successfully" << std::endl;
            PrintRoutesOnConsole(routes);

            gem::CoordinatesList shape0 = routes[0].getShape();
            gem::CoordinatesList shape1 = routes[1].getShape();

            //display routes shapes on map
            auto col1 = gem::MarkerCollection(gem::EMarkerType::MT_Polyline, "shape0");
            col1.add(gem::Marker(shape0));

            mapView->preferences().markers().add(col1);

            auto col2 = gem::MarkerCollection(gem::EMarkerType::MT_Polyline, "shape1");
            col2.add(gem::Marker(shape1));
            gem::MarkerCollectionRenderSettings markerCollDisplaySettings;
            markerCollDisplaySettings.polylineInnerColor = gem::Rgba(0, 0, 255, 0);

            mapView->preferences().markers().add(col2, markerCollDisplaySettings);
            ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

            gem::CoordinatesList shapesCoordinates;
            shapesCoordinates.insert(shapesCoordinates.end(), shape0.begin(), shape0.end());
            shapesCoordinates.insert(shapesCoordinates.end(), shape1.begin(), shape1.end());

            gem::PolygonGeographicArea polyArea(shapesCoordinates);
            mapView->centerOnArea(polyArea);
            ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

            WAIT_UNTIL_WINDOW_CLOSE();
        }
        else
            std::cout << "Problem couldn't be optimized" << std::endl;
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
