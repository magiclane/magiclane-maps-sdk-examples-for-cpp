// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>

#include <iostream>

int main(int argc, char** argv)
{
    // Get new project API token from:
    // https://developer.magiclane.com/api/projects
    std::string projectApiToken = "";

#if defined(API_TOKEN)
    projectApiToken = std::string(API_TOKEN);
#else
    auto value = std::getenv("GEM_TOKEN");
    if (value != nullptr)
        projectApiToken = value;
#endif

    // Sdk objects can be created & used below this line
    Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();


    {
        ProgressListener listener;
        gem::vrp::Service serv;

        //initialize first vehicle
        gem::vrp::Vehicle vehicle1;
        vehicle1.setName("ExampleVehicle1");
        vehicle1.setType(gem::vrp::EVehicleType::VT_Car);
        vehicle1.setStatus(gem::vrp::EVehicleStatus::VS_Available);
        vehicle1.setManufacturer("Kia");
        vehicle1.setModel("Ceed");
        vehicle1.setFuelType(gem::vrp::EFuelType::FT_GasolinePremium);
        vehicle1.setConsumption(6.5);
        vehicle1.setLicensePlate("BV01ASD");
        vehicle1.setMaxWeight(100);
        vehicle1.setMaxCube(2.1);

        //add first vehicle to database
        int res = serv.addVehicle(&listener, vehicle1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        // initialize second vehicle
        gem::vrp::Vehicle vehicle2;
        vehicle2.setName("ExampleVehicle2");
        vehicle2.setType(gem::vrp::EVehicleType::VT_Car);
        vehicle2.setStatus(gem::vrp::EVehicleStatus::VS_Available);
        vehicle2.setManufacturer("Kia");
        vehicle2.setModel("Ceed");
        vehicle2.setFuelType(gem::vrp::EFuelType::FT_GasolinePremium);
        vehicle2.setConsumption(6.5);
        vehicle2.setLicensePlate("BV02ASD");
        vehicle2.setMaxWeight(100);
        vehicle2.setMaxCube(2.1);

        //add second vehicle to database
        res = serv.addVehicle(&listener, vehicle2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //get all vehicles from database that contains string "Example"
        gem::vrp::VehicleList allVehicles;
        res = serv.getVehicles(&listener, allVehicles, "Example");
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
            std::cout << allVehicles.size() << " vehicles returned successfully" << std::endl;
        else
            std::cout << "No vehicle returned" << std::endl;
    }

    return 0;
}

#if ( defined(_WIN32) || defined(_WIN64) ) && !defined(__MINGW32__) && !defined(__MINGW64__)

int WINAPI WinMain(HINSTANCE hInstance, // Instance
    HINSTANCE hPrevInstance, // Previous Instance
    LPSTR lpCmdLine, // Command Line Parameters
    int nCmdShow)
{
    main(0, nullptr);

    return 0;
}

#endif
