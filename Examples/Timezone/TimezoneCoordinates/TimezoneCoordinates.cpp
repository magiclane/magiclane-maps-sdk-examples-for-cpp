// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_Timezone.h>
#include <API/GEM_Debug.h>

#include <iostream>

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

    {
        ProgressListener listener;
        gem::TimezoneService serv;
        gem::TimezoneResult timezoneResult;

        // The time for which the offsets are computed: now. Use the SDK time (gem::Time
        // is in MILLISECONDS)
        auto currentTime = gem::Time::getUniversalTime();

        // Bran Castle - coordinates given as lat,lon in degrees
        auto res = serv.getTimezoneInfo( timezoneResult, gem::Coordinates( 45.514928, 25.367094 ), currentTime, &listener );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 100000 );

        if( listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError )
        {
            GEM_LOGI( "Success getting timezone info! id=%s, utcOffset=%ds, dstOffset=%ds, currentOffset=%ds", timezoneResult.getTimezoneId().toStdString().c_str(),
                      timezoneResult.getUtcOffset(), timezoneResult.getDstOffset(), timezoneResult.getOffset() );
            std::cout << "Success getting timezone info! id=" << timezoneResult.getTimezoneId().toStdString() << ", utcOffset=" << timezoneResult.getUtcOffset()
                      << "s, dstOffset=" << timezoneResult.getDstOffset() << "s, currentOffset=" << timezoneResult.getOffset() << "s" << std::endl;
        }
        else
        {
            GEM_LOGE( "Failed getting timezone info! res=%d, listenerError=%d", res, listener.GetError() );
            std::cout << "Failed getting timezone info! res=" << res << ", listenerError=" << listener.GetError() << std::endl;
        }
    }

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
