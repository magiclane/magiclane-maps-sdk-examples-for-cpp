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

        gem::vrp::ConfigurationParameters setMatricesconfigParams;
        setMatricesconfigParams.setName("Optimization with set matrices");
        setMatricesconfigParams.setDistanceUnit(gem::vrp::EDistanceUnit::DU_Kilometers);
        
        gem::AddressInfo address;
        address.setField("France", gem::EAddressField::Country);
        address.setField("Brittany", gem::EAddressField::County);
        address.setField("Quédillac", gem::EAddressField::City);
        address.setField("35290", gem::EAddressField::PostalCode);
        address.setField("34", gem::EAddressField::StreetNumber);

        gem::vrp::Departure departure;
        departure.setAlias("Depot");
        departure.setAddress(address);
        departure.setCoordinates(gem::Coordinates(48.234370, -2.133108));

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
        vehicle1.setMaxWeight(35);
        vehicle1.setMaxCube(17);

        ret = serv.addVehicle(&listener, vehicle1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        vehicles.push_back(vehicle1);

        gem::vrp::VehicleConstraintsList vehConstraintsList;
        gem::vrp::VehicleConstraints vehConstr1;
        vehConstraintsList.push_back(vehConstr1);

        gem::vrp::Optimization optimization;
        optimization.setConfigurationParameters(setMatricesconfigParams);
        optimization.setOrders(orders);
        optimization.setDepartures({ departure });
        optimization.setVehicles(vehicles);
        optimization.setVehiclesConstraints(vehConstraintsList);
        optimization.setMatrixBuildType(gem::vrp::EMatrixBuildType::MBT_Set);
        optimization.setDistanceMatrices({ std::make_pair(gem::vrp::EVehicleType::VT_Car, gem::FloatListList{
            gem::FloatList{0,634.235,318.656,319.651,823.757,627.028,522.547,106.736,421.984,628.886,135.706,326.935,0},
            gem::FloatList{634.134,0,299.347,386.994,285.027,96.153,630.996,710.125,310.680,502.910,506.150,486.792,634.134},
            gem::FloatList{319.372,300.753,0,300.342,655.031,362.724,590.943,395.363,372.643,684.322,363.187,416.969,319.372},
            gem::FloatList{320.489,387.311,300.083,0,576.833,468.863,327.913,263.726,114.088,384.249,145.352,133.151,320.489},
            gem::FloatList{824.225,285.128,653.950,577.085,0,317.686,516.319,811.555,422.937,368.664,686.270,558.408,824.225},
            gem::FloatList{626.777,96.192,362.235,476.168,317.730,0,695.144,702.768,392.937,567.057,583.754,569.049,626.777},
            gem::FloatList{520.626,627.820,587.293,323.888,513.007,691.336,0,473.096,255.427,250.207,350.315,204.017,520.626},
            gem::FloatList{107.436,630.674,395.807,269.942,813.151,704.180,472.185,0,421.212,605.721,125.103,285.562,107.436},
            gem::FloatList{421.709,308.713,373.153,114.373,422.398,390.265,256.757,418.882,0,298.949,269.198,191.736,421.709},
            gem::FloatList{649.152,502.833,685.279,384.427,369.072,566.349,252.240,604.125,300.146,0,478.840,350.978,649.152},
            gem::FloatList{135.384,506.064,363.728,145.332,685.173,582.869,353.136,123.908,269.691,477.743,0,157.524,135.384},
            gem::FloatList{328.453,484.583,417.938,133.002,559.142,566.135,208.007,283.426,192.058,351.712,158.141,0,328.453},
            gem::FloatList{0,634.235,318.656,319.651,823.757,627.028,522.547,106.736,421.984,628.886,135.706,326.935,0} }) });     //distances from departure to the rest of the orders

        optimization.setTimeMatrices({ std::make_pair(gem::vrp::EVehicleType::VT_Car, gem::IntListList{
            gem::IntList{0,22449,11482,11980,27077,24843,18044,4154,15221,21387,6265,11558,0},
            gem::IntList{22483,0,13738,15955,15819,5678,23333,25053,13868,19527,19018,20298,22483},
            gem::IntList{11590,13817,0,11975,24610,15920,20788,14159,14747,23170,12669,14790,11590},
            gem::IntList{12088,15916,11907,0,20544,19446,13021,13153,6378,15191,7104,6954,12088},
            gem::IntList{27121,15808,24582,20594,0,12955,17860,26904,16924,13381,22591,18822,27121},
            gem::IntList{24730,5693,15852,19256,12854,0,26059,27300,17450,22252,22274,23880,24730},
            gem::IntList{18153,23389,20841,13009,17996,26059,0,17057,13308,9758,12885,8490,18153},
            gem::IntList{4160,25033,14094,13146,27028,27455,16942,0,15112,20561,6393,10746,4160},
            gem::IntList{15350,13809,14704,6409,16903,17338,13315,15100,0,12512,10333,8338,15350},
            gem::IntList{21511,19528,23263,15236,13426,22198,9652,20556,12548,0,16243,12474,21511},
            gem::IntList{6244,18958,12659,7071,22611,22423,12792,6352,10264,16145,0,6306,6244},
            gem::IntList{11675,20157,14897,6912,18984,23686,8504,10719,8306,12518,6407,0,11675},
            gem::IntList{0,22449,11482,11980,27077,24843,18044,4154,15221,21387,6265,11558,0} }) });

        //display orders on map
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "AddOptimizationWithSetMatrices");
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(oglContext, &mapListener);

        gem::LandmarkList lmks;
        gem::CoordinatesList coords;

        for (int i = 0; i < orders.size(); i++)
        {
            gem::Landmark landmark;
            landmark.setName(orders[i].getAlias());
            landmark.setCoordinates(orders[i].getCoordinates());
            landmark.setImage(gem::Icon::Core::BlueBall);
            if (i == 0)
                landmark.setImage(gem::Icon::Core::GreenBall);

            lmks.push_back(landmark);
            coords.push_back(orders[i].getCoordinates());
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

            gem::CoordinatesList shape = routes[0].getShape();

            //display routes shapes on map
            auto col = gem::MarkerCollection(gem::EMarkerType::MT_Polyline, "shape");
            col.add(gem::Marker(shape));
            mapView->preferences().markers().add(col);
            ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

            gem::PolygonGeographicArea polyArea(shape);
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
