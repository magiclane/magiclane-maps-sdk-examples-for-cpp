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
        c0.setAlias( "c0" );
        c0.setPhoneNumber( "+12312312" );
        c0.setEmail( "c0@yahoo.com" );
        int ret = serv.addCustomer( &listener, c0 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c1;
        c1.setCoordinates( gem::Coordinates( 48.860680, 2.336540 ) );
        c1.setAlias( "c1" );
        c1.setEmail( "c1@yahoo.com" );
        c1.setPhoneNumber( "+12312312" );
        ret = serv.addCustomer( &listener, c1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c2( gem::Coordinates( 48.853740, 2.333070 ) );
        c2.setAlias( "c2" );
        c2.setPhoneNumber( "+12312312" );
        c2.setEmail( "c2@yahoo.com" );
        ret = serv.addCustomer( &listener, c2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c3( gem::Coordinates( 48.842000, 2.350100 ) );
        c3.setAlias( "c3" );
        c3.setPhoneNumber( "+12312312" );
        c3.setEmail( "c3@yahoo.com" );
        ret = serv.addCustomer( &listener, c3 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c4( gem::Coordinates( 48.886700, 2.334500 ) );
        c4.setAlias( "c4" );
        c4.setPhoneNumber( "+12312312" );
        c4.setEmail( "c4@yahoo.com" );
        ret = serv.addCustomer( &listener, c4 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c5( gem::Coordinates( 48.884200, 2.319300 ) );
        c5.setAlias( "c5" );
        c5.setPhoneNumber( "+12312312" );
        c5.setEmail( "c5@yahoo.com" );
        ret = serv.addCustomer( &listener, c5 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c6( gem::Coordinates( 48.870900, 2.306900 ) );
        c6.setAlias( "c6" );
        c6.setPhoneNumber( "+12312312" );
        c6.setEmail( "c6@yahoo.com" );
        ret = serv.addCustomer( &listener, c6 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c7( gem::Coordinates( 48.856900, 2.306300 ) );
        c7.setAlias( "c7" );
        c7.setPhoneNumber( "+12312312" );
        c7.setEmail( "c7@yahoo.com" );
        ret = serv.addCustomer( &listener, c7 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c8( gem::Coordinates( 48.855700, 2.376000 ) );
        c8.setAlias( "c8" );
        c8.setPhoneNumber( "+12312312" );
        c8.setEmail( "c8@yahoo.com" );
        ret = serv.addCustomer( &listener, c8 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c9( gem::Coordinates( 48.831200, 2.355500 ) );
        c9.setAlias( "c9" );
        c9.setPhoneNumber( "+12312312" );
        c9.setEmail( "c9@yahoo.com" );
        ret = serv.addCustomer( &listener, c9 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c10( gem::Coordinates( 48.840400, 2.323700 ) );
        c10.setAlias( "c10" );
        c10.setPhoneNumber( "+12312312" );
        c10.setEmail( "c10@yahoo.com" );
        ret = serv.addCustomer( &listener, c10 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c11( gem::Coordinates( 48.871900, 2.381300 ) );
        c11.setAlias( "c11" );
        c11.setPhoneNumber( "+12312312" );
        c11.setEmail( "c11@yahoo.com" );
        ret = serv.addCustomer( &listener, c11 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        auto nowEpochTime = gem::Time::getUniversalTime();

        gem::vrp::OrderList orders;
        gem::vrp::Order order0( c0 );
        order0.setAlias( "order 0" );
        gem::AddressInfo address0;
        address0.setField( "France", gem::EAddressField::Country );
        address0.setField( "Ile-de-France", gem::EAddressField::County );
        address0.setField( "Paris 3e", gem::EAddressField::City );
        address0.setField( "75003", gem::EAddressField::PostalCode );
        order0.setAddress( address0 );
        order0.setPhoneNumber( "+12312312" );
        order0.setType( gem::vrp::EOrderType::OT_PickUp );
        order0.setNumberOfPackages( 5 );
        order0.setWeight( 15.7f );
        order0.setCube( 0.2f );
        order0.setServiceTime( 600 );
        order0.setTimeWindow( std::make_pair( 420, 900 ) ); // 07:00 - 15:00
        ret = serv.addOrder( &listener, order0, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order0 );
        gem::vrp::Order order1( c1 );
        order1.setAlias( "order 1" );
        gem::AddressInfo address1;
        address1.setField( "France", gem::EAddressField::Country );
        address1.setField( "Ile-de-France", gem::EAddressField::County );
        address1.setField( "Paris 1er", gem::EAddressField::City );
        address1.setField( "75001", gem::EAddressField::PostalCode );
        order1.setAddress( address1 );
        order1.setPhoneNumber( "+12312312" );
        order1.setType( gem::vrp::EOrderType::OT_PickUp );
        order1.setNumberOfPackages( 5 );
        order1.setWeight( 15.5f );
        order1.setCube( 0.9f );
        order1.setTimeWindow( std::make_pair( 600, 1080 ) ); // 10:00 - 18:00
        ret = serv.addOrder( &listener, order1, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order1 );
        gem::vrp::Order order2( c2 );
        order2.setAlias( "order 2" );
        gem::AddressInfo address2;
        address2.setField( "France", gem::EAddressField::Country );
        address2.setField( "Ile-de-France", gem::EAddressField::County );
        address2.setField( "Paris 6e", gem::EAddressField::City );
        address2.setField( "75006", gem::EAddressField::PostalCode );
        order2.setAddress( address2 );
        order2.setPhoneNumber( "+12312312" );
        order2.setType( gem::vrp::EOrderType::OT_Delivery );
        order2.setNumberOfPackages( 8 );
        order2.setWeight( 5.5f );
        order2.setCube( 0.5f );
        order2.setServiceTime( 600 );
        order2.setTimeWindow( std::make_pair( 780, 1200 ) ); // 13:00 - 20:00
        ret = serv.addOrder( &listener, order2, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order2 );
        gem::vrp::Order order3( c3 );
        order3.setAlias( "order 3" );
        gem::AddressInfo address3;
        address3.setField( "France", gem::EAddressField::Country );
        address3.setField( "Ile-de-France", gem::EAddressField::County );
        address3.setField( "Paris 5e", gem::EAddressField::City );
        address3.setField( "75005", gem::EAddressField::PostalCode );
        order3.setAddress( address3 );
        order3.setPhoneNumber( "+12312312" );
        order3.setType( gem::vrp::EOrderType::OT_Delivery );
        ret = serv.addOrder( &listener, order3, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order3 );
        gem::vrp::Order order4( c4 );
        order4.setAlias( "order 4" );
        gem::AddressInfo address4;
        address4.setField( "France", gem::EAddressField::Country );
        address4.setField( "Ile-de-France", gem::EAddressField::County );
        address4.setField( "Paris 18e", gem::EAddressField::City );
        address4.setField( "75018", gem::EAddressField::PostalCode );
        order4.setAddress( address4 );
        order4.setPhoneNumber( "+12312312" );
        order4.setType( gem::vrp::EOrderType::OT_PickUp );
        order4.setNumberOfPackages( 8 );
        order4.setWeight( 5.1f );
        order4.setCube( 0.1f );
        order4.setServiceTime( 600 );
        ret = serv.addOrder( &listener, order4, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order4 );
        gem::vrp::Order order5( c5 );
        order5.setAlias( "order 5" );
        gem::AddressInfo address5;
        address5.setField( "France", gem::EAddressField::Country );
        address5.setField( "Ile-de-France", gem::EAddressField::County );
        address5.setField( "Paris 17e", gem::EAddressField::City );
        address5.setField( "75017", gem::EAddressField::PostalCode );
        order5.setAddress( address5 );
        order5.setPhoneNumber( "+12312312" );
        order5.setType( gem::vrp::EOrderType::OT_Delivery );
        order5.setNumberOfPackages( 11 );
        order5.setWeight( 6.5f );
        order5.setCube( 0.4f );
        order5.setServiceTime( 900 );
        order5.setRevenue( 25 );
        ret = serv.addOrder( &listener, order5, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order5 );
        gem::vrp::Order order6( c6 );
        order6.setAlias( "order 6" );
        gem::AddressInfo address6;
        address6.setField( "France", gem::EAddressField::Country );
        address6.setField( "Ile-de-France", gem::EAddressField::County );
        address6.setField( "Paris 8e", gem::EAddressField::City );
        address6.setField( "75008", gem::EAddressField::PostalCode );
        order6.setAddress( address6 );
        order6.setPhoneNumber( "+12312312" );
        order6.setType( gem::vrp::EOrderType::OT_PickUp );
        order6.setNumberOfPackages( 4 );
        order6.setWeight( 1.5f );
        order6.setCube( 0.5f );
        order6.setServiceTime( 500 );
        ret = serv.addOrder( &listener, order6, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order6 );
        gem::vrp::Order order7( c7 );
        order7.setAlias( "order 7" );
        gem::AddressInfo address7;
        address7.setField( "France", gem::EAddressField::Country );
        address7.setField( "Ile-de-France", gem::EAddressField::County );
        address7.setField( "Paris 7e", gem::EAddressField::City );
        address7.setField( "75007", gem::EAddressField::PostalCode );
        order7.setAddress( address7 );
        order7.setPhoneNumber( "+12312312" );
        order7.setType( gem::vrp::EOrderType::OT_Delivery );
        order7.setNumberOfPackages( 2 );
        order7.setWeight( 6.1f );
        order7.setCube( 0.3f );
        order7.setServiceTime( 750 );
        ret = serv.addOrder( &listener, order7, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order7 );
        gem::vrp::Order order8( c8 );
        order8.setAlias( "order 8" );
        gem::AddressInfo address8;
        address8.setField( "France", gem::EAddressField::Country );
        address8.setField( "Ile-de-France", gem::EAddressField::County );
        address8.setField( "Paris 11e", gem::EAddressField::City );
        address8.setField( "75011", gem::EAddressField::PostalCode );
        order8.setAddress( address8 );
        order8.setPhoneNumber( "+12312312" );
        order8.setType( gem::vrp::EOrderType::OT_Delivery );
        order8.setNumberOfPackages( 7 );
        order8.setWeight( 2.5f );
        order8.setCube( 0.2f );
        order8.setServiceTime( 800 );
        order8.setRevenue( 110 );
        ret = serv.addOrder( &listener, order8, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order8 );
        gem::vrp::Order order9( c9 );
        order9.setAlias( "order 9" );
        gem::AddressInfo address9;
        address9.setField( "France", gem::EAddressField::Country );
        address9.setField( "Ile-de-France", gem::EAddressField::County );
        address9.setField( "Paris 13e", gem::EAddressField::City );
        address9.setField( "75013", gem::EAddressField::PostalCode );
        order9.setAddress( address9 );
        order9.setPhoneNumber( "+12312312" );
        order9.setType( gem::vrp::EOrderType::OT_PickUp );
        order9.setNumberOfPackages( 1 );
        order9.setWeight( 3.7f );
        order9.setCube( 0.3f );
        order9.setServiceTime( 1000 );
        ret = serv.addOrder( &listener, order9, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order9 );
        gem::vrp::Order order10( c10 );
        order10.setAlias( "order 10" );
        gem::AddressInfo address10;
        address10.setField( "France", gem::EAddressField::Country );
        address10.setField( "Ile-de-France", gem::EAddressField::County );
        address10.setField( "Paris 14e", gem::EAddressField::City );
        address10.setField( "75014", gem::EAddressField::PostalCode );
        order10.setAddress( address10 );
        order10.setPhoneNumber( "+12312312" );
        order10.setType( gem::vrp::EOrderType::OT_PickUp );
        order10.setNumberOfPackages( 9 );
        order10.setWeight( 4.3f );
        order10.setCube( 0.6f );
        order10.setServiceTime( 850 );
        ret = serv.addOrder( &listener, order10, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order10 );
        gem::vrp::Order order11( c11 );
        order11.setAlias( "order 11" );
        gem::AddressInfo address11;
        address11.setField( "France", gem::EAddressField::Country );
        address11.setField( "Ile-de-France", gem::EAddressField::County );
        address11.setField( "Paris 20e", gem::EAddressField::City );
        order11.setAddress( address11 );
        order11.setPhoneNumber( "+12312312" ); //set order's phone number
        order11.setType( gem::vrp::EOrderType::OT_PickUp );
        order11.setNumberOfPackages( 5 );
        order11.setWeight( 4.1f );
        order11.setCube( 0.4f );
        order11.setServiceTime( 600 );
        ret = serv.addOrder( &listener, order11, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        orders.push_back( order11 );

        gem::vrp::Departure departure;
        departure.setAlias( "Depot" );
        departure.setAddress( address1 );
        departure.setCoordinates( gem::Coordinates( 48.898700, 2.360800 ) );

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
        vehicle1.setMaxWeight( 350 );
        vehicle1.setMaxCube( 15 );
        vehicle1.setStartTime( 360 );
        vehicle1.setEndTime( 1260 );

        ret = serv.addVehicle( &listener, vehicle1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        vehicles.push_back( vehicle1 );

        gem::vrp::VehicleConstraintsList vehConstraintsList;
        gem::vrp::VehicleConstraints vehConstr1;
        vehConstr1.setStartDate( gem::Time( 2024, 8, 7 ) );
        vehConstraintsList.push_back( vehConstr1 );

        gem::vrp::Optimization optimization;
        optimization.setOrders( orders );
        optimization.setDepartures( { departure } );
        optimization.setVehicles( vehicles );
        optimization.setVehiclesConstraints( vehConstraintsList );

        //display orders on map
        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext( Environment::WindowFrameworks::Available, "AddOptimizationWithOrderFields" );
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

        //add optimization
        gem::vrp::Request request;
        ret = serv.addOptimization( &listener, optimization, request );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        WAIT_UNTIL(
            [&]()
            {
                serv.getRequest( &listener, request, request.id );
                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 7000 );
                return request.status == gem::vrp::ERequestStatus::eFinished;
            },
            40000 );

        gem::vrp::RouteList routes;
        ret = optimization.getSolution( &listener, routes );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        if( listener.IsFinished() && listener.GetError() == gem::KNoError && ret == gem::KNoError )
        {
            std::cout << "Problem optimized successfully" << std::endl;
            PrintRoutesOnConsole( routes );

            gem::CoordinatesList shape0 = routes[0].getShape();

            // display routes shapes on map
            auto col1 = gem::MarkerCollection( gem::EMarkerType::MT_Polyline, "shape0" );
            col1.add( gem::Marker( shape0 ) );
            mapView->preferences().markers().add( col1 );

            gem::CoordinatesList shapesCoordinates;
            shapesCoordinates.insert( shapesCoordinates.end(), shape0.begin(), shape0.end() );

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
