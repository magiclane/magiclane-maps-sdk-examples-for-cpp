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

        gem::vrp::Customer customer;
        gem::LargeInteger customerId = -1;      // set your customer id  
        int res = serv.getCustomer(&listener, customer, customerId);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        gem::vrp::CustomerOrderList history;
        res = serv.getCustomerOrdersHistory(&listener, history, customer);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
        {
            std::cout << "Customer has the following orders:" << std::endl;
            for (gem::vrp::CustomerOrder cOrders : history)
            {
                std::cout << "Order with id = " << cOrders.order.getId() << std::endl;
                std::cout << "  Optimization with id = " << cOrders.optimizationAndRoute.first.getId() << std::endl;
                std::cout << "  Route with id = " << cOrders.optimizationAndRoute.second.getId() << std::endl;
            }
        }
        else
            std::cout << "Customer doesn't have any orders." << std::endl;
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
