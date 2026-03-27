// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>
#include <API/GEM_NavigationListener.h>
#include <API/GEM_NavigationService.h>
#include <API/Extensions/GEM_Utils.h>
#include <API/GEM_OperationScheduler.h>
#include <API/GEM_SenseDataSource.h>
#include <API/GEM_SdkSettings.h>

#ifdef _MSC_VER
	#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

namespace
{
    constexpr const char16_t kWaypointsName[] = u"Waypoints";
    constexpr const char16_t kRouteName[] = u"Route";
    constexpr const char16_t kConnectionsName[] = u"Connections";
    constexpr const char16_t kInstructionName[] = u"Instruction";
}

class MyListener : public gem::INavigationListener, public gem::IPositionListener, public gem::IMapViewListener
{
public:
    MyListener( const gem::String& dataPath )
    {
        auto dest = gem::FileSystem().makePath( gem::FileSystem().removeLastComponent( dataPath ), u"tmp/" );
        gem::FileSystem().createFolder( dest, true );
        gem::FileSystem().uncompress( dataPath, dest );

        auto folder = gem::FileSystem().scan( dest, u"inst*.dat" );
        for( const auto& it : folder.files )
        {
            auto pos = it.path.findLast( '_' );
            if( pos != -1 )
            {
                m_instructions.emplace_back();
                m_instructions.back().stamp = it.path.toInt( pos + 1 );

                std::ifstream stream;
                stream.open( it.path.toStdString().c_str(), std::ios::binary );
                if( stream.good() )
                {
                    stream.seekg( 0, std::ios::end );
                    m_instructions.back().inst.reserve( int( stream.tellg() ) );
                    stream.seekg( 0, std::ios::beg );
                    stream.read( m_instructions.back().inst.getBytes<char>(), m_instructions.back().inst.size() );
                }
            }
        }

        folder = gem::FileSystem().scan( dest, u"route.dat" );
        if( !folder.files.empty() )
        {
            std::ifstream stream;
            stream.open( folder.files[0].path.toStdString().c_str(), std::ios::binary );
            if( stream.good() )
            {
                stream.seekg( 0, std::ios::end );
                m_route.reserve( int(stream.tellg()) );
                stream.seekg( 0, std::ios::beg );
                stream.read( m_route.getBytes<char>(), m_route.size() );
            }
        }
        folder = gem::FileSystem().scan( dest, u"*.nmea" );
        if( !folder.files.empty() )
        {
            gem::sense::DataSourcePtr dataSrc = gem::sense::DataSourceFactory::produceLog( folder.files[0].path );
            if( dataSrc )
            {
                dataSrc->start();
                gem::PositionService().setDataSource( dataSrc );
                m_startStamp = gem::Time::getUniversalTime().asInt();
            }
        }

        folder = gem::FileSystem().scan( dest, u"*.style" );
        if( !folder.files.empty() )
            m_stylePath = folder.files[0].path;
    }

private:
    //INavigationListener
    void onNavigationStarted() override
    {
        GEM_LOGI( "Simulation started" );
    }
    void onNavigationInstructionUpdated( const gem::NavigationInstruction &inst ) override
    {
        GEM_LOGI( "New instruction" );
    }
    void onWaypointReached( const gem::Landmark& lmk ) override
    {
        GEM_LOGI( "Intermediary destination reached" );
    }
    void onDestinationReached( const gem::Landmark& lmk ) override
    {
        GEM_LOGI( "Final destination reached" );
    }

    void onNavigationError( int error ) override
    {
        GEM_LOGI( "Nav error: %d", error );
    }
    
    void onRouteUpdated( const gem::Route& route ) override
    {
        GEM_LOGI( "Route updated" );
    }

    void onNavigationSound( gem::ISound const& sound ) override
    {
        GEM_LOGI( "Nav sound play request" );
    }

    bool canPlayNavigationSound() override
    {
        return true;
    }

    void onBetterRouteDetected( const gem::Route& route, int travelTime, int delay, int timeGain ) override
    {
    }

    //IPositionListener
    void onNewPosition( gem::sense::PositionPtr pos )
    {
        if( !m_instructions.empty() )
        {
            if( m_instructions[0].stamp < (gem::Time::getUniversalTime().asInt() - m_startStamp) )
            {

                loadInstruction( m_instructions[0].inst );
                m_instructions.erase( m_instructions.begin() );
            }
        }
    }

    //IMapViewListener
    void onViewRenderExtensions() override
    {
        if( m_AGTexture != -1 )
            m_view->extensions().renderTexture( m_AGTexture, gem::Rect( 10, 10, 110, 110 ) );
    }
    void onReady( gem::StrongPointer<gem::MapView> view ) override
    {
        m_view = view;
        m_view->preferences().setMapStyleByPath( m_stylePath );
        loadRoute( true );
    }

    void loadRoute( bool bInitialCalc )
    {
        //load route
        gem::RectangleGeographicArea area;

        //remove existing
        m_view->preferences().markers().remove( m_view->preferences().markers().indexOf( kWaypointsName ) );
        m_view->preferences().markers().remove( m_view->preferences().markers().indexOf( kRouteName ) );
        m_view->preferences().markers().remove( m_view->preferences().markers().indexOf( kConnectionsName ) );

        auto collector = [&]( gem::MarkerCollection coll, int type )
        {
            coll.setName( [&]()
                {
                    switch( type )
                    {
                    case gem::GMT_Route:
                        return kRouteName;
                    case gem::GMT_Connections:
                        return kConnectionsName;
                    default:
                        return kWaypointsName;
                    }
                }( ) );
            m_view->preferences().markers().add( coll, [&]()
                {
                    switch( type )
                    {
                    case gem::GMT_Route:
                        return gem::MarkerRenderSettings().setPolylineInnerColor( gem::Rgba( 200, 200, 255, 255 ) ).setPolylineInnerSize( 8 ).
                            setPolylineOuterColor( gem::Rgba( 50, 50, 200, 255 ) ).setPolylineOuterSize( 1 );
                    case gem::GMT_Connections:
                        return gem::MarkerRenderSettings().setPolylineInnerColor( gem::Rgba( 100, 100, 100, 255 ) ).setPolylineInnerSize( 5 ).
                            setPolylineOuterColor( gem::Rgba( 0, 0, 0, 255 ) ).setPolylineOuterSize( 1 );
                    default:
                        return gem::MarkerRenderSettings();
                    }
                }( ) );
            area.setUnion( coll.getArea() );
        };

        auto data = gem::DataArchiver().inflate( m_route );
        data = gem::DataArchiver().unpackCRC( data ).first;
        m_view->extensions().getMarkersFromRouteGeometry( collector, data );

        if( bInitialCalc )
        {
            m_view->centerOnArea( area );
        }
        else
        {
            m_view->preferences().markers().remove( m_view->preferences().markers().indexOf( kInstructionName ) );
        }

        m_route.reset();

        gem::OperationScheduler().timeoutOperation( 1000, [&]()
        {
            m_view->startFollowingPosition();
        }, gem::ProgressListener(), true );
    }

    void loadInstruction( const gem::DataBuffer& instData )
    {
        auto data = gem::DataArchiver().inflate( instData );
        data = gem::DataArchiver().unpackCRC( data ).first;
        auto inst = m_view->extensions().getMarkersFromRouteInstructionGeometry( data );

        inst.first.setName( kInstructionName );

        //delete existing
        m_view->preferences().markers().remove( m_view->preferences().markers().indexOf( kInstructionName ) );

        m_view->preferences().markers().add( inst.first, gem::MarkerRenderSettings().setPolylineInnerColor( gem::Rgba::black() ).setPolylineInnerSize( 10 ).
            setPolylineOuterColor( gem::Rgba::white() ).setPolylineOuterSize( 1 ) );

        auto bmp = gem::IBitmap::produce( gem::Size( 100, 100 ), gem::EImagePixelFormat::ARGB_8888 );
        inst.second.render( *bmp, gem::AbstractGeometryImageRenderSettings( gem::Rgba( 255, 255, 255, 255 ), gem::Rgba( 0, 0, 0, 255 ),
            gem::Rgba( 180, 180, 180, 255 ), gem::Rgba( 180, 180, 180, 255 ) ) );
        if( m_AGTexture != -1 )
            m_view->extensions().deleteTexture( m_AGTexture );
        m_AGTexture = m_view->extensions().createTexture( *bmp );
    }

private:
    struct TInstruction
    {
        gem::DataBuffer inst;
        int stamp;
    };
    std::vector<TInstruction> m_instructions;
    gem::DataBuffer m_route;
    gem::StrongPointer<gem::MapView> m_view;
    gem::LargeInteger m_startStamp = 0;
    gem::String m_stylePath;
    int m_AGTexture = -1;

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


    gem::SdkSettings().setAllowConnection( false, gem::OffboardListener() );

    auto sdkExamplesPath = Environment::GetInstance().GetSDKExamplesPath();
    auto srcZipPath = gem::FileSystem().makePath(sdkExamplesPath.c_str(), u"Examples", u"RoutesAndNavigation", u"PlayRoute", u"data.zip");

    MyListener listener( gem::String( argc > 1 ? argv[1] : srcZipPath) );
    
    // Create a map view
    CTouchEventListener pTouchEventListener;
    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::Available, "PlayRoute", &pTouchEventListener), &listener); 
    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
        return 0;
    }

    gem::PositionService().addListener( &listener );

    WAIT_UNTIL_WINDOW_CLOSE();
    return 0;
}
