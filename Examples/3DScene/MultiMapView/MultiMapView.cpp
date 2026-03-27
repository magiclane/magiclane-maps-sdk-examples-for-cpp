// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
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

    // Create a screen which can contain multiple interactive map views
    CTouchEventListener pTouchEventListener;
    gem::StrongPointer<gem::Screen> screen = gem::Screen::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "MultiMapView", &pTouchEventListener));

	// Map view in left viewport half
	gem::StrongPointer<gem::MapView> mapView1 = gem::MapView::produce(screen, gem::RectF(0.f, 0.f, 0.5f, 1.0f));
	// Map view in lower right viewport
	gem::StrongPointer<gem::MapView> mapView2 = gem::MapView::produce(screen, gem::RectF(0.5f, 0.f, 1.0f, 0.5f));
	// Map view in upper right viewport
	gem::StrongPointer<gem::MapView> mapView3 = gem::MapView::produce(screen, gem::RectF(0.5f, 0.5f, 1.f, 1.f));

    mapView2->centerOnCoordinates({ 48.8613, 2.33387 }, 65);
    mapView3->centerOnCoordinates({ 48.5213, 7.7377 }, 65);

	// Set a different map style in each mapView to differentiate them
	auto styleList = gem::ContentStore().getLocalContentList(gem::EContentType::CT_ViewStyleLowRes);

    for(int i = 0; i < styleList.size(); ++i)
    {
        if(styleList[i].getName().toStdString() == "Basic 1")
        {
            mapView1->preferences().setMapStyle(styleList[i]);
        }
        else if(styleList[i].getName().toStdString() == "Basic 2")
        {
            mapView2->preferences().setMapStyle(styleList[i]);
        }
        else if(styleList[i].getName().toStdString() == "Basic 3")
        {
            mapView3->preferences().setMapStyle(styleList[i]);
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
