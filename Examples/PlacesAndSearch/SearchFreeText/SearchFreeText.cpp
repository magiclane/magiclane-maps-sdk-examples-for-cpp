// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_SearchService.h>
#include "API/GEM_MapViewPreferences.h"

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


	// Create an interactive map view
	CTouchEventListener pTouchEventListener;
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "SearchFreeText", &pTouchEventListener)); 
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

	{
		// Perform the search
		gem::LandmarkList results;
		{
			ProgressListener searchListener;

			// Text to search for; position given as lat,lon coordinates in degrees
			gem::SearchService().search(results, &searchListener, "Empire State Building", gem::Coordinates(40.73956748162446, -73.98849902713857));

			auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &searchListener), 15000);
		}
		if ( results.size() > 0 )
		{
            auto firstResult = results[0];            

			mapView->centerOnCoordinates(firstResult.getCoordinates(), 60, gem::Xy(), gem::Animation(gem::AnimationLinear, gem::ProgressListener(), 5000));

            results.clear();
            results.push_back( firstResult );

			mapView->activateHighlight(results);
		}
	}

	WAIT_UNTIL_WINDOW_CLOSE();
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
