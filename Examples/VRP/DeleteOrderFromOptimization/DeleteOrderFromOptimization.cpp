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

        gem::LargeInteger optimizationId = -1; // set your optimization id

        gem::vrp::Optimization optimization;
        int res = serv.getOptimization( &listener, optimization, optimizationId );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 20000 );

        if( optimization.getOrders().size() > 4 )
        {
            gem::vrp::Order order = optimization.getOrders().at( 4 );

            res = optimization.deleteOrder( &listener, order );
            WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );

            if( listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError )
                std::cout << "Order deleted successfully" << std::endl;
            else
                std::cout << "Order couldn't be deleted" << std::endl;
        }
        else
            std::cout << "The optimization hasn't at least four orders" << std::endl;
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
