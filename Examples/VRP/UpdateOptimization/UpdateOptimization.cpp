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

        // change configuration parameters
        gem::vrp::ConfigurationParameters configParams = optimization.getConfigurationParameters();
        configParams.setName(configParams.getName() + " updated");
        configParams.setRouteType(gem::vrp::ERouteType::RT_EndAnywhere);
        configParams.setIgnoreTimeWindow(true);
        configParams.setDistanceUnit(gem::vrp::EDistanceUnit::DU_Miles);

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
        vehicle1.setMaxWeight(100);
        vehicle1.setMaxCube(2.1);

        res = serv.addVehicle(&listener, vehicle1);
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
        vehicle2.setMaxWeight(100);
        vehicle2.setMaxCube(2.1);

        res = serv.addVehicle(&listener, vehicle2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        vehicles.push_back(vehicle2);

        gem::vrp::Vehicle vehicle3;
        vehicle3.setName("Vehicle 3");
        vehicle3.setType(gem::vrp::EVehicleType::VT_Car);
        vehicle3.setStatus(gem::vrp::EVehicleStatus::VS_Available);
        vehicle3.setManufacturer("Kia");
        vehicle3.setModel("Ceed");
        vehicle3.setFuelType(gem::vrp::EFuelType::FT_GasolinePremium);
        vehicle3.setConsumption(6.5);
        vehicle3.setLicensePlate("BV03ASD");
        vehicle3.setMaxWeight(100);
        vehicle3.setMaxCube(2.1);

        res = serv.addVehicle(&listener, vehicle2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        vehicles.push_back(vehicle3);

        // change vehicle constraints
        gem::vrp::VehicleConstraintsList vehConstraintsList = optimization.getVehiclesConstraints();
        gem::vrp::VehicleConstraints vehConstr = vehConstraintsList.at(0);
        vehConstr.setMaxNumberOfPackages(80);
        vehConstr.setMinNumberOfOrders(1);
        vehConstr.setMaxDistance(932); // 932 miles
        auto beginIt = vehConstraintsList.begin();
        vehConstraintsList.erase(beginIt, beginIt + 1);
        vehConstraintsList.insert(vehConstraintsList.begin(), vehConstr);

        gem::vrp::Departure departure;       
        departure.setAlias("Depot");
        departure.setCoordinates(gem::Coordinates(48.234270, -2.133208));
        
        optimization.setConfigurationParameters(configParams);
        optimization.setVehiclesConstraints(vehConstraintsList);
        optimization.setVehicles(vehicles);
        optimization.setDepartures({departure});

        gem::vrp::Request request;
        res = serv.updateOptimization(&listener, optimization, request);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);
        
        WAIT_UNTIL([&]() {
            serv.getRequest(&listener, request, request.id);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 7000);
            return request.status == gem::vrp::ERequestStatus::eFinished;
            }, 40000);

        gem::vrp::RouteList routes;
        res = optimization.getSolution(&listener, routes);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
        {
            std::cout << "Optimization updated successfully" << std::endl;
            PrintRoutesOnConsole(routes);
        }
        else
            std::cout << "Optimization couldn't be updated" << std::endl;
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
