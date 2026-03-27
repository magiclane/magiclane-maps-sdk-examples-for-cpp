// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>

#include <iostream>

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

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


    {
        ProgressListener listener;
        gem::vrp::Service serv;

        //initialize customers
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

        //initialize orders
        gem::vrp::OrderList orders1, orders2;
        gem::vrp::Order order0(c0);
        order0.setNumberOfPackages(5);
        order0.setWeight(15.7);
        order0.setCube(0.2);
        order0.setServiceTime(600);
        order0.setTimeWindow(std::make_pair(420, 1860)); //  7:00:00 AM - next day 7:00:00 AM
        order0.setType(gem::vrp::EOrderType::OT_PickUp);
        orders1.push_back(order0);

        gem::vrp::Order order1(c1);
        order1.setNumberOfPackages(4);
        order1.setWeight(15.5);
        order1.setCube(0.9);
        order1.setTimeWindow(std::make_pair(420, 1860)); //  7:00:00 AM - next day 7:00:00 AM
        order1.setType(gem::vrp::EOrderType::OT_PickUp);
        orders1.push_back(order1);

        gem::vrp::Order order2(c2);
        order2.setNumberOfPackages(8);
        order2.setWeight(5.5);
        order2.setCube(0.3);
        order2.setServiceTime(600);
        order2.setTimeWindow(std::make_pair(660, 1353)); //  11:00:00 AM - 10:33:20 PM
        order2.setType(gem::vrp::EOrderType::OT_Delivery);
        orders1.push_back(order2);

        gem::vrp::Order order3(c3);
        order3.setTimeWindow(std::make_pair(753, 1860)); //  12:33:20 PM - next day 7:00:00 AM
        order3.setType(gem::vrp::EOrderType::OT_Delivery);
        orders1.push_back(order3);

        gem::vrp::Order order4(c4);
        order4.setNumberOfPackages(8);
        order4.setWeight(5.1);
        order4.setCube(0.2);
        order4.setServiceTime(600);
        order4.setTimeWindow(std::make_pair(1086, 1791)); //  6:06:40 PM - next day 5:51:40 AM
        order4.setType(gem::vrp::EOrderType::OT_PickUp);
        orders1.push_back(order4);

        gem::vrp::Order order5(c5);
        order5.setNumberOfPackages(11);
        order5.setWeight(6.5);
        order5.setCube(0.1);
        order5.setServiceTime(900);
        order5.setTimeWindow(std::make_pair(1053, 1860)); //  5:33:20 PM -  next day 7:00:00 AM
        order5.setRevenue(25);
        order5.setType(gem::vrp::EOrderType::OT_Delivery);
        orders1.push_back(order5);

        gem::vrp::Order order6(c6);
        order6.setNumberOfPackages(4);
        order6.setWeight(1.5);
        order6.setCube(0.5);
        order6.setServiceTime(500);
        order6.setTimeWindow(std::make_pair(841, 1686)); //  2:01:40 PM - next day 4:06:40 AM
        order6.setType(gem::vrp::EOrderType::OT_PickUp);
        orders1.push_back(order6);

        gem::vrp::Order order7(c7);
        order7.setNumberOfPackages(12);
        order7.setWeight(6.1);
        order7.setCube(0.4);
        order7.setServiceTime(750);
        order7.setTimeWindow(std::make_pair(1086, 1718)); //  6:06:40 PM - next day 4:38:20 AM
        order7.setRevenue(75);
        order7.setType(gem::vrp::EOrderType::OT_Delivery);
        orders1.push_back(order7);

        gem::vrp::Order order8(c8);
        order8.setNumberOfPackages(7);
        order8.setWeight(2.5);
        order8.setCube(0.3);
        order8.setServiceTime(800);
        order8.setTimeWindow(std::make_pair(770, 1140)); //  12:50:00 PM - 7:00:00 PM
        order8.setType(gem::vrp::EOrderType::OT_Delivery);
        order8.setRevenue(110);
        orders1.push_back(order8);

        gem::vrp::Order order9(c9);
        order9.setNumberOfPackages(12);
        order9.setWeight(0.7);
        order9.setCube(0.5);
        order9.setServiceTime(1000);
        order9.setTimeWindow(std::make_pair(836, 1408)); //  1:56:40 PM - 11:28:20 PM
        order9.setType(gem::vrp::EOrderType::OT_PickUp);
        orders1.push_back(order9);

        gem::vrp::Order order10(c10);
        order10.setNumberOfPackages(9);
        order10.setWeight(4.3);
        order10.setCube(0.6);
        order10.setServiceTime(850);
        order10.setTimeWindow(std::make_pair(903, 1520)); //  3:03:20 PM - next day 1:20:00 AM
        order10.setType(gem::vrp::EOrderType::OT_PickUp);
        orders1.push_back(order10);

        gem::vrp::Order order11(c11);
        order11.setNumberOfPackages(5);
        order11.setWeight(4.1);
        order11.setCube(0.4);
        order11.setServiceTime(600);
        order11.setTimeWindow(std::make_pair(703, 1203)); //  11:43:20 AM - 8:03:20 PM
        order11.setType(gem::vrp::EOrderType::OT_PickUp);
        orders1.push_back(order11);
        
        //make a copy of the order list that will be used to create the second optimization
        orders2 = orders1;
        
        //add orders list for first optimization
        for (int i = 0; i < orders2.size(); i++)
        {
            ret = serv.addOrder(&listener, orders1.at_nc(i), false);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        }

        //add orders list for second optimization
        for (int i = 0; i < orders2.size(); i++)
        {
            ret = serv.addOrder(&listener, orders2.at_nc(i), false);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        }

        gem::vrp::OrdersSequenceMap ordersSequence;
        gem::LargeIntListList fixedSequence = { { orders1[2].getId(), orders1[8].getId(), orders1[6].getId()} };
        ordersSequence.insert(std::make_pair(gem::vrp::EOrdersSequenceOption::OSO_InFixedSequence, fixedSequence));

        //initialize configurations
        gem::vrp::ConfigurationParameters configParams1;
        configParams1.setName("France optimization1");
        configParams1.setIgnoreTimeWindow(false);
        configParams1.setOptimizationCriterion(gem::vrp::EOptimizationCriterion::OC_Distance);
        configParams1.setOptimizationQuality(gem::vrp::EOptimizationQuality::OQ_Optimized);
        configParams1.setMaxWaitTime(18000); // A vehicle can wait maximum 5 hours between a order and the next one, in order to visit the next one within its time window
        configParams1.setRouteType(gem::vrp::ERouteType::RT_CustomEnd);
        configParams1.setRestrictions(gem::vrp::ERoadRestrictions::RR_None);
        configParams1.setDistanceUnit(gem::vrp::EDistanceUnit::DU_Kilometers);
        configParams1.setOrderSequenceOptions(ordersSequence);

        gem::vrp::OrdersSequenceMap ordersSequence2;
        gem::LargeIntListList fixedSequence2 = { { orders2[2].getId(), orders2[8].getId(), orders2[6].getId()} };
        ordersSequence2.insert(std::make_pair(gem::vrp::EOrdersSequenceOption::OSO_InFixedSequence, fixedSequence2));

        gem::vrp::ConfigurationParameters configParams2;
        configParams2.setName("France optimization2");
        configParams2.setIgnoreTimeWindow(false);
        configParams2.setOptimizationCriterion(gem::vrp::EOptimizationCriterion::OC_Distance);
        configParams2.setOptimizationQuality(gem::vrp::EOptimizationQuality::OQ_Optimized);
        configParams2.setMaxWaitTime(18000); // A vehicle can wait maximum 5 hours between a order and the next one, in order to visit the next one within its time window
        configParams2.setRouteType(gem::vrp::ERouteType::RT_CustomEnd);
        configParams2.setRestrictions(gem::vrp::ERoadRestrictions::RR_None);
        configParams2.setDistanceUnit(gem::vrp::EDistanceUnit::DU_Kilometers);
        configParams2.setOrderSequenceOptions(ordersSequence2);

        //initialize first vehicle
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
        vehicle1.setMaxWeight(300);
        vehicle1.setMaxCube(15);
        vehicle1.setStartTime(420); //7:00:00 AM in minutes
        vehicle1.setEndTime(2580);   //next day 7:00:00 PM in minutes

        //add first vehicle
        int res = serv.addVehicle(&listener, vehicle1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        vehicles.push_back(vehicle1);

        //initialize second vehicle
        gem::vrp::Vehicle vehicle2;
        vehicle2.setName("Vehicle 2");
        vehicle2.setType(gem::vrp::EVehicleType::VT_Car);
        vehicle2.setStatus(gem::vrp::EVehicleStatus::VS_Available);
        vehicle2.setManufacturer("Kia");
        vehicle2.setModel("Ceed");
        vehicle2.setFuelType(gem::vrp::EFuelType::FT_GasolinePremium);
        vehicle2.setConsumption(6.5);
        vehicle2.setLicensePlate("BV02ASD");
        vehicle2.setMaxWeight(300);
        vehicle2.setMaxCube(15);
        vehicle2.setStartTime(480); //8:00:00 AM in minutes
        vehicle2.setEndTime(2520);   //next day 6:00:00 PM in minutes

        //add second vehicle
        res = serv.addVehicle(&listener, vehicle2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        vehicles.push_back(vehicle2);

        //initialize vehicle constraints
        gem::vrp::VehicleConstraintsList vehConstraintsList;
        gem::vrp::VehicleConstraints vehConstr1;
        vehConstr1.setMaxNumberOfPackages(100);
        vehConstr1.setMaxRevenue(2000);
        vehConstr1.setStartDate(gem::Time(2020, 8, 7));
        vehConstr1.setMinNumberOfOrders(1);
        vehConstr1.setMaxNumberOfOrders(50);
        vehConstr1.setMinDistance(1);
        vehConstr1.setMaxDistance(19000);
        vehConstraintsList.push_back(vehConstr1);
        gem::vrp::VehicleConstraints vehConstr2;
        vehConstr2.setMaxNumberOfPackages(100);
        vehConstr2.setMaxRevenue(2000);
        vehConstr2.setStartDate(gem::Time(2020, 8, 7));
        vehConstr2.setMinNumberOfOrders(2);
        vehConstr2.setMaxNumberOfOrders(60);
        vehConstr2.setMinDistance(2);
        vehConstr2.setMaxDistance(20000);
        vehConstraintsList.push_back(vehConstr2);

        //initialize departures
        gem::vrp::Departure departure1;
        departure1.setAlias("Depot 1");
        departure1.setCoordinates(gem::Coordinates(48.618893, -1.353635));
        gem::vrp::Departure departure2;
        departure2.setAlias("Depot 2");
        departure2.setCoordinates(gem::Coordinates(46.213984, 1.693113));

        gem::vrp::Destination destination;
        destination.setAlias("Destination");
        destination.setCoordinates(gem::Coordinates(47.617484, 1.152466));

        //initialize first optimization
        gem::vrp::Optimization optimization1;
        optimization1.setConfigurationParameters(configParams1);
        optimization1.setVehicles(vehicles);
        optimization1.setDepartures({ departure1,departure2 });
        optimization1.setDestinations({ destination }); // both vehicles will end their routes at the same destination
        optimization1.setOrders(orders1);
        optimization1.setVehiclesConstraints(vehConstraintsList);
        optimization1.setMatrixBuildType(gem::vrp::EMatrixBuildType::MBT_Real);

        //add first optimization
        gem::vrp::Request request1;
        ret = serv.addOptimization(&listener, optimization1, request1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        WAIT_UNTIL([&]() {
            serv.getRequest(&listener, request1, request1.id);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 7000);
            return request1.status == gem::vrp::ERequestStatus::eFinished;
            }, 40000);

        gem::vrp::RouteList routes1;
        ret = optimization1.getSolution(&listener, routes1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);
    
        //initialize second optimization
        gem::vrp::Optimization optimization2;
        optimization2.setConfigurationParameters(configParams2);
        optimization2.setVehicles(vehicles);
        optimization2.setDepartures({ departure1,departure2 });
        optimization2.setDestinations({ destination }); // both vehicles will end their routes at the same destination
        optimization2.setOrders(orders2);
        optimization2.setVehiclesConstraints(vehConstraintsList);
        optimization2.setMatrixBuildType(gem::vrp::EMatrixBuildType::MBT_Real);
        
        //add second optimization
        gem::vrp::Request request2;
        ret = serv.addOptimization(&listener, optimization2, request2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        WAIT_UNTIL([&]() {
            serv.getRequest(&listener, request2, request2.id);
            WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 7000);
            return request2.status == gem::vrp::ERequestStatus::eFinished;
            }, 40000);

        gem::vrp::RouteList routes2;
        ret = optimization1.getSolution(&listener, routes2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);
   
        //get all optimizations from database that contains string "optimization1"
        gem::vrp::OptimizationList allOptimizations;
        res = serv.getOptimizations(&listener, allOptimizations, "optimization1");
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
            std::cout << allOptimizations.size() << " optimizations returned successfully" << std::endl;
        else
            std::cout << "No optimization returned" << std::endl;
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
