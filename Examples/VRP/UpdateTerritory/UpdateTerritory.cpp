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

        gem::vrp::Territory territory;
        gem::LargeInteger territoryId = -1; // set your territory id  
        int res = serv.getTerritory(&listener, territory, territoryId);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        //update territory
        territory.setName("Rectangle territory (updated)");
        territory.setType(gem::vrp::ETerritoryType::TT_Rectangle);
        gem::CoordinatesList data;
        gem::Coordinates corner1 = { 46.575722, 0.345311 };
        gem::Coordinates corner2 = { 46.582438, 0.358969 };
        data.push_back(corner1); data.push_back(corner2);
        territory.setData(data);

        res = serv.updateTerritory(&listener, territory);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
            std::cout << "Territory updated successfully" << std::endl;
        else
            std::cout << "Territory couldn't be updated" << std::endl;
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
