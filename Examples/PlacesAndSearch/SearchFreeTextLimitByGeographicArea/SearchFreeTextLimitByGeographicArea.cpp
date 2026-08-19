// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_Markers.h>
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
        session.produceOpenGLContext( Environment::WindowFrameworks::Available, "SearchFreeTextLimitByGeographicArea", &pTouchEventListener ) );
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    {
        // Upper-left,lower-right bounding box given as lat,lon coordinate pairs in degrees.
        gem::RectangleGeographicArea rgaLatLon( { 34.14083, -118.12958 }, { 34.136145, -118.12187 } );

        // Perform the search
        gem::LandmarkList results;
        {
            ProgressListener searchListener;

            // Text to search for
            gem::SearchService().search( results, &searchListener, "Laboratory", gem::Coordinates( 34.138, -118.124 ), gem::SearchPreferences(), rgaLatLon );

            WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &searchListener ), 15000 );
        }

        const gem::Coordinates topLeft = rgaLatLon.getTopLeft();
        const gem::Coordinates bottomRight = rgaLatLon.getBottomRight();

        {
            // Render the search-limit area as an unfilled rectangle outline (polyline
            // marker), so it is visible that only results inside it are highlighted.
            auto col = gem::MarkerCollection( gem::EMarkerType::MT_Polyline, "search area" );

            // Closed rectangle outline: the 4 corners, then back to the first one.
            col.add( gem::Marker( { { topLeft.getLatitude(), topLeft.getLongitude() },
                                    { topLeft.getLatitude(), bottomRight.getLongitude() },
                                    { bottomRight.getLatitude(), bottomRight.getLongitude() },
                                    { bottomRight.getLatitude(), topLeft.getLongitude() },
                                    { topLeft.getLatitude(), topLeft.getLongitude() } } ) );

            gem::MarkerCollectionRenderSettings markerCollDisplaySettings;
            markerCollDisplaySettings.setPolylineInnerColor( gem::Rgba( 255, 98, 0, 255 ) ).setPolylineInnerSize( 1.5 );

            mapView->preferences().markers().add( col, markerCollDisplaySettings );
        }

        if( results.size() > 0 )
        {
            GEM_LOGI( "Area-limited search returned %d results inside the rectangle, highlighting them", ( int ) results.size() );
            // Fit the search-limit area with some breathing room around it (auto zoom):
            // centering on the exact area would put the rectangle outline right at the
            // screen edges, so center on a copy padded by 25% of the area's extents.
            const double latMargin = ( topLeft.getLatitude() - bottomRight.getLatitude() ) * 0.25;
            const double lonMargin = ( bottomRight.getLongitude() - topLeft.getLongitude() ) * 0.25;

            gem::RectangleGeographicArea paddedArea( { topLeft.getLatitude() + latMargin, topLeft.getLongitude() - lonMargin },
                                                     { bottomRight.getLatitude() - latMargin, bottomRight.getLongitude() + lonMargin } );

            mapView->centerOnArea( paddedArea );
            mapView->activateHighlight( results );
        }
        else
        {
            GEM_LOGE( "Area-limited search returned no results - nothing to highlight" );
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
