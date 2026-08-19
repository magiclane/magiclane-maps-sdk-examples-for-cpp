// SPDX-FileCopyrightText: 2025-2026 Magic Lane International B.V. <info@magiclane.com>
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

namespace
{
    gem::vrp::Customer CreateCustomer( const std::string& alias, const std::string& contactId, const gem::Coordinates& coords )
    {
        gem::vrp::Customer customer;
        customer.setCoordinates( coords );
        customer.setAlias( alias );
        customer.setPhoneNumber( "+33144781234" );
        customer.setEmail( contactId + "@example.fr" ); // contactId is kept mail-safe; the alias is what shows on the map
        return customer;
    }

    gem::vrp::Vehicle CreateVehicle( const std::string& name, gem::vrp::EVehicleType type, gem::vrp::EVehicleStatus status, const std::string& manufacturer,
                                     const std::string& model, gem::vrp::EFuelType fuelType, double consumption, const std::string& licensePlate, double maxWeight, double maxCube,
                                     int startTime, int endTime )
    {
        gem::vrp::Vehicle vehicle;
        vehicle.setName( name );
        vehicle.setType( type );
        vehicle.setStatus( status );
        vehicle.setManufacturer( manufacturer );
        vehicle.setModel( model );
        vehicle.setFuelType( fuelType );
        vehicle.setConsumption( static_cast<float>( consumption ) );
        vehicle.setLicensePlate( licensePlate );
        vehicle.setMaxWeight( static_cast<float>( maxWeight ) );
        vehicle.setMaxCube( static_cast<float>( maxCube ) );
        vehicle.setStartTime( startTime );
        vehicle.setEndTime( endTime );
        return vehicle;
    }

    gem::vrp::Order CreateOrder( const gem::vrp::Customer& customer, int numberOfPackages, double weight, double cube, double revenue, int serviceTime,
                                 std::pair<int, int> timeWindow, gem::vrp::EOrderType type )
    {
        gem::vrp::Order order( customer );
        order.setNumberOfPackages( numberOfPackages );
        order.setWeight( static_cast<float>( weight ) );
        order.setCube( static_cast<float>( cube ) );
        order.setServiceTime( serviceTime );
        order.setTimeWindow( timeWindow );
        order.setRevenue( static_cast<float>( revenue ) );
        order.setType( type );
        return order;
    }

    gem::vrp::Optimization SetUpOptimization()
    {
        ProgressListener listener;
        gem::vrp::Service serv;

        // A single working day for a small parcel fleet operating inside Paris, within the peripherique.
        gem::vrp::CustomerList customerList;

        const std::vector<std::string> customerAliases = {
            "Marche des Enfants Rouges (3e)", "Rue de Rivoli - Louvre (1er)", "Saint-Germain-des-Pres (6e)", "Rue Mouffetard (5e)",
            "Rue Lepic - Montmartre (18e)",   "Les Batignolles (17e)",        "Rue de Ponthieu (8e)",        "Rue Cler (7e)",
            "Rue de la Roquette (11e)",       "Place d'Italie (13e)",         "Rue de la Gaite (14e)",       "Rue de Belleville (20e)" };

        const gem::CoordinatesList coordinates = { gem::Coordinates( 48.862930, 2.362400 ), gem::Coordinates( 48.860680, 2.336540 ), gem::Coordinates( 48.853740, 2.333070 ),
                                                   gem::Coordinates( 48.842000, 2.350100 ), gem::Coordinates( 48.886700, 2.334500 ), gem::Coordinates( 48.884200, 2.319300 ),
                                                   gem::Coordinates( 48.870900, 2.306900 ), gem::Coordinates( 48.856900, 2.306300 ), gem::Coordinates( 48.855700, 2.376000 ),
                                                   gem::Coordinates( 48.831200, 2.355500 ), gem::Coordinates( 48.840400, 2.323700 ), gem::Coordinates( 48.871900, 2.381300 ) };

        for( size_t index = 0; index < coordinates.size(); index++ )
        {
            gem::vrp::Customer customer = CreateCustomer( customerAliases[index], "paris-c" + std::to_string( index ), coordinates[index] );
            serv.addCustomer( &listener, customer );
            WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
            customerList.push_back( customer );
        }

        const std::vector<uint8_t> numberOfPackages = { 6, 4, 9, 3, 7, 5, 12, 8, 6, 10, 4, 7 };

        // Kilograms. Total = 461.5 kg against 2 x 300 kg, so the vans have to share the day.
        const std::vector<float> weights = { 42.5, 18.0, 65.0, 12.5, 30.0, 22.5, 88.0, 47.0, 26.5, 55.0, 19.5, 35.0 };

        const std::vector<float> cubes = { 0.8, 0.3, 1.2, 0.2, 0.6, 0.4, 2.1, 0.9, 0.5, 1.4, 0.3, 0.7 };
        const std::vector<float> revenues = { 0.0, 0.0, 145.0, 0.0, 0.0, 60.0, 0.0, 210.0, 0.0, 180.0, 0.0, 0.0 };

        // SECONDS spent at the stop (see Order::setServiceTime).
        const std::vector<uint16_t> serviceTimes = { 900, 600, 1200, 480, 720, 600, 1500, 900, 600, 1080, 480, 720 };

        // MINUTES FROM MIDNIGHT (see Order::setTimeWindow); { 0, INT_MAX } is the default, "any time in shift".
        // Only 3 of the 12 orders are constrained: 07:00-12:00, 13:00-18:00 and 15:00-19:00.
        //
        // Both bounds must be set for a window to exist. The solver applies it only when start != 0 AND
        // end != INT_MAX, so a one-sided window such as { 0, 720 } or { 900, INT_MAX } is silently ignored.
        const std::vector<std::pair<int, int>> timeWindows = { { 420, 720 },  { 0, INT_MAX }, { 0, INT_MAX }, { 0, INT_MAX }, { 0, INT_MAX }, { 0, INT_MAX },
                                                               { 780, 1080 }, { 0, INT_MAX }, { 0, INT_MAX }, { 0, INT_MAX }, { 0, INT_MAX }, { 900, 1140 } };

        const std::vector<gem::vrp::EOrderType> orderTypes = { gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_Delivery,
                                                               gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_PickUp,
                                                               gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_PickUp,
                                                               gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_Delivery, gem::vrp::EOrderType::OT_PickUp };

        gem::vrp::OrderList orderList;
        for( size_t index = 0; index < customerList.size(); index++ )
        {
            gem::vrp::Order order = CreateOrder( customerList[index], numberOfPackages[index], weights[index], cubes[index], revenues[index], serviceTimes[index],
                                                 timeWindows[index], orderTypes[index] );
            serv.addOrder( &listener, order, false );
            WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
            orderList.push_back( order );
        }

        // Two depots, north and south-east, so the vans start from opposite halves of the city.
        gem::vrp::Departure departure1;
        departure1.setAlias( "Depot Nord - Chapelle International (18e)" );
        departure1.setCoordinates( gem::Coordinates( 48.898700, 2.360800 ) );
        gem::vrp::Departure departure2;
        departure2.setAlias( "Depot Sud - Quai de Bercy (12e)" );
        departure2.setCoordinates( gem::Coordinates( 48.833900, 2.386800 ) );

        // Shared end-of-shift yard for both vehicles.
        gem::vrp::Destination destination;
        destination.setAlias( "Depot central - Gare d'Austerlitz (13e)" );
        destination.setCoordinates( gem::Coordinates( 48.842400, 2.365600 ) );

        gem::vrp::VehicleList vehicles;

        // MINUTES FROM MIDNIGHT (see Vehicle::setStartTime / setEndTime): 06:30-15:30 and 11:30-19:30.
        //
        // KEEP BOTH END TIMES UNDER 1440 (24:00). The server derives the planning horizon from the largest
        // vehicle end time, and once that crosses a full day it builds a multi-day model that carves the
        // out-of-window intervals of every extra day out of each windowed stop's arrival-time domain, on a
        // thread pool the model build blocks on. A single-day horizon skips that path entirely.
        gem::vrp::Vehicle vehicle1 = CreateVehicle( "Van Paris Nord", gem::vrp::EVehicleType::VT_Car, gem::vrp::EVehicleStatus::VS_Available, "Volkswagen", "Transporter",
                                                    gem::vrp::EFuelType::FT_GasolinePremium, 8.5, "AB-123-CD", 300, 15, 390, 930 );
        serv.addVehicle( &listener, vehicle1 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        vehicles.push_back( vehicle1 );
        gem::vrp::Vehicle vehicle2 = CreateVehicle( "Van Paris Sud", gem::vrp::EVehicleType::VT_Car, gem::vrp::EVehicleStatus::VS_Available, "Volkswagen", "Transporter",
                                                    gem::vrp::EFuelType::FT_GasolinePremium, 8.5, "EF-456-GH", 300, 15, 690, 1170 );
        serv.addVehicle( &listener, vehicle2 );
        WAIT_UNTIL( std::bind( &ProgressListener::IsFinished, &listener ), 5000 );
        vehicles.push_back( vehicle2 );

        // One shared constraints entry, so both vehicles share a start date. The server offsets each shift by
        // (its start date - the earliest one), so vehicles dated on different days re-enable the multi-day path.
        gem::vrp::VehicleConstraintsList vehConstraintsList;
        gem::vrp::VehicleConstraints vehConstr1;
        vehConstr1.setMaxNumberOfPackages( 100 );
        vehConstr1.setMaxRevenue( 2000 );
        vehConstr1.setStartDate( gem::Time( 2026, 9, 15 ) ); // September 15, 2026 - a Tuesday
        vehConstr1.setMinNumberOfOrders( 1 );
        vehConstr1.setMaxNumberOfOrders( 50 );
        vehConstr1.setMinDistance( 1 );
        vehConstr1.setMaxDistance( 250 ); // km - a full day of city driving inside Paris, not a country-wide tour

        vehConstraintsList.push_back( vehConstr1 );

        // Rue de Rivoli -> Rue Mouffetard -> Saint-Germain.
        //
        // OSO_InFixedSequence is stronger than "visit these in this order": the server makes the stops strictly
        // CONSECUTIVE on one vehicle, with nothing inserted between them. Keep the members free of time windows -
        // a windowed order inside a fixed sequence pins the whole rigid block to one shift and one time span,
        // and the optimization then comes back with no routes at all.
        gem::vrp::OrdersSequenceMap ordersSequence;
        gem::LargeIntListList fixedSequence = gem::LargeIntListList { gem::LargeIntList { orderList[1].getId(), orderList[3].getId(), orderList[2].getId() } };
        ordersSequence.insert( std::make_pair( gem::vrp::EOrdersSequenceOption::OSO_InFixedSequence, fixedSequence ) );

        gem::vrp::ConfigurationParameters configParams;
        configParams.setName( "Paris intra-muros delivery optimization" );
        configParams.setIgnoreTimeWindow( false );
        configParams.setOptimizationCriterion( gem::vrp::EOptimizationCriterion::OC_Distance );
        configParams.setOptimizationQuality( gem::vrp::EOptimizationQuality::OQ_Optimized );
        configParams.setMaxWaitTime( 7200 ); // SECONDS - a van may idle at most 2 h waiting for a window to open
        configParams.setRouteType( gem::vrp::ERouteType::RT_CustomEnd );
        configParams.setRestrictions( gem::vrp::ERoadRestrictions::RR_None );
        configParams.setDistanceUnit( gem::vrp::EDistanceUnit::DU_Kilometers );
        configParams.setOrderSequenceOptions( ordersSequence );

        // Defaults to false, which means one unservable order leaves you with no routes at all and no hint as to
        // why. Enabled, the solver retries with the orders made optional and returns a partial solution instead.
        configParams.setAllowDroppingOrders( true );

        gem::vrp::Optimization optimization;
        optimization.setConfigurationParameters( configParams );
        optimization.setVehicles( vehicles );
        optimization.setDepartures( { departure1, departure2 } );
        optimization.setDestinations( { destination } ); // both vehicles will end their routes at the same destination
        optimization.setOrders( orderList );
        optimization.setVehiclesConstraints( vehConstraintsList );
        optimization.setMatrixBuildType( gem::vrp::EMatrixBuildType::MBT_Real );

        return optimization;
    }
}

static bool g_showLoadingPopup = false;
static bool g_showErrorPopup = false;
static std::string g_message;

class UIController
{
public:
    UIController( gem::StrongPointer<gem::MapView> mapView )
        : m_mapView( mapView )
    {
    }

    void DrawRoutes( const gem::vrp::RouteList& routes )
    {
        m_mapView->deactivateAllHighlights();
        m_mapView->preferences().markers().clear();

        gem::CoordinatesList coords;
        gem::vrp::OrderList orderList;
        gem::vrp::DepartureList departures;
        gem::vrp::DestinationList destinations;
        for( const auto& route : routes )
        {
            gem::CoordinatesList shape = route.getShape();

            auto shapeToDraw = gem::MarkerCollection( gem::EMarkerType::MT_Polyline, "shape" );
            shapeToDraw.add( gem::Marker( shape ) );

            gem::MarkerCollectionDisplaySettings settings;
            gem::Rgba color = GetColor();
            settings.setPolylineInnerColor( color );

            m_mapView->preferences().markers().add( shapeToDraw, settings );

            coords.insert( coords.end(), shape.begin(), shape.end() );

            orderList.insert( orderList.end(), route.getOrders().begin(), route.getOrders().end() );
            departures.push_back( route.getDeparture() );
            if( route.getConfigurationParameters().getRouteType() != gem::vrp::RT_RoundRoute )
                destinations.push_back( route.getDestination() );
        }

        DrawOrders( orderList, departures, destinations );
        CenterOnArea( coords, -1 );
    }

    void DrawOrders( const gem::vrp::OrderList& orders, const gem::vrp::DepartureList& departures, const gem::vrp::DestinationList& destinations )
    {
        gem::LandmarkList landmarks;
        for( const auto& order : orders )
        {
            gem::Landmark landmark;
            landmark.setName( order.getAlias() );
            landmark.setCoordinates( order.getCoordinates() );
            landmark.setImage( gem::Icon::Core::CoreBase );
            landmarks.push_back( landmark );
        }

        for( const auto& departure : departures )
        {
            gem::Landmark landmark;
            landmark.setName( departure.getAlias() );
            landmark.setCoordinates( departure.getCoordinates() );
            landmark.setImage( gem::Icon::Core::Waypoint_Start );
            landmarks.push_back( landmark );
        }

        for( const auto& destination : destinations )
        {
            gem::Landmark landmark;
            landmark.setName( destination.getAlias() );
            landmark.setCoordinates( destination.getCoordinates() );
            landmark.setImage( gem::Icon::Core::Waypoint_Finish );
            landmarks.push_back( landmark );
        }

        if( m_mapView != nullptr && !landmarks.empty() )
            m_mapView->activateHighlight( landmarks, gem::HO_ShowLandmark | gem::HO_NoFading | gem::HO_Overlap );
    }

    void CenterOnArea( const gem::CoordinatesList& coordinates, int zoomLevel = 30 )
    {
        if( coordinates.empty() )
        {
            gem::CoordinatesList cursorCoordinates = gem::CoordinatesList { m_mapView->getCursorWgsPosition() };
            gem::PolygonGeographicArea polyArea( cursorCoordinates );
            m_mapView->centerOnArea( polyArea, zoomLevel );
        }
        else
        {
            gem::PolygonGeographicArea polyArea( coordinates );
            m_mapView->centerOnArea( polyArea, zoomLevel );
        }
    }

    void TriggerErrorPopup( const std::string& message )
    {
        g_message = message;
        g_showErrorPopup = true;

        if( !ImGui::IsPopupOpen( "Error", ImGuiPopupFlags_AnyPopup ) )
            ImGui::OpenPopup( "Error" );
    }

    void TriggerLoadingPopup( const std::string& message )
    {
        g_message = message;
        g_showLoadingPopup = true;

        if( !ImGui::IsPopupOpen( "Loading", ImGuiPopupFlags_AnyPopup ) )
            ImGui::OpenPopup( "Loading" );
    }

    void ShowErrorPopup()
    {
        if( ImGui::BeginPopupModal( "Error", NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
        {
            ImGui::TextWrapped( "%s", g_message.c_str() );
            ImGui::Separator();

            if( ImGui::Button( "Close" ) )
            {
                ImGui::CloseCurrentPopup();
                g_showErrorPopup = false;
            }

            ImGui::EndPopup();
        }
    }

    void ShowLoadingPopup()
    {
        if( ImGui::BeginPopupModal( "Loading", NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
        {
            ImGui::Text( "Please wait, %s", g_message.c_str() );
            ImGui::Separator();
            ImGui::Text( "This may take a few seconds." );

            if( !g_showLoadingPopup )
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

private:
    int randomInt()
    {
        static std::random_device rd;
        static std::mt19937 gen( rd() );
        static std::uniform_int_distribution<int> dis( 0, 255 );
        return dis( gen );
    }

    gem::Rgba GetColor()
    {
        return gem::Rgba( randomInt(), randomInt(), randomInt(), 255 );
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

    enum class ECalculationState
    {
        Idle = 0,    ///< Nothing submitted yet, or the last attempt failed - the vehicles are free.
        Running = 1, ///< Submitted, waiting for the request to finish and for the solution.
        Solved = 2,  ///< Routes came back, so the vehicles are assigned and cannot be used again.
    };

    Controller( gem::StrongPointer<gem::MapView> mapView )
        : m_operation( EControllerOperation::None )
        , m_UIController( UIController( mapView ) )
        , m_calculationState( ECalculationState::Idle )
    {
        m_optimization = SetUpOptimization();
        m_UIController.DrawOrders( m_optimization.getOrders(), m_optimization.getDepartures(), m_optimization.getDestinations() );

        std::vector<gem::Coordinates> coordinates;
        auto append_coordinates = [&]( const auto& vec )
        {
            std::transform( vec.begin(), vec.end(), std::back_inserter( coordinates ),
                            []( const auto& obj )
                            {
                                return obj.getCoordinates();
                            } );
        };
        append_coordinates( m_optimization.getOrders() );
        append_coordinates( m_optimization.getDepartures() );
        append_coordinates( m_optimization.getDestinations() );

        m_UIController.CenterOnArea( coordinates, -1 );
    }

    void notifyStart( bool hasProgress ) override
    {
        if( m_operation != EControllerOperation::None && m_operation != EControllerOperation::GetRequest )
        {
            switch( m_operation )
            {
                case EControllerOperation::AddOptimization:
                    g_message = "adding optimization...";
                    break;

                case EControllerOperation::GetSolution:
                    g_message = "loading solution...";
                default:
                    break;
            }
            m_UIController.TriggerLoadingPopup( g_message );
        }
    }

    void notifyComplete( int reason, gem::String hint ) override
    {
        if( reason == gem::KNoError )
            switch( m_operation )
            {
                case EControllerOperation::AddOptimization:
                    GetRequest();
                    break;

                case EControllerOperation::GetSolution:

                    g_showLoadingPopup = false;

                    if( !m_routes.empty() )
                    {
                        // Only here are the vehicles actually committed to routes.
                        m_calculationState = ECalculationState::Solved;
                        m_UIController.DrawRoutes( m_routes );
                    }
                    else
                    {
                        // No routes came back, so the vehicles were never assigned and can be retried.
                        m_calculationState = ECalculationState::Idle;
                        m_UIController.TriggerErrorPopup( hint.toStdString() );
                    }

                    break;

                case EControllerOperation::GetRequest:
                    if( m_request.status == gem::vrp::ERequestStatus::eFinished )
                    {
                        g_showLoadingPopup = false;
                        GetSolution( m_optimization, m_routes );
                    }
                    else
                        GetRequest();
                    break;

                default:
                    break;
            }

        else
        {
            // The step failed, so no route was produced and the vehicles are still free.
            m_calculationState = ECalculationState::Idle;

            g_showLoadingPopup = false;
            m_UIController.TriggerErrorPopup( hint.toStdString() );
        }
    }

    void CalculateOptimization()
    {
        m_calculationState = ECalculationState::Running;
        AddOptimization( m_optimization );
    }

    ECalculationState GetCalculationState() const
    {
        return m_calculationState;
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
    void AddOptimization( gem::vrp::Optimization& optimization )
    {
        m_operation = EControllerOperation::AddOptimization;
        gem::vrp::Service serv;

        if( gem::vrp::Service().addOptimization( this, optimization, m_request ) != gem::KNoError )
        {
            m_calculationState = ECalculationState::Idle;
            m_UIController.TriggerErrorPopup( "Failed to send addOptimization request." );
        }
    }

    void GetSolution( gem::vrp::Optimization& optimization, gem::vrp::RouteList& routes )
    {
        m_operation = EControllerOperation::GetSolution;
        if( optimization.getSolution( this, routes ) != gem::KNoError )
        {
            m_calculationState = ECalculationState::Idle;
            m_UIController.TriggerErrorPopup( "Failed to send getSolution request." );
        }
    }

    void GetRequest()
    {
        m_operation = EControllerOperation::GetRequest;
        if( gem::vrp::Service().getRequest( this, m_request, m_request.id ) != gem::KNoError )
        {
            m_calculationState = ECalculationState::Idle;
            m_UIController.TriggerErrorPopup( "Failed to send getRequest request." );
        }
    }

private:
    EControllerOperation m_operation;
    UIController m_UIController;
    ECalculationState m_calculationState;

    gem::vrp::Request m_request;
    gem::vrp::Optimization m_optimization;
    gem::vrp::RouteList m_routes;
};

namespace
{
    auto getUiRender()
    {
        return std::bind(
            [&]( gem::StrongPointer<gem::MapView> mapView )
            {
                const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos( ImVec2( main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20 ), ImGuiCond_FirstUseEver );
                ImGui::Begin( "panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings );

                static std::shared_ptr<Controller> controller = std::make_shared<Controller>( mapView );

                // Disabled while a calculation is in flight, and permanently once one has produced routes: the
                // vehicles are then assigned and cannot be optimized again. A failed attempt leaves them free,
                // so the button comes back and the optimization can be retried.
                const Controller::ECalculationState state = controller->GetCalculationState();

                ImGui::BeginDisabled( state != Controller::ECalculationState::Idle );
                if( ImGui::Button( "Calculate Optimization" ) )
                    controller->CalculateOptimization();
                ImGui::EndDisabled();

                if( state == Controller::ECalculationState::Solved )
                    ImGui::TextDisabled( "The vehicles are assigned to these routes - restart to run another optimization." );

                if( g_showLoadingPopup )
                    controller->ShowLoadingPopup();
                if( g_showErrorPopup )
                    controller->ShowErrorPopup();

                ImGui::End();
            },
            std::placeholders::_1 );
    }
}

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

    //// Create an interactive map view
    CTouchEventListener pTouchEventListener;
    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "CalculateOptimization", &pTouchEventListener, getUiRender() ) );

    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    WAIT_UNTIL_WINDOW_CLOSE();

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
