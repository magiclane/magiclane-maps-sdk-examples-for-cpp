// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>

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

        //get 2 customers. if you don't have any customers added, first add them (see AddCustomer example)
        gem::vrp::Customer customer1;
        gem::LargeInteger customerId1 = -1;      // set your customer id  
        int res = serv.getCustomer(&listener, customer1, customerId1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        gem::vrp::Customer customer2;
        gem::LargeInteger customerId2 = -1;      // set your customer id  
        res = serv.getCustomer(&listener, customer2, customerId2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //create one order for each customer
        gem::vrp::RouteOrderList ordersToAdd;
        gem::vrp::RouteOrder orderToAdd1(customer1);    //the order will have the name, coordinates, address and phone number of the customer
        orderToAdd1.setCoordinates(gem::Coordinates(48.324664, -0.497207)); //now the coordinates set from the customer will be replaced with these ones
        orderToAdd1.setNumberOfPackages(5);
        orderToAdd1.setServiceTime(600);
        orderToAdd1.setType(gem::vrp::EOrderType::OT_PickUp);
        res = serv.addOrder(&listener, orderToAdd1, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        ordersToAdd.push_back(orderToAdd1);
        gem::vrp::RouteOrder orderToAdd2(customer2);
        orderToAdd2.setCoordinates(gem::Coordinates(48.692307, 3.461280));
        orderToAdd2.setNumberOfPackages(4);
        orderToAdd2.setServiceTime(480);
        orderToAdd2.setType(gem::vrp::EOrderType::OT_Delivery);
        res = serv.addOrder(&listener, orderToAdd1, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        ordersToAdd.push_back(orderToAdd2);

        //get the route to which you want to add the orders
        gem::vrp::Route route;
        gem::LargeInteger routeId = -1;      // set your route id  
        res = serv.getRoute(&listener, route, routeId);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        //add the orders
        bool reoptimize = false;             // if this is true, all the route's orders (including the newly added ones) will be rearranged
        bool optimzalPosition = false;       // because this is false and the orders don't have the indexInRoute set, the orders will be added at the end of the route
        gem::vrp::Request request;
        res = route.addOrders(&listener, ordersToAdd, optimzalPosition, request, reoptimize);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        WAIT_UNTIL([&]() {
            serv.getRequest(&listener, request, request.id);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 7000);
            return request.status == gem::vrp::ERequestStatus::eFinished;
            }, 40000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
        {
            std::cout << "Orders added successfully" << std::endl;
            PrintRouteOnConsole(route);
        }
        else
            std::cout << "Orders couldn't be added" << std::endl;
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
