// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_VRP.h>
#include <API/GEM_MapView.h>

#include <iostream>

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
        gem::Coordinates coords(48.853543, 2.265137);

        gem::AddressInfo address;
        address.setField("France", gem::EAddressField::Country);
        address.setField("Île-de-France", gem::EAddressField::County);
        address.setField("Paris", gem::EAddressField::City);
        address.setField("75016", gem::EAddressField::PostalCode);
        address.setField("Rue du Dr Blanche", gem::EAddressField::StreetName);
        address.setField("38", gem::EAddressField::StreetNumber);

        gem::vrp::Customer customer(coords);
        customer.setAlias("Auteuil");
        customer.setAddress(address);
        customer.setEmail("example@example.com");
        customer.setPhoneNumber("+1234567890");

        ProgressListener listener;
        gem::vrp::Service serv;
        int res = serv.addCustomer(&listener, customer);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);

        if (listener.IsFinished() && listener.GetError() == gem::KNoError && res == gem::KNoError)
            std::cout << "Customer added successfully and has the id = " << customer.getId() << std::endl;
        else
            std::cout << "Customer couldn't be added " << std::endl;

        gem::Landmark landmark;
        landmark.setName(customer.getAlias());
        landmark.setCoordinates(customer.getCoordinates());
        landmark.setImage(gem::Icon::Core::BlueBall);

        gem::LandmarkList lmks;
        lmks.push_back(landmark);

        MapViewListenerImpl mapListener;
        auto oglContext = session.produceOpenGLContext(Environment::WindowFrameworks::Available, "AddCustomer");
        gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(oglContext, &mapListener);
        
        mapView->activateHighlight(lmks);
        mapView->centerOnCoordinates(customer.getCoordinates(), 50);

        auto ret = WAIT_UNTIL(std::bind(&MapViewListenerImpl::IsFinished, &mapListener), 15000);

        WAIT_UNTIL_WINDOW_CLOSE();
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
