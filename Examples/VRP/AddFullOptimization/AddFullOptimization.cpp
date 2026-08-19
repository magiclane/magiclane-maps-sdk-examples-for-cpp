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

namespace
{
    void PrintRouteOnConsole( gem::vrp::Route route )
    {
        int totalTime = static_cast<int>( ( route.getOrders().at( route.getOrders().size() - 1 ).getArrivalTime().asInt() - route.getOrders().at( 0 ).getArrivalTime().asInt() ) /
                                          1000 );
        std::cout << route.getConfigurationParameters().getName().toStdString() << " totalDistance=" << std::setprecision( 2 ) << std::fixed << route.getTotalDistance()
                  << " totalTime=" << totalTime << " cost=" << route.getCost() << std::endl;
        for( gem::vrp::RouteOrder routeOrder : route.getOrders() )
        {
            gem::String type[2] = { "PickUp", "Delivery" };
            std::cout << "id=" << routeOrder.getId() << " index=" << routeOrder.getIndexInRoute() << " index in opt=" << routeOrder.getIndexInOptimization()
                      << "customerId=" << routeOrder.getCustomer().getId() << " alias=" << routeOrder.getAlias().toStdString() << std::setprecision( 6 ) << " coordinates=["
                      << routeOrder.getCoordinates().getLatitude() << "," << routeOrder.getCoordinates().getLongitude()
                      << "] type=" << type[( int ) routeOrder.getType()].toStdString() << " time-window=[";

            if( routeOrder.getTimeWindow().first > 0 )
                std::cout << routeOrder.getTimeWindow().first << ";" << routeOrder.getTimeWindow().second;
            else
                std::cout << "not set";

            std::cout << "] serviceTime=" << routeOrder.getServiceTime() << " arrivalTime=" << std::setfill( '0' ) << std::setw( 2 ) << routeOrder.getArrivalTime().getHour() << ":"
                      << std::setw( 2 ) << routeOrder.getArrivalTime().getMinute() << ":" << std::setw( 2 ) << routeOrder.getArrivalTime().getSecond() << " " << std::setw( 2 )
                      << routeOrder.getArrivalTime().getDay() << "/" << std::setw( 2 ) << routeOrder.getArrivalTime().getMonth() << "/" << routeOrder.getArrivalTime().getYear()
                      << " timeToNextOrder=" << routeOrder.getTimeToNextOrder() << " waitTime=" << routeOrder.getWaitTime()
                      << " numberOfPiecesAtArrival=" << routeOrder.getNumberOfPackagesAtArrival() << " numberOfPiecesCollected=" << routeOrder.getCollectedNumberOfPackages()
                      << " numberOfPiecesDelivered=" << routeOrder.getDeliveredNumberOfPackages() << " weightAtArrival=" << routeOrder.getWeightAtArrival()
                      << " weightCollected=" << routeOrder.getCollectedWeight() << " weightDelivered=" << routeOrder.getDeliveredWeight()
                      << " cubeArrival=" << routeOrder.getCubeAtArrival() << " cubeCollected=" << routeOrder.getCollectedCube()
                      << " cubeDelivered=" << routeOrder.getDeliveredCube() << " traveledDistance=" << routeOrder.getTraveledDistance()
                      << " distanceToNextOrder=" << routeOrder.getDistanceToNextOrder() << std::endl;
        }
        std::cout << std::endl;
    }

    void PrintRoutesOnConsole( gem::vrp::RouteList routes )
    {
        for( gem::vrp::Route route : routes )
            PrintRouteOnConsole( route );
    }
}

int main( int argc, char** argv )
{
    // Get new project API token from:
    // https://developer.magiclane.com/api/projects
    Environment::HandleHelpOption( argc, argv );

    std::string projectApiToken = Environment::ResolveApiToken( argc, argv );

    // Sdk objects can be created & used below this line
    Environment::SdkSession session( projectApiToken, { argc > 1 && argv[1][0] != '-' ? argv[1] : "" } ); // SDK API debug logging path

    if( GEM_GET_API_ERROR() != gem::KNoError ) // check for errors after session creation
        return GEM_GET_API_ERROR();

    {
        ProgressListener listener;
        gem::vrp::Service serv;

        gem::vrp::Customer c0;
        c0.setCoordinates( gem::Coordinates( 48.862930, 2.362400 ) );
        c0.setAlias( "Marche des Enfants Rouges (3e)" );
        c0.setPhoneNumber( "+12312312" );
        c0.setEmail( "c0@yahoo.com" );
        int ret = serv.addCustomer( &listener, c0 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c1;
        c1.setCoordinates( gem::Coordinates( 48.860680, 2.336540 ) );
        c1.setAlias( "Rue de Rivoli - Louvre (1er)" );
        c1.setEmail( "c1@yahoo.com" );
        c1.setPhoneNumber( "+12312312" );
        ret = serv.addCustomer( &listener, c1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c2( gem::Coordinates( 48.853740, 2.333070 ) );
        c2.setAlias( "Saint-Germain-des-Pres (6e)" );
        c2.setPhoneNumber( "+12312312" );
        c2.setEmail( "c2@yahoo.com" );
        ret = serv.addCustomer( &listener, c2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c3( gem::Coordinates( 48.842000, 2.350100 ) );
        c3.setAlias( "Rue Mouffetard (5e)" );
        c3.setPhoneNumber( "+12312312" );
        c3.setEmail( "c3@yahoo.com" );
        ret = serv.addCustomer( &listener, c3 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c4( gem::Coordinates( 48.886700, 2.334500 ) );
        c4.setAlias( "Rue Lepic - Montmartre (18e)" );
        c4.setPhoneNumber( "+12312312" );
        c4.setEmail( "c4@yahoo.com" );
        ret = serv.addCustomer( &listener, c4 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c5( gem::Coordinates( 48.884200, 2.319300 ) );
        c5.setAlias( "Les Batignolles (17e)" );
        c5.setPhoneNumber( "+12312312" );
        c5.setEmail( "c5@yahoo.com" );
        ret = serv.addCustomer( &listener, c5 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c6( gem::Coordinates( 48.870900, 2.306900 ) );
        c6.setAlias( "Rue de Ponthieu (8e)" );
        c6.setPhoneNumber( "+12312312" );
        c6.setEmail( "c6@yahoo.com" );
        ret = serv.addCustomer( &listener, c6 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c7( gem::Coordinates( 48.856900, 2.306300 ) );
        c7.setAlias( "Rue Cler (7e)" );
        c7.setPhoneNumber( "+12312312" );
        c7.setEmail( "c7@yahoo.com" );
        ret = serv.addCustomer( &listener, c7 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c8( gem::Coordinates( 48.855700, 2.376000 ) );
        c8.setAlias( "Rue de la Roquette (11e)" );
        c8.setPhoneNumber( "+12312312" );
        c8.setEmail( "c8@yahoo.com" );
        ret = serv.addCustomer( &listener, c8 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c9( gem::Coordinates( 48.831200, 2.355500 ) );
        c9.setAlias( "Place d'Italie (13e)" );
        c9.setPhoneNumber( "+12312312" );
        c9.setEmail( "c9@yahoo.com" );
        ret = serv.addCustomer( &listener, c9 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c10( gem::Coordinates( 48.840400, 2.323700 ) );
        c10.setAlias( "Rue de la Gaite (14e)" );
        c10.setPhoneNumber( "+12312312" );
        c10.setEmail( "c10@yahoo.com" );
        ret = serv.addCustomer( &listener, c10 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c11( gem::Coordinates( 48.871900, 2.381300 ) );
        c11.setAlias( "Rue de Belleville (20e)" );
        c11.setPhoneNumber( "+12312312" );
        c11.setEmail( "c11@yahoo.com" );
        ret = serv.addCustomer( &listener, c11 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        // One working day for a two-van parcel fleet operating inside Paris, within the peripherique. All 12
        // stops, both depots and the shared end yard fit in a ~7.5 km box, and that is what keeps this
        // example solvable: the tour fits comfortably in a single day, so the server never builds the
        // multi-day model. Same geography as the interactive VRPOptimization example.
        //
        // Time windows are MINUTES FROM MIDNIGHT, and only 3 of the 12 orders carry one:
        //
        //   order0  Marais      07:00 - 12:00   morning market drop
        //   order6  Ponthieu    13:00 - 18:00   office reception, afternoon only
        //   order11 Belleville  15:00 - 19:00   last collection of the day
        //
        // Two rules decide where a window can go:
        //
        //   1. It must fit inside ONE vehicle's shift. order0 is reachable only by vehicle 1 (06:30-15:30),
        //      order6 and order11 only by vehicle 2 (11:30-19:30). A window straddling both shifts, or
        //      falling outside them, makes the model infeasible.
        //   2. Never on a member of the fixed sequence - see the note next to it.
        //
        // The other nine are left unconstrained on purpose: each windowed order costs the solver real work,
        // and three are enough to shape the day.
        gem::vrp::OrderList orders;
        gem::vrp::Order order0( c0 );
        order0.setNumberOfPackages( 5 );
        order0.setWeight( 15.7f );
        order0.setCube( 0.2f );
        order0.setServiceTime( 600 );
        order0.setTimeWindow( std::make_pair( 420, 720 ) ); // 07:00 - 12:00 - Marais, morning market drop
        order0.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, order0, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order0 );
        gem::vrp::Order order1( c1 );
        order1.setNumberOfPackages( 4 );
        order1.setWeight( 15.5 );
        order1.setCube( 0.9f );
        order1.setServiceTime( 600 );
        order1.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, order1, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order1 );
        gem::vrp::Order order2( c2 );
        order2.setNumberOfPackages( 8 );
        order2.setWeight( 5.5f );
        order2.setCube( 0.3f );
        order2.setServiceTime( 600 );
        order2.setType( gem::vrp::EOrderType::OT_Delivery );
        ret = serv.addOrder( &listener, order2, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order2 );
        gem::vrp::Order order3( c3 );
        order3.setNumberOfPackages( 6 );
        order3.setWeight( 3.2 );
        order3.setCube( 0.3 );
        order3.setServiceTime( 600 );
        order3.setType( gem::vrp::EOrderType::OT_Delivery );
        ret = serv.addOrder( &listener, order3, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order3 );
        gem::vrp::Order order4( c4 );
        order4.setNumberOfPackages( 8 );
        order4.setWeight( 5.1f );
        order4.setCube( 0.2f );
        order4.setServiceTime( 600 );
        order4.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, order4, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order4 );
        gem::vrp::Order order5( c5 );
        order5.setNumberOfPackages( 11 );
        order5.setWeight( 6.5f );
        order5.setCube( 0.1f );
        order5.setServiceTime( 900 );
        order5.setRevenue( 25 );
        order5.setType( gem::vrp::EOrderType::OT_Delivery );
        ret = serv.addOrder( &listener, order5, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order5 );
        gem::vrp::Order order6( c6 );
        order6.setNumberOfPackages( 4 );
        order6.setWeight( 1.5f );
        order6.setCube( 0.5f );
        order6.setServiceTime( 500 );
        order6.setTimeWindow( std::make_pair( 780, 1080 ) ); // 13:00 - 18:00 - Ponthieu, office reception, afternoon only
        order6.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, order6, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order6 );
        gem::vrp::Order order7( c7 );
        order7.setNumberOfPackages( 12 );
        order7.setWeight( 6.1f );
        order7.setCube( 0.4f );
        order7.setServiceTime( 750 );
        order7.setRevenue( 75 );
        order7.setType( gem::vrp::EOrderType::OT_Delivery );
        ret = serv.addOrder( &listener, order7, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order7 );
        gem::vrp::Order order8( c8 );
        order8.setNumberOfPackages( 7 );
        order8.setWeight( 2.5f );
        order8.setCube( 0.3f );
        order8.setServiceTime( 800 );
        order8.setType( gem::vrp::EOrderType::OT_Delivery );
        order8.setRevenue( 110 );
        ret = serv.addOrder( &listener, order8, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order8 );
        gem::vrp::Order order9( c9 );
        order9.setNumberOfPackages( 12 );
        order9.setWeight( 0.7f );
        order9.setCube( 0.5f );
        order9.setServiceTime( 1000 );
        order9.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, order9, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order9 );
        gem::vrp::Order order10( c10 );
        order10.setNumberOfPackages( 9 );
        order10.setWeight( 4.3f );
        order10.setCube( 0.6f );
        order10.setServiceTime( 850 );
        order10.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, order10, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order10 );
        gem::vrp::Order order11( c11 );
        order11.setNumberOfPackages( 5 );
        order11.setWeight( 4.1f );
        order11.setCube( 0.4f );
        order11.setServiceTime( 600 );
        order11.setTimeWindow( std::make_pair( 900, 1140 ) ); // 15:00 - 19:00 - Belleville, last collection of the day
        order11.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, order11, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order11 );

        // Rue de Rivoli -> Rue Mouffetard -> Saint-Germain: three central stops within ~2 km of each other,
        // in the order a van would drive them anyway.
        //
        // That matters, because OSO_InFixedSequence is stronger than "visit these in this order": the server
        // makes the three stops strictly CONSECUTIVE on one vehicle, with nothing inserted between them. A
        // sequence that zig-zags across the country, or whose members carry time windows, pins that rigid
        // block to one shift and one time span - and the optimization then comes back with no routes at all.
        // Keep the members geographically adjacent and window-free.
        gem::vrp::OrdersSequenceMap ordersSequence;
        gem::LargeIntListList fixedSequence = gem::LargeIntListList { gem::LargeIntList { orders[1].getId(), orders[3].getId(), orders[2].getId() } };
        ordersSequence.insert( std::make_pair( gem::vrp::EOrdersSequenceOption::OSO_InFixedSequence, fixedSequence ) );

        gem::vrp::ConfigurationParameters configParams;
        configParams.setName( "Paris intra-muros optimization" );
        configParams.setIgnoreTimeWindow( false );
        configParams.setOptimizationCriterion( gem::vrp::EOptimizationCriterion::OC_Distance );
        configParams.setOptimizationQuality( gem::vrp::EOptimizationQuality::OQ_Optimized );
        configParams.setMaxWaitTime( 7200 ); // A vehicle can wait maximum 2 hours between a order and the next one, in order to visit the next one within its time window
        configParams.setRouteType( gem::vrp::ERouteType::RT_CustomEnd );
        configParams.setRestrictions( gem::vrp::ERoadRestrictions::RR_None );
        configParams.setDistanceUnit( gem::vrp::EDistanceUnit::DU_Kilometers );
        configParams.setOrderSequenceOptions( ordersSequence );

        gem::vrp::VehicleList vehicles;
        gem::vrp::Vehicle vehicle1;
        vehicle1.setName( "Vehicle 1" );
        vehicle1.setType( gem::vrp::EVehicleType::VT_Car );
        vehicle1.setStatus( gem::vrp::EVehicleStatus::VS_Available );
        vehicle1.setManufacturer( "Kia" );
        vehicle1.setModel( "Ceed" );
        vehicle1.setFuelType( gem::vrp::EFuelType::FT_GasolinePremium );
        vehicle1.setConsumption( 6.5 );
        vehicle1.setLicensePlate( "BV01ASD" );
        vehicle1.setMaxWeight( 300 );
        vehicle1.setMaxCube( 15 );
        // Shift times are MINUTES FROM MIDNIGHT. Two staggered shifts, an early and a late one, so the two
        // legs of the day are covered: 06:30-15:30 and 11:30-19:30.
        //
        // KEEP BOTH END TIMES UNDER 1440 (24:00). The largest end time is the planning horizon, and the
        // moment it crosses a full day the server switches to a multi-day model - it widens every windowed
        // stop's arrival domain to the whole horizon and removes the out-of-window interval of each extra
        // day, on a thread pool the model build blocks on. A single-day horizon skips that entirely, and it
        // is the main reason this Paris version builds quickly where a country-wide tour does not.
        vehicle1.setStartTime( 390 ); // 06:30
        vehicle1.setEndTime( 930 );   // 15:30

        serv.addVehicle( &listener, vehicle1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        vehicles.push_back( vehicle1 );

        gem::vrp::Vehicle vehicle2;
        vehicle2.setName( "Vehicle 2" );
        vehicle2.setType( gem::vrp::EVehicleType::VT_Car );
        vehicle2.setStatus( gem::vrp::EVehicleStatus::VS_Available );
        vehicle2.setManufacturer( "Kia" );
        vehicle2.setModel( "Ceed" );
        vehicle2.setFuelType( gem::vrp::EFuelType::FT_GasolinePremium );
        vehicle2.setConsumption( 6.5 );
        vehicle2.setLicensePlate( "BV02ASD" );
        vehicle2.setMaxWeight( 300 );
        vehicle2.setMaxCube( 15 );
        vehicle2.setStartTime( 690 ); // 11:30
        vehicle2.setEndTime( 1170 );  // 19:30

        serv.addVehicle( &listener, vehicle2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        vehicles.push_back( vehicle2 );

        gem::vrp::VehicleConstraintsList vehConstraintsList;
        gem::vrp::VehicleConstraints vehConstr1;
        vehConstr1.setMaxNumberOfPackages( 100 );
        vehConstr1.setMaxRevenue( 2000 );
        vehConstr1.setStartDate( gem::Time( 2020, 8, 7 ) );
        vehConstr1.setMinNumberOfOrders( 1 );
        vehConstr1.setMaxNumberOfOrders( 50 );
        vehConstr1.setMinDistance( 1 );
        vehConstr1.setMaxDistance( 250 );
        vehConstraintsList.push_back( vehConstr1 );
        gem::vrp::VehicleConstraints vehConstr2;
        vehConstr2.setMaxNumberOfPackages( 100 );
        vehConstr2.setMaxRevenue( 2000 );
        vehConstr2.setStartDate( gem::Time( 2020, 8, 7 ) );
        vehConstr2.setMinNumberOfOrders( 2 );
        vehConstr2.setMaxNumberOfOrders( 60 );
        vehConstr2.setMinDistance( 2 );
        vehConstr2.setMaxDistance( 250 );
        vehConstraintsList.push_back( vehConstr2 );

        gem::vrp::Departure departure1;
        departure1.setAlias( "Depot Nord - Chapelle International (18e)" );
        departure1.setCoordinates( gem::Coordinates( 48.898700, 2.360800 ) );
        gem::vrp::Departure departure2;
        departure2.setAlias( "Depot Sud - Quai de Bercy (12e)" );
        departure2.setCoordinates( gem::Coordinates( 48.833900, 2.386800 ) );

        gem::vrp::Destination destination;
        destination.setAlias( "Depot central - Gare d'Austerlitz (13e)" );
        destination.setCoordinates( gem::Coordinates( 48.842400, 2.365600 ) );
        gem::vrp::Optimization optimization;
        optimization.setConfigurationParameters( configParams );
        optimization.setVehicles( vehicles );
        optimization.setDepartures( { departure1, departure2 } );
        optimization.setDestinations( { destination } ); // both vehicles will end their routes at the same destination
        optimization.setOrders( orders );
        optimization.setVehiclesConstraints( vehConstraintsList );
        optimization.setMatrixBuildType( gem::vrp::EMatrixBuildType::MBT_Real );

        // display orders on map
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext( Environment::WindowFrameworks::Available, "AddFullOptimization" );
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce( oglContext, &mapListener );

        gem::LandmarkList lmks;
        gem::CoordinatesList coords;

        for( int i = 0; i < optimization.getDepartures().size(); i++ )
        {
            gem::Landmark landmark;
            landmark.setName( optimization.getDepartures()[i].getAlias() );
            landmark.setCoordinates( optimization.getDepartures()[i].getCoordinates() );
            landmark.setImage( gem::Icon::Core::GreenBall );

            lmks.push_back( landmark );
            coords.push_back( optimization.getDepartures()[i].getCoordinates() );
        }

        for( int i = 0; i < orders.size(); i++ )
        {
            gem::Landmark landmark;
            landmark.setName( orders[i].getAlias() );
            landmark.setCoordinates( orders[i].getCoordinates() );
            landmark.setImage( gem::Icon::Core::BlueBall );

            lmks.push_back( landmark );
            coords.push_back( orders[i].getCoordinates() );
        }

        for( int i = 0; i < optimization.getDestinations().size(); i++ )
        {
            gem::Landmark landmark;
            landmark.setName( optimization.getDestinations()[i].getAlias() );
            landmark.setCoordinates( optimization.getDestinations()[i].getCoordinates() );
            landmark.setImage( gem::Icon::Core::RedBall );

            lmks.push_back( landmark );
            coords.push_back( optimization.getDestinations()[i].getCoordinates() );
        }

        mapView->activateHighlight( lmks );
        gem::PolygonGeographicArea polyArea( coords );
        mapView->centerOnArea( polyArea );

        ret = WAIT_UNTIL( std::bind( &MapViewListenerImpl::IsFinished, &mapListener ), 15000 );

        gem::vrp::Request request;
        ret = serv.addOptimization( &listener, optimization, request );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 1000000 );

        WAIT_UNTIL(
            [&]()
            {
                serv.getRequest( &listener, request, request.id );
                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 7000 );
                return request.status == gem::vrp::ERequestStatus::eFinished;
            },
            55000 );

        gem::vrp::RouteList routes;
        ret = optimization.getSolution( &listener, routes );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 1000000 );

        if( listener.IsFinished() && listener.GetError() == gem::KNoError && ret == gem::KNoError )
        {
            std::cout << "Problem optimized successfully" << std::endl;
            PrintRoutesOnConsole( routes );

            gem::CoordinatesList shape0 = routes[0].getShape();
            gem::CoordinatesList shape1 = routes[1].getShape();

            // display routes shapes on map
            auto col1 = gem::MarkerCollection( gem::EMarkerType::MT_Polyline, "shape0" );
            col1.add( gem::Marker( shape0 ) );
            mapView->preferences().markers().add( col1 );

            auto col2 = gem::MarkerCollection( gem::EMarkerType::MT_Polyline, "shape1" );
            col2.add( gem::Marker( shape1 ) );
            gem::MarkerCollectionRenderSettings markerCollDisplaySettings;
            markerCollDisplaySettings.polylineInnerColor = gem::Rgba( 0, 0, 255, 0 );
            mapView->preferences().markers().add( col2, markerCollDisplaySettings );
            ret = WAIT_UNTIL( std::bind( &MapViewListenerImpl::IsFinished, &mapListener ), 15000 );

            gem::CoordinatesList shapesCoordinates;
            shapesCoordinates.insert( shapesCoordinates.end(), shape0.begin(), shape0.end() );
            shapesCoordinates.insert( shapesCoordinates.end(), shape1.begin(), shape1.end() );

            gem::PolygonGeographicArea polyArea( shapesCoordinates );
            mapView->centerOnArea( polyArea );
            ret = WAIT_UNTIL( std::bind( &MapViewListenerImpl::IsFinished, &mapListener ), 15000 );

            WAIT_UNTIL_WINDOW_CLOSE();
        }
        else
            std::cout << "Problem couldn't be optimized" << std::endl;
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
