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

        gem::vrp::Optimization optimization;
        gem::LargeInteger optimizationId = -1; // set your optimization id  
        int res = serv.getOptimization(&listener, optimization, optimizationId);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        //update the order 2 from the optimization
        gem::vrp::Order order = optimization.getOrders().at(2);
        order.setNumberOfPackages(3);
        order.setServiceTime(360);

        res = optimization.updateOrder(&listener, order);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
            std::cout << "Order updated successfully" << std::endl;
        else
            std::cout << "Order couldn't be updated" << std::endl;
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
