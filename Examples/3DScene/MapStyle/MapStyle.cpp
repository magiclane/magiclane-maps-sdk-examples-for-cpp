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
	if ( value != nullptr )
		projectApiToken = value;
#endif

	// Sdk objects can be created & used below this line
	Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


	// Create an interactive map view
	CTouchEventListener pTouchEventListener;

	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce( session.produceOpenGLContext( Environment::WindowFrameworks::Available, "MapStyle", &pTouchEventListener ) );
	if ( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    ProgressListener listener;

    auto searchAndApply = [&]()
    {
        auto styles = gem::ContentStore().getStoreContentList(gem::EContentType::CT_ViewStyleLowRes).first;

        gem::ContentStoreItem nightStyle;

        //find first night style
        for (auto style : styles)
        {
            auto bckColor = style.getContentParameters().findParameter("Background-Color");
            if (bckColor && !gem::Rgba(bckColor.getValue<int>()).isLight())
            {
                //night style - download if needed
                if (style.getStatus() == gem::EContentStoreItemStatus::CIS_Unavailable)
                {
                    listener.Reset();
                    style.asyncDownload(&listener);

                    WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), INT_MAX);
                }

                nightStyle = style;
                break;
            }
        }

        if (nightStyle && nightStyle.isCompleted())
            return GEM_TEST_NOEXCEPT(mapView->preferences().setMapStyle(nightStyle)) == gem::KNoError;
        else
            return false;
    };

    //try with local list
    if (!searchAndApply())
    {
        //get available store styles
        gem::ContentStore().asyncGetStoreContentList(gem::EContentType::CT_ViewStyleLowRes, &listener);

        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), INT_MAX);

        //retry
        searchAndApply();
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
