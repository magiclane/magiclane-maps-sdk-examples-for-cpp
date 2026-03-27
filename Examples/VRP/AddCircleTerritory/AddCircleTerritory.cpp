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
        gem::vrp::Territory territory;
        territory.setName("Circle territory");
        territory.setType(gem::vrp::ETerritoryType::TT_Circle);
        territory.setColor(gem::Rgba(255, 42, 0, 1));

        gem::CoordinatesList data;
        gem::Coordinates center = { 46.603125, 0.354550 };
        gem::Coordinates radius = { 46.593494, 0.376579 };
        data.push_back(center); data.push_back(radius);
        territory.setData(data);

        ProgressListener listener;
        gem::vrp::Service serv;
        int res = serv.addTerritory(&listener, territory);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
        {
            std::cout << "Territory added successfully and has the id = " << territory.getId() << "." << std::endl;
        }
        else
            std::cout << "Territory couldn't be added" << std::endl;

        // The customers returned by the method territory.getCustomers() are only the ones that were previously saved using the method serv.addCustomer(), not the customers that were used in optimizations.
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
