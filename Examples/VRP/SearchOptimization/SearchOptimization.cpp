// SPDX-FileCopyrightText: 2024-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>

#include <iostream>

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

        //initialize customers
        gem::vrp::Customer c0;
        c0.setCoordinates( gem::Coordinates( 48.862930, 2.362400 ) );
        c0.setAlias( "c0" );
        c0.setPhoneNumber( "+12312312" );
        c0.setEmail( "c0@yahoo.com" );
        serv.addCustomer( &listener, c0 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c1;
        c1.setCoordinates( gem::Coordinates( 48.860680, 2.336540 ) );
        c1.setAlias( "c1" );
        c1.setEmail( "c1@yahoo.com" );
        c1.setPhoneNumber( "+12312312" );
        serv.addCustomer( &listener, c1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c2( gem::Coordinates( 48.853740, 2.333070 ) );
        c2.setAlias( "c2" );
        c2.setPhoneNumber( "+12312312" );
        c2.setEmail( "c2@yahoo.com" );
        serv.addCustomer( &listener, c2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c3( gem::Coordinates( 48.842000, 2.350100 ) );
        c3.setAlias( "c3" );
        c3.setPhoneNumber( "+12312312" );
        c3.setEmail( "c3@yahoo.com" );
        serv.addCustomer( &listener, c3 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c4( gem::Coordinates( 48.886700, 2.334500 ) );
        c4.setAlias( "c4" );
        c4.setPhoneNumber( "+12312312" );
        c4.setEmail( "c4@yahoo.com" );
        serv.addCustomer( &listener, c4 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c5( gem::Coordinates( 48.884200, 2.319300 ) );
        c5.setAlias( "c5" );
        c5.setPhoneNumber( "+12312312" );
        c5.setEmail( "c5@yahoo.com" );
        serv.addCustomer( &listener, c5 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c6( gem::Coordinates( 48.870900, 2.306900 ) );
        c6.setAlias( "c6" );
        c6.setPhoneNumber( "+12312312" );
        c6.setEmail( "c6@yahoo.com" );
        serv.addCustomer( &listener, c6 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c7( gem::Coordinates( 48.856900, 2.306300 ) );
        c7.setAlias( "c7" );
        c7.setPhoneNumber( "+12312312" );
        c7.setEmail( "c7@yahoo.com" );
        serv.addCustomer( &listener, c7 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c8( gem::Coordinates( 48.855700, 2.376000 ) );
        c8.setAlias( "c8" );
        c8.setPhoneNumber( "+12312312" );
        c8.setEmail( "c8@yahoo.com" );
        serv.addCustomer( &listener, c8 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c9( gem::Coordinates( 48.831200, 2.355500 ) );
        c9.setAlias( "c9" );
        c9.setPhoneNumber( "+12312312" );
        c9.setEmail( "c9@yahoo.com" );
        serv.addCustomer( &listener, c9 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c10( gem::Coordinates( 48.840400, 2.323700 ) );
        c10.setAlias( "c10" );
        c10.setPhoneNumber( "+12312312" );
        c10.setEmail( "c10@yahoo.com" );
        serv.addCustomer( &listener, c10 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c11( gem::Coordinates( 48.871900, 2.381300 ) );
        c11.setAlias( "c11" );
        c11.setPhoneNumber( "+12312312" );
        c11.setEmail( "c11@yahoo.com" );
        serv.addCustomer( &listener, c11 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        //initialize orders
        gem::vrp::OrderList orders1, orders2;
        gem::vrp::Order order0( c0 );
        order0.setNumberOfPackages( 5 );
        order0.setWeight( 15.7f );
        order0.setCube( 0.2f );
        order0.setServiceTime( 600 );
        order0.setTimeWindow( std::make_pair( 420, 900 ) ); // 07:00 - 15:00
        order0.setType( gem::vrp::EOrderType::OT_PickUp );
        orders1.push_back( order0 );

        gem::vrp::Order order1( c1 );
        order1.setNumberOfPackages( 4 );
        order1.setWeight( 15.5 );
        order1.setCube( 0.9f );
        order1.setTimeWindow( std::make_pair( 600, 1080 ) ); // 10:00 - 18:00
        order1.setType( gem::vrp::EOrderType::OT_PickUp );
        orders1.push_back( order1 );

        gem::vrp::Order order2( c2 );
        order2.setNumberOfPackages( 8 );
        order2.setWeight( 5.5 );
        order2.setCube( 0.3f );
        order2.setServiceTime( 600 );
        order2.setTimeWindow( std::make_pair( 780, 1200 ) ); // 13:00 - 20:00
        order2.setType( gem::vrp::EOrderType::OT_Delivery );
        orders1.push_back( order2 );

        gem::vrp::Order order3( c3 );
        order3.setType( gem::vrp::EOrderType::OT_Delivery );
        orders1.push_back( order3 );

        gem::vrp::Order order4( c4 );
        order4.setNumberOfPackages( 8 );
        order4.setWeight( 5.1f );
        order4.setCube( 0.2f );
        order4.setServiceTime( 600 );
        order4.setType( gem::vrp::EOrderType::OT_PickUp );
        orders1.push_back( order4 );

        gem::vrp::Order order5( c5 );
        order5.setNumberOfPackages( 11 );
        order5.setWeight( 6.5 );
        order5.setCube( 0.1f );
        order5.setServiceTime( 900 );
        order5.setRevenue( 25 );
        order5.setType( gem::vrp::EOrderType::OT_Delivery );
        orders1.push_back( order5 );

        gem::vrp::Order order6( c6 );
        order6.setNumberOfPackages( 4 );
        order6.setWeight( 1.5 );
        order6.setCube( 0.5 );
        order6.setServiceTime( 500 );
        order6.setType( gem::vrp::EOrderType::OT_PickUp );
        orders1.push_back( order6 );

        gem::vrp::Order order7( c7 );
        order7.setNumberOfPackages( 12 );
        order7.setWeight( 6.1f );
        order7.setCube( 0.4f );
        order7.setServiceTime( 750 );
        order7.setRevenue( 75 );
        order7.setType( gem::vrp::EOrderType::OT_Delivery );
        orders1.push_back( order7 );

        gem::vrp::Order order8( c8 );
        order8.setNumberOfPackages( 7 );
        order8.setWeight( 2.5 );
        order8.setCube( 0.3f );
        order8.setServiceTime( 800 );
        order8.setType( gem::vrp::EOrderType::OT_Delivery );
        order8.setRevenue( 110 );
        orders1.push_back( order8 );

        gem::vrp::Order order9( c9 );
        order9.setNumberOfPackages( 12 );
        order9.setWeight( 0.7f );
        order9.setCube( 0.5 );
        order9.setServiceTime( 1000 );
        order9.setType( gem::vrp::EOrderType::OT_PickUp );
        orders1.push_back( order9 );

        gem::vrp::Order order10( c10 );
        order10.setNumberOfPackages( 9 );
        order10.setWeight( 4.3f );
        order10.setCube( 0.6f );
        order10.setServiceTime( 850 );
        order10.setType( gem::vrp::EOrderType::OT_PickUp );
        orders1.push_back( order10 );

        gem::vrp::Order order11( c11 );
        order11.setNumberOfPackages( 5 );
        order11.setWeight( 4.1f );
        order11.setCube( 0.4f );
        order11.setServiceTime( 600 );
        order11.setType( gem::vrp::EOrderType::OT_PickUp );
        orders1.push_back( order11 );

        //make a copy of the order list that will be used to create the second optimization
        orders2 = orders1;

        //add orders list for first optimization
        for( int i = 0; i < orders2.size(); i++ )
        {
            serv.addOrder( &listener, orders1.at_nc( i ), false );
            WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        }

        //add orders list for second optimization
        for( int i = 0; i < orders2.size(); i++ )
        {
            serv.addOrder( &listener, orders2.at_nc( i ), false );
            WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        }

        gem::vrp::OrdersSequenceMap ordersSequence;
        gem::LargeIntListList fixedSequence = { { orders1[2].getId(), orders1[8].getId(), orders1[6].getId() } };
        ordersSequence.insert( std::make_pair( gem::vrp::EOrdersSequenceOption::OSO_InFixedSequence, fixedSequence ) );

        //initialize configurations
        gem::vrp::ConfigurationParameters configParams1;
        configParams1.setName( "Paris intra-muros optimization1" );
        configParams1.setIgnoreTimeWindow( false );
        configParams1.setOptimizationCriterion( gem::vrp::EOptimizationCriterion::OC_Distance );
        configParams1.setOptimizationQuality( gem::vrp::EOptimizationQuality::OQ_Optimized );
        configParams1.setMaxWaitTime( 7200 ); // A vehicle can wait maximum 2 hours between a order and the next one, in order to visit the next one within its time window
        configParams1.setRouteType( gem::vrp::ERouteType::RT_CustomEnd );
        configParams1.setRestrictions( gem::vrp::ERoadRestrictions::RR_None );
        configParams1.setDistanceUnit( gem::vrp::EDistanceUnit::DU_Kilometers );
        configParams1.setOrderSequenceOptions( ordersSequence );

        gem::vrp::OrdersSequenceMap ordersSequence2;
        gem::LargeIntListList fixedSequence2 = { { orders2[2].getId(), orders2[8].getId(), orders2[6].getId() } };
        ordersSequence2.insert( std::make_pair( gem::vrp::EOrdersSequenceOption::OSO_InFixedSequence, fixedSequence2 ) );

        gem::vrp::ConfigurationParameters configParams2;
        configParams2.setName( "Paris intra-muros optimization2" );
        configParams2.setIgnoreTimeWindow( false );
        configParams2.setOptimizationCriterion( gem::vrp::EOptimizationCriterion::OC_Distance );
        configParams2.setOptimizationQuality( gem::vrp::EOptimizationQuality::OQ_Optimized );
        configParams2.setMaxWaitTime( 7200 ); // A vehicle can wait maximum 2 hours between a order and the next one, in order to visit the next one within its time window
        configParams2.setRouteType( gem::vrp::ERouteType::RT_CustomEnd );
        configParams2.setRestrictions( gem::vrp::ERoadRestrictions::RR_None );
        configParams2.setDistanceUnit( gem::vrp::EDistanceUnit::DU_Kilometers );
        configParams2.setOrderSequenceOptions( ordersSequence2 );

        //initialize first vehicle
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
        vehicle1.setStartTime( 360 ); // 06:00, in minutes from midnight
        vehicle1.setEndTime( 1260 );  // 21:00, in minutes from midnight - a single-day shift

        //add first vehicle
        int res = serv.addVehicle( &listener, vehicle1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        vehicles.push_back( vehicle1 );

        //initialize second vehicle
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
        vehicle2.setStartTime( 360 ); // 06:00, in minutes from midnight
        vehicle2.setEndTime( 1260 );  // 21:00, in minutes from midnight - a single-day shift

        //add second vehicle
        res = serv.addVehicle( &listener, vehicle2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        vehicles.push_back( vehicle2 );

        //initialize vehicle constraints
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

        //initialize departures
        gem::vrp::Departure departure1;
        departure1.setAlias( "Depot 1" );
        departure1.setCoordinates( gem::Coordinates( 48.898700, 2.360800 ) );
        gem::vrp::Departure departure2;
        departure2.setAlias( "Depot 2" );
        departure2.setCoordinates( gem::Coordinates( 48.833900, 2.386800 ) );

        gem::vrp::Destination destination;
        destination.setAlias( "Destination" );
        destination.setCoordinates( gem::Coordinates( 48.842400, 2.365600 ) );

        //initialize first optimization
        gem::vrp::Optimization optimization1;
        optimization1.setConfigurationParameters( configParams1 );
        optimization1.setVehicles( vehicles );
        optimization1.setDepartures( { departure1, departure2 } );
        optimization1.setDestinations( { destination } ); // both vehicles will end their routes at the same destination
        optimization1.setOrders( orders1 );
        optimization1.setVehiclesConstraints( vehConstraintsList );
        optimization1.setMatrixBuildType( gem::vrp::EMatrixBuildType::MBT_Real );

        //add first optimization
        gem::vrp::Request request1;
        serv.addOptimization( &listener, optimization1, request1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        WAIT_UNTIL(
            [&]()
            {
                serv.getRequest( &listener, request1, request1.id );
                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 7000 );
                return request1.status == gem::vrp::ERequestStatus::eFinished;
            },
            40000 );

        gem::vrp::RouteList routes1;
        optimization1.getSolution( &listener, routes1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        //initialize second optimization
        gem::vrp::Optimization optimization2;
        optimization2.setConfigurationParameters( configParams2 );
        optimization2.setVehicles( vehicles );
        optimization2.setDepartures( { departure1, departure2 } );
        optimization2.setDestinations( { destination } ); // both vehicles will end their routes at the same destination
        optimization2.setOrders( orders2 );
        optimization2.setVehiclesConstraints( vehConstraintsList );
        optimization2.setMatrixBuildType( gem::vrp::EMatrixBuildType::MBT_Real );

        //add second optimization
        gem::vrp::Request request2;
        serv.addOptimization( &listener, optimization2, request2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        WAIT_UNTIL(
            [&]()
            {
                serv.getRequest( &listener, request2, request2.id );
                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 7000 );
                return request2.status == gem::vrp::ERequestStatus::eFinished;
            },
            40000 );

        gem::vrp::RouteList routes2;
        optimization1.getSolution( &listener, routes2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        //get all optimizations from database that contains string "optimization1"
        gem::vrp::OptimizationList allOptimizations;
        res = serv.getOptimizations( &listener, allOptimizations, "optimization1" );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 20000 );

        if( listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError )
            std::cout << allOptimizations.size() << " optimizations returned successfully" << std::endl;
        else
            std::cout << "No optimization returned" << std::endl;
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
