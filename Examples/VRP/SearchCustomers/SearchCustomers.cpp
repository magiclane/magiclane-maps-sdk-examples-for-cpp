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

        //initialize first address
        gem::AddressInfo address1;
        address1.setField("France", gem::EAddressField::Country);
        address1.setField("Île-de-France", gem::EAddressField::County);
        address1.setField("Paris", gem::EAddressField::City);
        address1.setField("75016", gem::EAddressField::PostalCode);
        address1.setField("Rue du Dr Blanche", gem::EAddressField::StreetName);
        address1.setField("38", gem::EAddressField::StreetNumber);

        //initialize second address
        gem::AddressInfo address2;
        address2.setField("France", gem::EAddressField::Country);
        address2.setField("Île-de-France", gem::EAddressField::County);
        address2.setField("Paris", gem::EAddressField::City);
        address2.setField("75016", gem::EAddressField::PostalCode);
        address2.setField("Rue Henri Heine", gem::EAddressField::StreetName);
        address2.setField("22", gem::EAddressField::StreetNumber);

        //initialize first customer
        gem::vrp::Customer customer1(gem::Coordinates(48.853543, 2.265137));
        customer1.setAlias("ExampleAlias1");
        customer1.setAddress(address1);
        customer1.setEmail("example1@example.com");
        customer1.setPhoneNumber("+1234567890");

        //add first customer to database
        int res = serv.addCustomer(&listener, customer1);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //initialize second customer
        gem::vrp::Customer customer2(gem::Coordinates(48.852431, 2.265413));
        customer2.setAlias("ExampleAlias2");
        customer2.setAddress(address2);
        customer2.setEmail("example2@example.com");
        customer2.setPhoneNumber("+1234567890");

        //add second customer to database
        res = serv.addCustomer(&listener, customer2);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        //get all customers from database that contains string "Alias1"
        gem::vrp::CustomerList allCustomers;
        res = serv.getCustomers(&listener, allCustomers, "Alias1");
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 20000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
            std::cout << allCustomers.size() << " customers returned successfully" << std::endl;
        else
            std::cout << "No customer returned" << std::endl;
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
