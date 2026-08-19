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
    Environment::HandleHelpOption( argc, argv );

    std::string projectApiToken = Environment::ResolveApiToken( argc, argv );

    // Sdk objects can be created & used below this line
    Environment::SdkSession session( projectApiToken, { argc > 1 && argv[1][0] != '-' ? argv[1] : "" } ); // SDK API debug logging path

    if( GEM_GET_API_ERROR() != gem::KNoError ) // check for errors after session creation
        return GEM_GET_API_ERROR();

    // Create an interactive map view
    CTouchEventListener pTouchEventListener;
    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::Available, "SearchAround", &pTouchEventListener ) );
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    {
        // Perform the search
        gem::LandmarkList results;
        {
            ProgressListener searchListener;

            // Position around which to search, given as lat,lon coordinates in degrees.
            gem::SearchService().searchAroundPosition( results, &searchListener, gem::Coordinates( 25.607, -80.399 ) );

            WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &searchListener ), 15000 );
        }
        if( !results.empty() )
        {
            GEM_LOGI( "Search around position returned %d results, highlighting them", ( int ) results.size() );
            mapView->centerOnCoordinates( results[0].getCoordinates(), 80 );
            mapView->activateHighlight( results );
        }
        else
        {
            GEM_LOGE( "Search around position returned no results - nothing to highlight" );
        }
    }

    WAIT_UNTIL_WINDOW_CLOSE();
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
