// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
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

        gem::vrp::Optimization optimization;
        gem::LargeInteger optimizationId = -1; // set your optimization id
        int ret = serv.getOptimization( &listener, optimization, optimizationId );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 20000 );

        gem::vrp::Customer c1;
        c1.setCoordinates( gem::Coordinates( 48.849000, 2.399000 ) );
        c1.setAlias( "c1" );
        c1.setPhoneNumber( "+1231231" );
        c1.setEmail( "c1@yahoo.com" );
        ret = serv.addCustomer( &listener, c1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::Customer c2( gem::Coordinates( 48.845800, 2.276800 ) );
        c2.setAlias( "c2" );
        c2.setPhoneNumber( "+1231231" );
        c2.setEmail( "c2@yahoo.com" );
        ret = serv.addCustomer( &listener, c2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

        gem::vrp::OrderList ordersToAdd;
        gem::vrp::Order orderToAdd1( c1 );
        orderToAdd1.setServiceTime( 600 );
        orderToAdd1.setType( gem::vrp::EOrderType::OT_PickUp );
        ret = serv.addOrder( &listener, orderToAdd1, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        ordersToAdd.push_back( orderToAdd1 );
        gem::vrp::Order orderToAdd2( c2 );
        orderToAdd2.setType( gem::vrp::EOrderType::OT_Delivery );
        ret = serv.addOrder( &listener, orderToAdd2, false );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        ordersToAdd.push_back( orderToAdd2 );

        gem::FloatListList distancesToTheOtherOrders {
            gem::FloatList { 228.000f, 418.767f, 132.615f, 304.354f, 790.060f, 480.738f, 622.623f, 303.991f, 376.655f, 687.590f, 289.146f, 427.011f,
                             228.000f }, //on the last position is the distance to the departure
            gem::FloatList { 576.646f, 903.98f, 376.969f, 329.505f, 223.536f, 142.910f, 555.398f, 652.637f, 253.191f, 427.311f, 448.662f, 429.303f, 542.869f, 576.646f } };
        gem::IntListList timesToTheOtherOrders {
            gem::IntList { 7814, 19451, 6606, 12432, 26843, 21555, 21043, 10384, 15204, 23708, 9441, 14557, 7814 }, //on the last position is the time to the departure
            gem::IntList { 19189, 5553, 16549, 12661, 12359, 7948, 19183, 21759, 10575, 15377, 15724, 17004, 18917, 19189 } };

        bool reoptimize = true;
        gem::vrp::Request request;
        ret = optimization.addOrders( &listener, ordersToAdd, request, reoptimize, { std::make_pair( gem::vrp::EVehicleType::VT_Car, distancesToTheOtherOrders ) },
                                      { std::make_pair( gem::vrp::EVehicleType::VT_Car, timesToTheOtherOrders ) } );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        WAIT_UNTIL(
            [&]()
            {
                serv.getRequest( &listener, request, request.id );
                WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 7000 );
                return request.status == gem::vrp::ERequestStatus::eFinished;
            },
            40000 );

        if( listener.IsFinished() && listener.GetError() == gem::KNoError && ret == gem::KNoError )
            std::cout << "Orders added successfully" << std::endl;
        else
            std::cout << "Orders couldn't be added or the optimization couldn't be reoptimized" << std::endl;
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
