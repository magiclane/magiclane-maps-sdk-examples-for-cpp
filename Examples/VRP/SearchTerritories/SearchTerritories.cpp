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

        //initialize first territory
        gem::vrp::Territory circleTerritory;
        circleTerritory.setName("Circle territory");
        circleTerritory.setType(gem::vrp::ETerritoryType::TT_Circle);
        circleTerritory.setColor(gem::Rgba(255, 42, 0, 1));

        gem::CoordinatesList data1;
        gem::Coordinates center = { 46.603125, 0.354550 };
        gem::Coordinates radius = { 46.593494, 0.376579 };
        data1.push_back(center); data1.push_back(radius);
        circleTerritory.setData(data1);

        //add first territory to database
        int res = serv.addTerritory(&listener, circleTerritory);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        
        //initialize second territory
        gem::vrp::Territory poligonTerritory;
        poligonTerritory.setName("Polygon territory");
        poligonTerritory.setType(gem::vrp::ETerritoryType::TT_Polygon);
        poligonTerritory.setColor(gem::Rgba(255, 42, 0, 50));

        gem::CoordinatesList data2;
        gem::Coordinates point1 = { 46.585964, 0.346915 };
        gem::Coordinates point2 = { 46.593774, 0.340877 };
        gem::Coordinates point3 = { 46.587993, 0.357528 };
        gem::Coordinates point4 = { 46.580442, 0.344825 };
        gem::Coordinates point5 = { 46.587344, 0.339847 };
        data2.push_back(point1); data2.push_back(point2);
        data2.push_back(point3); data2.push_back(point4);
        data2.push_back(point5);
        poligonTerritory.setData(data2);
        
        //add second territory to database
        res = serv.addTerritory(&listener, poligonTerritory);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //get all vehicles from database that contains string "territory"
        gem::vrp::TerritoryList allTerritories;
        res = serv.getTerritories(&listener, allTerritories, "territory");
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
            std::cout << allTerritories.size() << " territories returned successfully" << std::endl;
        else
            std::cout << "No territory returned" << std::endl;
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
