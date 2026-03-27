// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>

#include <iostream>
#include <iomanip>

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


    {
        ProgressListener listener;
        gem::vrp::Service serv;

        gem::vrp::FuelPricesList fuelPrices;
        int res = serv.getFuelPrices(&listener, fuelPrices);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 10000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
        {
            std::cout << "Prices returned successfully" << std::endl;
            for (auto fuelPrice : fuelPrices)
            {
                std::cout << "Time of addition: " << fuelPrice.additionTime.getYear() << "\\" << std::setfill('0') << std::setw(2) << fuelPrice.additionTime.getMonth() << "\\" << std::setw(2) << fuelPrice.additionTime.getDay() << " " << std::setw(2) << fuelPrice.additionTime.getHour() << ":" << std::setw(2) << fuelPrice.additionTime.getMinute() << std::endl;
                for (int i = 0; i < fuelPrice.fuelPrices.size(); i++)
                {
                    const gem::vrp::FuelPricePair fp = fuelPrice.fuelPrices[i];
                    std::cout << std::fixed << std::setprecision(2);
                    if (fp.fuel == gem::vrp::FT_DieselStandard)
                        std::cout << "Diesel standard: " << fp.price << std::endl;
                    else
                        if (fp.fuel == gem::vrp::FT_DieselPremium)
                            std::cout << "Diesel premium: " << fp.price << std::endl;
                        else
                            if (fp.fuel == gem::vrp::FT_GasolineStandard)
                                std::cout << "Gasoline standard: " << fp.price << std::endl;
                            else
                                if (fp.fuel == gem::vrp::FT_GasolinePremium)
                                    std::cout << "Gasoline premium: " << fp.price << std::endl;
                                else
                                    if (fp.fuel == gem::vrp::FT_Electric)
                                        std::cout << "Current: " << fp.price << std::endl;
                                    else
                                        if (fp.fuel == gem::vrp::FT_LPG)
                                            std::cout << "LPG: " << fp.price << std::endl;
                }
            }
        }
        else
            std::cout << "Prices couldn't be returned" << std::endl;
    }

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
