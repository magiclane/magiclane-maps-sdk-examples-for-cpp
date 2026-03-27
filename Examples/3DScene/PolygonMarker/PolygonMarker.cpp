// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_Markers.h>
#include <API/GEM_ContentStore.h>

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
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "PolygonMarker", &pTouchEventListener)); 
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

	{
		auto col = gem::MarkerCollection(gem::EMarkerType::MT_Polygon, "test");

		col.add(gem::Marker({
			{ 37.78301592799968, -122.44509746977026 },
			{ 37.744870145184954, -122.47291375685005 },
			{ 37.73182234501792, -122.39309744523473 }
		}));
		col.add(gem::Marker({
			{ 37.7727019264254, -122.42707148907742 },
			{ 37.76671282078619, -122.39085098046263 },
			{ 37.75083649188404, -122.41110088290034 }
		}));

		gem::MarkerCollectionRenderSettings markerCollDisplaySettings;
		markerCollDisplaySettings.polygonFillColor = gem::Rgba(0, 100, 100, 100);

		mapView->preferences().markers().add(col, markerCollDisplaySettings);
		mapView->centerOnArea(col.getArea(), 55);
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
