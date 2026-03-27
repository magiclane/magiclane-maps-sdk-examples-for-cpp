// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_SearchService.h>

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
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "SearchFreeTextLimitByGeographicArea", &pTouchEventListener));
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

	{
		// Perform the search
		gem::LandmarkList results;
		{
			ProgressListener searchListener;

			// Upper-left,lower-right bounding box given as lat,lon coordinate pairs in degrees.
			gem::RectangleGeographicArea rgaLatLon({34.14083, -118.12958}, {34.13146, -118.12187});

			// Text to search for
			gem::SearchService().search(results, &searchListener, "Laboratory", gem::Coordinates(34.138, -118.124), gem::SearchPreferences(), rgaLatLon);

			auto ret = WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &searchListener), 15000);
		}
		if ( results.size() > 0 )
		{
			mapView->centerOnCoordinates(results[0].getCoordinates(), 75);
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
