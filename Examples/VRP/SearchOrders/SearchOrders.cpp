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

        //initialize first customer
        gem::vrp::Customer c1;
        c1.setCoordinates(gem::Coordinates(47.016075, -0.849623));
        c1.setAlias("c1");
        c1.setPhoneNumber("+12312312");
        c1.setEmail("c1@yahoo.com");

        //add first customer to database
        int ret = serv.addCustomer(&listener, c1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //initialize second customer
        gem::vrp::Customer c2(gem::Coordinates(45.212821, 3.166858));
        c2.setAlias("c2");
        c2.setPhoneNumber("+12312312");
        c2.setEmail("c2@yahoo.com");

        //add second customer to database
        ret = serv.addCustomer(&listener, c2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //initialize first order
        gem::vrp::Order order1(c1);
        order1.setFirstName("ExampleOrder1");
        order1.setNumberOfPackages(5);
        order1.setServiceTime(600);
        order1.setType(gem::vrp::EOrderType::OT_PickUp);

        //add first order to database
        ret = serv.addOrder(&listener, order1, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //initialize second order
        gem::vrp::Order order2(c2);
        order2.setFirstName("ExampleOrder2");
        order2.setNumberOfPackages(4);
        order2.setType(gem::vrp::EOrderType::OT_Delivery);

        //add second order to database
        ret = serv.addOrder(&listener, order2, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //get all orders from database that contains string "Example"
        gem::vrp::OrderList allOrders;
        ret = serv.getOrders(&listener, allOrders, "Example");
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && ret == gem::KNoError)
            std::cout << allOrders.size() << " orders returned successfully" << std::endl;
        else
            std::cout << "No order returned" << std::endl;
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
