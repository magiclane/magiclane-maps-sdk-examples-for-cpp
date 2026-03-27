// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_ExternalInfo.h>

using namespace gem;

class WikipediaProgressListenerImpl : public gem::IProgressListener
{

public:
    WikipediaProgressListenerImpl() { Reset(); }

    void notifyStart(bool) override
    {
        m_bStarted = true;
        m_bFinished = false;
        m_hint = "";
        m_error = gem::KNoError;
    }
    void notifyProgress(int value) override
    {
        m_bProgress = true;
        m_progressValue = value;
    }
    void notifyComplete(int reason, gem::String hint = gem::String()) override
    {
        GEM_INFO_LOG( "Wikipedia fetch completed with reason: %d, hint: %s", reason, hint.constData() );

        String title = m_externalInfo->getWikiPageTitle();
        String description = m_externalInfo->getWikiPageDescription();

        GEM_INFO_LOG( "Wikipedia Page Title: %s", title );
        GEM_INFO_LOG( "Wikipedia Page Description: %s", description );


        m_bFinished = true;
        m_error = reason;
        m_hint = hint;
    }
    void notifyStatusChanged(int value) override
    {
        m_status = value;
    }

    int GetStatus() const
    {
        return m_status;
    }
    int GetProgressValue() const
    {
        return m_progressValue;
    }
    bool IsStarted()
    {
        return m_bStarted;
    }
    bool HasProgress()
    {
        return m_bProgress;
    }
    bool IsFinished()
    {
        return m_bFinished;
    }
    int GetError()
    {
        return m_error;
    }
    gem::String GetHint()
    {
        return m_hint;
    }

    void Reset()
    {
        m_bFinished = m_bStarted = m_bProgress = false;
        m_error = gem::KNoError;
        m_progressValue = 0;
        m_status = 0;
        m_hint.clear();
        m_externalInfo = ExternalInfo::produce();
    }

    void FetchWikipediaInfo( const Landmark& landmark )
    {
        if( m_externalInfo->hasWikiInfo( landmark ) )
        {
            m_externalInfo->requestWikiInfo( landmark, this );
        }
        else
        {
            GEM_INFO_LOG( "The selected landmark does not have Wikipedia info." );
            m_bFinished = true;
        }
    }

private:
    bool m_bFinished, m_bStarted, m_bProgress;
    int m_error;
    int m_progressValue;
    int m_status;
    gem::String m_hint;
    StrongPointer<ExternalInfo> m_externalInfo;
};

class MyMapViewListenerImpl : public IMapViewListener
{
public:

    void setMapView(StrongPointer<MapView> mv)
    {
        mapView = mv;
    }

    void onLongDown(const Xy &pos) override
    {
        GEM_INFO_LOG( "Long down at position: (%d, %d)", pos.x, pos.y );

        if(wikipediaProgressListenerImpl->IsStarted() && !wikipediaProgressListenerImpl->IsFinished())
        {
            GEM_INFO_LOG( "Fetch already in progress, please wait..." );
            return;
        }

        // Set cursor position on the screen.
        mapView->setCursorScreenPosition(pos);

        // Wait for the map view to fully load the tiles

        // Get the landmarks at that position.
        auto items = mapView->cursorSelectionLandmarks();

        // Fetch Wikipedia info for the first landmark found.
        for (auto item : items) 
        {
            wikipediaProgressListenerImpl->FetchWikipediaInfo( item );
            break;
        }

    }
private:
    StrongPointer<MapView> mapView;
    StrongPointer<WikipediaProgressListenerImpl> wikipediaProgressListenerImpl = StrongPointerFactory<WikipediaProgressListenerImpl>();;
};

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
    auto myMapViewListenerImpl = gem::StrongPointerFactory<MyMapViewListenerImpl>();
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "LocationWikipedia", &pTouchEventListener), myMapViewListenerImpl); 
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    myMapViewListenerImpl->setMapView( mapView );

	mapView->centerOnCoordinates({ 51.51944, -0.12675 }, 90);

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
