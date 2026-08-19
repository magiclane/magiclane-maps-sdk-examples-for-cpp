// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>

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
        gem::vrp::FuelPricePair dieselStPrice;
        dieselStPrice.fuel = gem::vrp::EFuelType::FT_DieselStandard;
        dieselStPrice.price = 1.08f;
        gem::vrp::FuelPricePair dieselPrPrice;
        dieselPrPrice.fuel = gem::vrp::EFuelType::FT_DieselPremium;
        dieselPrPrice.price = 1.15f;

        gem::vrp::FuelPricePairList pricesPairList;
        pricesPairList.toStd().push_back( dieselStPrice );
        pricesPairList.toStd().push_back( dieselPrPrice );

        gem::vrp::FuelPrices prices;
        prices.fuelPrices = pricesPairList;

        ProgressListener listener;
        gem::vrp::Service serv;

        int res = serv.addFuelPrices( &listener, prices );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 10000 );

        if( listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError )
            std::cout << "Fuel prices set successfully" << std::endl;
        else
            std::cout << "Fuel prices couldn't be set" << std::endl;
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
