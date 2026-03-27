// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_RoutingService.h>

#include <API/GEM_VRP.h>
#include <API/GEM_MapView.h>
#include <API/GEM_Markers.h>

#include <imgui.h>
#include <random>

gem::vrp::Customer CreateCustomer(const std::string& alias,const gem::Coordinates& coords)
{
    gem::vrp::Customer customer;
    customer.setCoordinates(coords);
    customer.setAlias(alias);
    customer.setPhoneNumber("+12312312");
    customer.setEmail(alias + "@yahoo.com");
    return customer;
}

gem::vrp::Vehicle CreateVehicle( const std::string& name, gem::vrp::EVehicleType type, gem::vrp::EVehicleStatus status, const std::string& manufacturer, const std::string& model, gem::vrp::EFuelType fuelType, double consumption, const std::string& licensePlate, double maxWeight, double maxCube, int startTime, int endTime) {
    gem::vrp::Vehicle vehicle;
    vehicle.setName(name);
    vehicle.setType(type);
    vehicle.setStatus(status);
    vehicle.setManufacturer(manufacturer);
    vehicle.setModel(model);
    vehicle.setFuelType(fuelType);
    vehicle.setConsumption(consumption);
    vehicle.setLicensePlate(licensePlate);
    vehicle.setMaxWeight(maxWeight);
    vehicle.setMaxCube(maxCube);
    vehicle.setStartTime(startTime);
    vehicle.setEndTime(endTime);
    return vehicle;
}

gem::vrp::Order CreateOrder(const gem::vrp::Customer& customer, int numberOfPackages, double weight, double cube, double revenue, int serviceTime, std::pair<int, int> timeWindow, gem::vrp::EOrderType type)
{
    gem::vrp::Order order(customer);
    order.setNumberOfPackages(numberOfPackages);
    order.setWeight(weight);
    order.setCube(cube);
    order.setServiceTime(serviceTime);
    order.setTimeWindow(timeWindow);
    order.setRevenue(revenue);
    order.setType(type);
    return order;
}

gem::vrp::Optimization SetUpOptimization()
{
    ProgressListener listener;
    gem::vrp::Service serv;
    
    gem::vrp::CustomerList customerList;
    const gem::CoordinatesList coordinates = { gem::Coordinates(48.234270, -2.133208), gem::Coordinates(45.854137, 2.853998), gem::Coordinates(46.199373, 0.069986), gem::Coordinates(48.052503, 0.119726),
    gem::Coordinates(44.346051, 4.694878), gem::Coordinates(44.464582, 2.455020), gem::Coordinates(48.656644, 5.907131), gem::Coordinates(49.161539, 0.500580),
    gem::Coordinates(47.702421, 3.384226), gem::Coordinates(47.198274, 4.630011), gem::Coordinates(49.655296, 2.243181), gem::Coordinates(50.719729, 2.160877) };

    for (size_t index = 0; index < 12; index++)
    {
        gem::vrp::Customer customer = CreateCustomer("c" + std::to_string(index), coordinates[index]);
        int ret = serv.addCustomer(&listener, customer);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        customerList.push_back(customer);
    }

    const std::vector<uint8_t> numberOfPackages = { 5, 4, 8, 0, 8, 11, 4, 12, 7, 12, 9, 5 };
    const std::vector<float> weights = { 15.7, 15.5, 5.5, 0.0, 5.1, 6.5, 1.5, 6.1, 2.5, 0.7, 4.3, 4.1 };
    const std::vector<float> cubes = { 0.2, 0.9, 0.3, 0.0, 0.2, 0.1, 0.5, 0.4, 0.3, 0.5, 0.6, 0.4 };
    const std::vector<float> revenues = { 0.0, 0.0, 0.0, 0.0, 0.0, 25.0, 0.0, 75.0, 110.0, 0.0, 0.0, 0.0 };
    const std::vector<uint16_t> serviceTimes = { 600, 0, 600, 0, 600, 900, 500, 750, 800, 1000, 850, 600 };
    const std::vector<std::pair<int, int>> timeWindows = { {420, 420},{420, 420},{660, 1366},{753, 420},{1086, 331},{1053, 420},{821, 226},{1086, 278},{750, 1120},{816, 1408},{883, 80},{696, 1236}};

    const std::vector<gem::vrp::EOrderType> orderTypes = {gem::vrp::EOrderType::OT_PickUp,gem::vrp::EOrderType::OT_PickUp,gem::vrp::EOrderType::OT_Delivery,gem::vrp::EOrderType::OT_Delivery,gem::vrp::EOrderType::OT_PickUp,gem::vrp::EOrderType::OT_Delivery,
    gem::vrp::EOrderType::OT_PickUp,gem::vrp::EOrderType::OT_Delivery,gem::vrp::EOrderType::OT_Delivery,gem::vrp::EOrderType::OT_PickUp,gem::vrp::EOrderType::OT_PickUp,gem::vrp::EOrderType::OT_PickUp};

    gem::vrp::OrderList orderList;
    for (size_t index = 0; index < customerList.size()-1; index++)
    {
        gem::vrp::Order order = CreateOrder(customerList[index], numberOfPackages[index], weights[index], cubes[index], revenues[index], serviceTimes[index], timeWindows[index], orderTypes[index]);
        int ret = serv.addOrder(&listener, order, false);
        WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
        orderList.push_back(order);
    }
    
    gem::vrp::Departure departure1;
    departure1.setAlias("Depot 1");
    departure1.setCoordinates(gem::Coordinates(48.618893, -1.353635));
    gem::vrp::Departure departure2;
    departure2.setAlias("Depot 2");
    departure2.setCoordinates(gem::Coordinates(46.213984, 1.693113));

    gem::vrp::Destination destination;
    destination.setAlias("Destination");
    destination.setCoordinates(gem::Coordinates(47.617484, 1.152466));

    gem::vrp::VehicleList vehicles;

    gem::vrp::Vehicle vehicle1 = CreateVehicle("Vehicle 1", gem::vrp::EVehicleType::VT_Car, gem::vrp::EVehicleStatus::VS_Available, "Volkswagen", "Transporter", gem::vrp::EFuelType::FT_GasolinePremium, 8.5, "BV01MGL", 300, 15, 400, 1860);
    int res = serv.addVehicle(&listener, vehicle1);
    WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
    vehicles.push_back(vehicle1);
    gem::vrp::Vehicle vehicle2 = CreateVehicle("Vehicle 2", gem::vrp::EVehicleType::VT_Car, gem::vrp::EVehicleStatus::VS_Available, "Volkswagen", "Transporter", gem::vrp::EFuelType::FT_GasolinePremium, 8.5, "BV02MGL", 300, 15, 480, 2520);
    res = serv.addVehicle(&listener, vehicle2);
    WAIT_UNTIL(std::bind(&ProgressListener::IsFinished, &listener), 5000);
    vehicles.push_back(vehicle2);

    gem::vrp::VehicleConstraintsList vehConstraintsList;
    gem::vrp::VehicleConstraints vehConstr1;
    vehConstr1.setMaxNumberOfPackages(100);
    vehConstr1.setMaxRevenue(2000);
    vehConstr1.setStartDate(gem::Time(2020, 8, 7)); // August 7, 2020 
    vehConstr1.setMinNumberOfOrders(1);
    vehConstr1.setMaxNumberOfOrders(50);
    vehConstr1.setMinDistance(1);
    vehConstr1.setMaxDistance(19000);
    vehConstraintsList.push_back(vehConstr1);

    gem::vrp::OrdersSequenceMap ordersSequence;
    gem::LargeIntListList fixedSequence = gem::LargeIntListList{ gem::LargeIntList{orderList[2].getId(), orderList[8].getId(), orderList[6].getId()} };
    ordersSequence.insert(std::make_pair(gem::vrp::EOrdersSequenceOption::OSO_InFixedSequence, fixedSequence));

    gem::vrp::ConfigurationParameters configParams;
    configParams.setName("France optimization");
    configParams.setIgnoreTimeWindow(false);
    configParams.setOptimizationCriterion(gem::vrp::EOptimizationCriterion::OC_Distance);
    configParams.setOptimizationQuality(gem::vrp::EOptimizationQuality::OQ_Optimized);
    configParams.setMaxWaitTime(18000); // A vehicle can wait maximum 5 hours between a order and the next one, in order to visit the next one within its time window
    configParams.setRouteType(gem::vrp::ERouteType::RT_CustomEnd);
    configParams.setRestrictions(gem::vrp::ERoadRestrictions::RR_None);
    configParams.setDistanceUnit(gem::vrp::EDistanceUnit::DU_Kilometers);
    configParams.setOrderSequenceOptions(ordersSequence);

    gem::vrp::Optimization optimization;
    optimization.setConfigurationParameters(configParams);
    optimization.setVehicles(vehicles);
    optimization.setDepartures({ departure1,departure2 });
    optimization.setDestinations({ destination }); // both vehicles will end their routes at the same destination
    optimization.setOrders(orderList);
    optimization.setVehiclesConstraints(vehConstraintsList);
    optimization.setMatrixBuildType(gem::vrp::EMatrixBuildType::MBT_Real);

    return optimization;
}

static bool g_showLoadingPopup = false;
static bool g_showErrorPopup = false;
static std::string g_message;

class UIController
{
public:
    UIController(gem::StrongPointer<gem::MapView> mapView)
        : m_mapView(mapView)
    {

    }

    void DrawRoutes(const gem::vrp::RouteList& routes)
    {
        m_mapView->deactivateAllHighlights();
        m_mapView->preferences().markers().clear();
    
        gem::CoordinatesList coords;
        gem::vrp::OrderList orderList;
        gem::vrp::DepartureList  departures;
        gem::vrp::DestinationList  destinations;
        for (const auto& route : routes)
        {
            gem::CoordinatesList  shape = route.getShape();

            auto shapeToDraw = gem::MarkerCollection(gem::EMarkerType::MT_Polyline, "shape");
            shapeToDraw.add(gem::Marker(shape));

            gem::MarkerCollectionDisplaySettings settings;
            settings.setPolylineInnerColor(GetColor());

            m_mapView->preferences().markers().add(shapeToDraw, settings);

            coords.insert(coords.end(), shape.begin(), shape.end());

            orderList.insert(orderList.end(), route.getOrders().begin(), route.getOrders().end());
            departures.push_back(route.getDeparture());
            if (route.getConfigurationParameters().getRouteType() != gem::vrp::RT_RoundRoute)
                destinations.push_back(route.getDestination());
        }

        DrawOrders(orderList, departures, destinations);
        CenterOnArea(coords, -1);
    }

    void DrawOrders(const gem::vrp::OrderList& orders, const gem::vrp::DepartureList& departures, const gem::vrp::DestinationList& destinations)
    {
        gem::LandmarkList landmarks;
        for (const auto& order : orders)
        {
            gem::Landmark landmark;
            landmark.setName(order.getAlias());
            landmark.setCoordinates(order.getCoordinates());
            landmark.setImage(gem::Icon::Core::CoreBase);
            landmarks.push_back(landmark);
        }
        
        for (const auto& departure: departures)
        {
            gem::Landmark landmark;
            landmark.setName(departure.getAlias());
            landmark.setCoordinates(departure.getCoordinates());
            landmark.setImage(gem::Icon::Core::Waypoint_Start);
            landmarks.push_back(landmark);
        }

        for (const auto& destination : destinations)
        {
            gem::Landmark landmark;
            landmark.setName(destination.getAlias());
            landmark.setCoordinates(destination.getCoordinates());
            landmark.setImage(gem::Icon::Core::Waypoint_Finish);
            landmarks.push_back(landmark);
        }

        if (m_mapView != nullptr && !landmarks.empty())
            m_mapView->activateHighlight(landmarks, gem::HO_ShowLandmark | gem::HO_NoFading | gem::HO_Overlap);
    }

    void CenterOnArea(const gem::CoordinatesList& coordinates, int zoomLevel = 30)
    {
        if (coordinates.empty())
        {
            gem::CoordinatesList cursorCoordinates = gem::CoordinatesList{ m_mapView->getCursorWgsPosition() };
            gem::PolygonGeographicArea polyArea(cursorCoordinates);
            m_mapView->centerOnArea(polyArea, zoomLevel);
        }
        else
        {
            gem::PolygonGeographicArea polyArea(coordinates);
            m_mapView->centerOnArea(polyArea, zoomLevel);
        }
    }

    void TriggerErrorPopup(const std::string& message)
    {
        g_message = message;
        g_showErrorPopup = true;

        if (!ImGui::IsPopupOpen("Error", ImGuiPopupFlags_AnyPopup))
            ImGui::OpenPopup("Error");
    }

    void TriggerLoadingPopup(const std::string& message)
    {
        g_message = message;
        g_showLoadingPopup = true;

        if (!ImGui::IsPopupOpen("Loading", ImGuiPopupFlags_AnyPopup))
            ImGui::OpenPopup("Loading");
    }

    void ShowErrorPopup()
    {
        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextWrapped("%s", g_message.c_str());
            ImGui::Separator();

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
                g_showErrorPopup = false; 
            }

            ImGui::EndPopup();
        }
    }

    void ShowLoadingPopup()
    {
        if (ImGui::BeginPopupModal("Loading", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Please wait, %s", g_message.c_str());
            ImGui::Separator();
            ImGui::Text("This may take a few seconds.");

            if (!g_showLoadingPopup)
                ImGui::CloseCurrentPopup(); 

            ImGui::EndPopup();
        }
    }
private:
    float randomInt() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> dis(0, 255);
        return dis(gen);
    }

    gem::Rgba GetColor()
    {
        return gem::Rgba(randomInt(), randomInt(), randomInt(), 255);
    }


private:
    gem::StrongPointer<gem::MapView> m_mapView;
};

class Controller : public gem::IProgressListener
{
public:

    enum class EControllerOperation
    {
        None = 0,
        AddOptimization = 1,
        GetSolution = 2,
        GetRequest = 3,
    };

    Controller(gem::StrongPointer<gem::MapView> mapView)
        : m_operation(EControllerOperation::None), m_UIController(UIController(mapView))
    {
        m_optimization = SetUpOptimization();
        m_UIController.DrawOrders(m_optimization.getOrders(), m_optimization.getDepartures(), m_optimization.getDestinations());

        std::vector<gem::Coordinates> coordinates;
        auto append_coordinates = [&](const auto& vec) {
            std::transform(vec.begin(), vec.end(), std::back_inserter(coordinates),
                [](const auto& obj) { return obj.getCoordinates(); });
        };
        append_coordinates(m_optimization.getOrders());
        append_coordinates(m_optimization.getDepartures());
        append_coordinates(m_optimization.getDestinations());

        m_UIController.CenterOnArea(coordinates, -1);
    }

    void notifyStart(bool hasProgress) override
    {
        if (m_operation != EControllerOperation::None && m_operation != EControllerOperation::GetRequest)
        {
            switch (m_operation)
            {
            case EControllerOperation::AddOptimization:
                g_message = "adding optimization...";
                break;

            case EControllerOperation::GetSolution:
                g_message =  "loading solution...";
            default:
                break;
            }
            m_UIController.TriggerLoadingPopup(g_message);
        }
    }

    void notifyComplete(int reason, gem::String hint) override 
    {
        if (reason == gem::KNoError)
            switch (m_operation)
            {
            case EControllerOperation::AddOptimization:
                GetRequest();
                break;

            case EControllerOperation::GetSolution:

                g_showLoadingPopup = false;

                if (!m_routes.empty())
                    m_UIController.DrawRoutes(m_routes);
                else
                    m_UIController.TriggerErrorPopup(hint.toStdString());

                break;

            case EControllerOperation::GetRequest:
                if (m_request.status == gem::vrp::ERequestStatus::eFinished)
                {
                    g_showLoadingPopup = false;
                    GetSolution(m_optimization, m_routes);
                }
                else
                    GetRequest();
                break;

            default:
                break;
            }

        else
            m_UIController.TriggerErrorPopup(hint.toStdString());
    }

    void CalculateOptimization() 
    {
        AddOptimization(m_optimization);
    }

    void ShowErrorPopup()
    {
        m_UIController.ShowErrorPopup();
    }

    void ShowLoadingPopup()
    {
        m_UIController.ShowLoadingPopup();
    }
private:
    //VRP Operations
    void AddOptimization(gem::vrp::Optimization& optimization)
    {
        m_operation = EControllerOperation::AddOptimization;
        gem::vrp::Service serv;

         if (gem::vrp::Service().addOptimization(this, optimization, m_request) != gem::KNoError)
             m_UIController.TriggerErrorPopup("Failed to send addOptimization request.");
    }

    void GetSolution(gem::vrp::Optimization& optimization, gem::vrp::RouteList& routes)
    {
        m_operation = EControllerOperation::GetSolution;
        if (optimization.getSolution(this, routes) != gem::KNoError)
            m_UIController.TriggerErrorPopup("Failed to send getSolution request.");
    }

    void GetRequest()
    {
        m_operation = EControllerOperation::GetRequest;
        if (gem::vrp::Service().getRequest(this, m_request, m_request.id) != gem::KNoError)
            m_UIController.TriggerErrorPopup("Failed to send getRequest request.");
    }

private:

    EControllerOperation m_operation;
    UIController m_UIController;

    gem::vrp::Request m_request;
    gem::vrp::Optimization m_optimization;
    gem::vrp::RouteList m_routes;
};

auto getUiRender()
{
    return std::bind([&](gem::StrongPointer<gem::MapView> mapView)
        {
            ImGuiIO& io = ImGui::GetIO();
            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::Begin("panel", nullptr, ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoDecoration
                | ImGuiWindowFlags_AlwaysAutoResize
                | ImGuiWindowFlags_NoSavedSettings);


            static std::shared_ptr<Controller> controller = std::make_shared<Controller>(mapView);

            if (ImGui::Button("Calculate Optimization"))
                controller->CalculateOptimization();


            if (g_showLoadingPopup)
                controller->ShowLoadingPopup();
            if (g_showErrorPopup)
                controller->ShowErrorPopup();

            ImGui::End();
        }
    , std::placeholders::_1);
}

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


	//// Create an interactive map view
	CTouchEventListener pTouchEventListener;
	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(session.produceOpenGLContext(Environment::WindowFrameworks::ImGUI, "CalculateOptimization", &pTouchEventListener, getUiRender()));
    
	if ( !mapView )
	{
		GEM_LOGE("Error creating gem::MapView: %d", GEM_GET_API_ERROR());
	}

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
