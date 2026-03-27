// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_Timezone.h>

#include <iostream>
#include <chrono>

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
		gem::TimezoneService serv;
		gem::TimezoneResult timezoneResult;

		int currentTimestamp = static_cast<int>( std::chrono::duration_cast<std::chrono::seconds>( std::chrono::system_clock::now().time_since_epoch() ).count() );

		auto res = serv.getTimezoneInfo( timezoneResult, gem::String( "Africa/Johannesburg" ), currentTimestamp, &listener );
		WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 100000 );

		if ( listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError )
			std::cout << "Success getting timezone info!" << std::endl;
		else
			std::cout << "Failed getting timezone info!" << std::endl;
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
