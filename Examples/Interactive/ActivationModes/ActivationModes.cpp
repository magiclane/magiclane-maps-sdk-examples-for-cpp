// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

// ActivationModes
// ---------------
// Shows every way an SDK with auto-activation can become (and stop being) activated, and how an application can
// reflect the SDK's activation state to its user.
//
// SDKs downloaded from the Magic Lane website have auto-activation enabled: given a project API token, the SDK
// activates itself the first time it can reach Magic Lane Services. That leaves two situations an application must
// handle on its own:
//
//   1. The device has no internet at all (or the application must not go online). The SDK cannot auto-activate, so it
//      reports itself as NOT ACTIVATED through gem::ISdkExceptions::onSdkNotActivated(), and offline functionality is
//      disabled until it is activated. The application decides how to show this - here we draw a watermark text on
//      the map, and clear it again when gem::ISdkExceptions::onSdkActivated() arrives.
//
//   2. Activating (or releasing) such a device MANUALLY, without the device ever going online - the offline ceremony:
//        a. gem::ActivationService::getOfflineActivationRequestBlob()   -> a request blob, produced entirely on-device
//        b. take the blob to Magic Lane Services from a machine that IS online. Either scan the QR code shown here with
//           the Magic Lane companion app, or send the REST request shown here yourself - both return an
//           offline_activation_key.
//        c. gem::ActivationService::completeOfflineActivation( key )    -> the SDK is activated, offline stays usable
//      Deactivation mirrors it with getOfflineDeactivationRequestBlob() / completeOfflineDeactivation().
//
// The example starts with the internet connection DISALLOWED so the not-activated state is visible immediately. Use
// the panel to allow the connection (auto-activation), or to walk through the manual offline ceremony. A "Reset to first
// run" button returns the device to the not-activated offline state so every scenario can be replayed in one run.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_ActivationService.h>
#include <API/GEM_SdkSettings.h>

#include <imgui.h>

#ifdef HAVE_QRCODEGEN
    #include <qrcodegen.hpp>
#endif

#include <atomic>
#include <string>
#include <cstring>

namespace
{
    // Magic Lane Services identifier of the activation service, used to look up its URL for the REST request shown to
    // the developer (gem::Debug::getDefUrls). The SDK's own activation code talks to the same service.
    constexpr int kActivationServiceId = 3;

    // REST contract of the activation service (the same one the SDK uses internally):
    //   POST   {service url}/services/tokens/v1/activations   body {"request_blob": "..."}  -> {"offline_activation_key": "..."}
    //   DELETE {service url}/services/tokens/v1/activations   body {"request_blob": "..."}  -> {"offline_deactivation_key": "..."}
    constexpr const char* kActivationsPath = "/services/tokens/v1/activations";

    // Where the SDK itself learns the service URLs from: a plain GET returns a JSON array of services, each with a
    // "service_id" and its "URLs". A developer can call it from any online machine to look up the activation service
    // (service_id 3) for a device that is never allowed online.
    constexpr const char* kServicesListJsonUrl = "https://m71os.services.magicearthsdk.com/services_list_json";

    // Everything the panel shows and edits. The notification callbacks may run on SDK threads, so they only touch the
    // atomics; the UI thread reads them each frame and does the actual work (watermark, status text).
    struct DemoState
    {
        // written by the SDK notifications (any thread), consumed by the UI thread
        std::atomic<bool> notActivated { false };
        std::atomic<int> notActivatedReason { 0 };
        std::atomic<int> notificationCounter { 0 }; // bumps on every notification so the UI knows something changed

        // UI thread only
        bool watermarkShown = false;
        bool allowOnline = false;
        std::string lastNotification = "none yet";
        std::string message;

        std::string activationBlob;
        std::string deactivationBlob;
        char licenseKeyInput[64] = "";           // optional portal-issued license key; empty => the SDK generates one
        char offlineActivationKeyInput[64] = ""; // pasted back from the companion app / REST response
        char offlineDeactivationKeyInput[64] = "";
        char activationServiceUrlInput[160] = ""; // filled from the SDK once it knows it, or typed in on an offline device

        gem::StrongPointer<OffboardListenerImpl> offboardListener;
    };

    DemoState g_state;

    std::string ToStd( const gem::String& s )
    {
        return s.toStdString();
    }

    const char* ReasonToText( gem::ESdkNotActivatedReason reason )
    {
        switch( reason )
        {
            case gem::ESdkNotActivatedReason::NotYetActivated:
                return "not yet activated";
            case gem::ESdkNotActivatedReason::ActivationExpired:
                return "activation expired";
            default:
                return "unknown";
        }
    }

    // The base URL of the activation service. The SDK learns its services configuration from Magic Lane Services, so
    // gem::Debug::getDefUrls() only knows it after the SDK has been online once; on a device that is never allowed online
    // the developer types it in, looked up with kServicesListJsonUrl from an online machine. Returns an empty string
    // while unknown.
    std::string ActivationServiceUrl()
    {
        if( g_state.activationServiceUrlInput[0] == '\0' )
        {
            gem::StringList urls = gem::Debug().getDefUrls( kActivationServiceId );
            if( urls.size() > 0 )
            {
                const std::string url = ToStd( urls.at( 0 ) );
                std::strncpy( g_state.activationServiceUrlInput, url.c_str(), sizeof( g_state.activationServiceUrlInput ) - 1 );
            }
        }
        return g_state.activationServiceUrlInput;
    }

    // Draws `text` as a QR code the Magic Lane companion app can scan. Falls back to a note when the QR generator is
    // not available in this build.
    void DrawQrCode( const std::string& text, float sizePx )
    {
#ifdef HAVE_QRCODEGEN
        try
        {
            const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText( text.c_str(), qrcodegen::QrCode::Ecc::LOW );
            const int modules = qr.getSize();
            const int quietZone = 4; // modules of white border, required by the QR standard
            const float cell = sizePx / float( modules + 2 * quietZone );

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImVec2 origin = ImGui::GetCursorScreenPos();

            drawList->AddRectFilled( origin, ImVec2( origin.x + sizePx, origin.y + sizePx ), IM_COL32( 255, 255, 255, 255 ) );
            for( int y = 0; y < modules; ++y )
            {
                for( int x = 0; x < modules; ++x )
                {
                    if( !qr.getModule( x, y ) )
                        continue;
                    const ImVec2 a( origin.x + ( x + quietZone ) * cell, origin.y + ( y + quietZone ) * cell );
                    drawList->AddRectFilled( a, ImVec2( a.x + cell, a.y + cell ), IM_COL32( 0, 0, 0, 255 ) );
                }
            }
            ImGui::Dummy( ImVec2( sizePx, sizePx ) ); // reserve the layout space we drew into
        }
        catch( const std::exception& e )
        {
            ImGui::TextWrapped( "Blob too large for a QR code (%s). Use the raw blob or the REST request below.", e.what() );
        }
#else
        ( void ) text;
        ( void ) sizePx;
        ImGui::TextWrapped( "QR rendering is not available in this build (nayuki-qr-code-generator not found). Use the raw blob or the REST request below." );
#endif
    }

    // Shows the exact request a developer can issue with any HTTP client to obtain the offline key for a blob.
    void DrawRestRequest( const char* method, const std::string& blob, const char* responseKeyName )
    {
        ImGui::PushID( responseKeyName ); // the same widgets appear in the activation and the deactivation section

        ImGui::TextUnformatted( "Or send the request yourself, from any online machine:" );

        std::string serviceUrl = ActivationServiceUrl();
        if( serviceUrl.empty() )
        {
            ImGui::TextWrapped( "The activation service URL is not known yet: the SDK receives it in its services list, which it has not fetched because "
                                "the connection is disallowed. From any online machine, GET the services list yourself:" );
            std::string servicesListRequest = std::string( "GET " ) + kServicesListJsonUrl;
            ImGui::SetNextItemWidth( 460 );
            ImGui::InputText( "##services_list", &servicesListRequest[0], servicesListRequest.size() + 1, ImGuiInputTextFlags_ReadOnly );
            ImGui::TextWrapped( "It returns a JSON array of services; take the entry with \"service_id\": %d and use one of its \"URLs\", typed in below. "
                                "Alternatively tick \"Allow internet connection\" once and this panel fills the field in from gem::Debug().getDefUrls( %d ).",
                                kActivationServiceId, kActivationServiceId );
            serviceUrl = "https://<activation-service>";
        }
        ImGui::InputTextWithHint( "Activation service URL", "https://...", g_state.activationServiceUrlInput, sizeof( g_state.activationServiceUrlInput ) );

        std::string request = std::string( method ) + " " + serviceUrl + kActivationsPath + "\nContent-Type: application/json\n\n{\"request_blob\": \"" + blob + "\"}";
        ImGui::InputTextMultiline( "##rest", &request[0], request.size() + 1, ImVec2( 460, 90 ), ImGuiInputTextFlags_ReadOnly );
        ImGui::Text( "The response carries \"%s\" - paste it below.", responseKeyName );

        ImGui::PopID();
    }

    // The application's reaction to the SDK's activation state: a watermark text while not activated, none otherwise.
    // This is the application's choice; the SDK itself draws nothing for this state.
    void ApplyWatermark( gem::StrongPointer<gem::MapView> mapView )
    {
        const bool shouldShow = g_state.notActivated.load();
        if( shouldShow == g_state.watermarkShown )
            return;

        if( shouldShow )
            mapView->setWatermarkText( u"SDK not activated", u"Limited offline functionality", 1.0f, gem::EWPCenter );
        else
            mapView->setWatermarkText( u"", u"" );

        g_state.watermarkShown = shouldShow;
    }

    // Finds the license key of the currently active CORE activation (needed to build a deactivation request).
    std::string ActiveLicenseKey()
    {
        gem::ActivationInfoList activations = gem::ActivationService().getActivationsForProduct( gem::ProductID::CORE );
        for( unsigned i = 0; i < activations.size(); ++i )
        {
            if( activations.at( i ).status == gem::EActivationStatus::Activated )
                return ToStd( activations.at( i ).licenseKey );
        }
        return {};
    }

    // A status line "label : value" with the value in green (good) or red (bad).
    void DrawStatusLine( const char* label, bool good, const char* goodText, const char* badText )
    {
        const ImVec4 green( 0.30f, 0.85f, 0.35f, 1.0f );
        const ImVec4 red( 0.95f, 0.35f, 0.30f, 1.0f );

        ImGui::TextUnformatted( label );
        ImGui::SameLine();
        ImGui::TextColored( good ? green : red, "%s", good ? goodText : badText );
    }

    void DrawStatus()
    {
        const bool active = gem::ActivationService().isActive( gem::ProductID::CORE );
        const bool online = g_state.allowOnline && g_state.offboardListener && g_state.offboardListener->IsOnline();

        DrawStatusLine( "Activation :", active, "ACTIVATED", "NOT ACTIVATED" );
        DrawStatusLine( "Connection :", online, "online", "offline" );

        // The application id (the token's audience) is what the manual offline calls need. SdkSettings derives it from
        // the token given at initialization, so the application does not have to keep the token around.
        gem::String appId = gem::SdkSettings().getApplicationId();
        ImGui::Text( "App id     : %s", appId.empty() ? "(no token set)" : ToStd( appId ).c_str() );

        ImGui::Text( "Last SDK notification: %s", g_state.lastNotification.c_str() );
        if( !g_state.message.empty() )
            ImGui::TextWrapped( "%s", g_state.message.c_str() );
    }

    // Allows / disallows the SDK's internet connection. The same listener is handed over in both directions so it keeps
    // receiving onConnectionStatusUpdated() - passing an empty listener when disallowing would leave ours stuck "online".
    void SetConnectionAllowed( bool allow )
    {
        if( !g_state.offboardListener )
            g_state.offboardListener = gem::StrongPointerFactory<OffboardListenerImpl>();

        g_state.allowOnline = allow;
        gem::SdkSettings().setAllowConnection( allow, g_state.offboardListener );
        g_state.message = allow ? "Connection allowed - auto-activation will run once online." : "Connection disallowed.";
    }

    void DrawConnectivity()
    {
        ImGui::Separator();
        ImGui::TextUnformatted( "1. Auto-activation (online)" );
        ImGui::TextWrapped( "With the connection allowed, the SDK reaches Magic Lane Services and activates itself from the project token. "
                            "Disallow it to simulate a device without internet." );

        bool allow = g_state.allowOnline;
        if( ImGui::Checkbox( "Allow internet connection", &allow ) )
            SetConnectionAllowed( allow );
    }

    void DrawOfflineActivation()
    {
        ImGui::Separator();
        ImGui::TextUnformatted( "2. Manual offline activation" );
        ImGui::TextWrapped( "For a device that never goes online. The request blob is generated on-device; the offline_activation_key comes back "
                            "from Magic Lane Services via the companion app (scan the QR) or your own REST call." );

        ImGui::InputTextWithHint( "License key (optional)", "leave empty to let the SDK generate one", g_state.licenseKeyInput, sizeof( g_state.licenseKeyInput ) );

        if( ImGui::Button( "Get activation request blob" ) )
        {
            gem::String appId = gem::SdkSettings().getApplicationId();
            if( appId.empty() )
            {
                g_state.message = "No application token is set on the SDK - cannot build an activation request.";
            }
            else
            {
                auto result = gem::ActivationService().getOfflineActivationRequestBlob( appId, gem::String( g_state.licenseKeyInput ), gem::ProductID::CORE );
                if( result.first == gem::KNoError )
                {
                    g_state.activationBlob = ToStd( result.second );
                    g_state.message = "Activation request blob ready. Scan the QR with the companion app, or send the REST request.";
                }
                else
                {
                    g_state.activationBlob.clear();
                    g_state.message = "getOfflineActivationRequestBlob failed (" + std::to_string( result.first ) + "): " + ToStd( result.second );
                }
            }
        }

        if( !g_state.activationBlob.empty() )
        {
            ImGui::TextUnformatted( "Scan with the Magic Lane companion app:" );
            DrawQrCode( g_state.activationBlob, 260.f );

            ImGui::TextUnformatted( "Raw blob:" );
            ImGui::InputTextMultiline( "##ablob", &g_state.activationBlob[0], g_state.activationBlob.size() + 1, ImVec2( 460, 60 ), ImGuiInputTextFlags_ReadOnly );

            DrawRestRequest( "POST", g_state.activationBlob, "offline_activation_key" );

            ImGui::InputText( "offline_activation_key", g_state.offlineActivationKeyInput, sizeof( g_state.offlineActivationKeyInput ) );
            if( ImGui::Button( "Complete offline activation" ) )
            {
                auto result = gem::ActivationService().completeOfflineActivation( gem::String( g_state.offlineActivationKeyInput ) );
                if( result.first == gem::KNoError )
                {
                    g_state.message = "Offline activation completed - the SDK is activated.";
                    g_state.activationBlob.clear();
                    std::memset( g_state.offlineActivationKeyInput, 0, sizeof( g_state.offlineActivationKeyInput ) );
                }
                else
                {
                    g_state.message = "completeOfflineActivation failed (" + std::to_string( result.first ) + "): " + ToStd( result.second );
                }
            }
        }
    }

    void DrawOfflineDeactivation()
    {
        ImGui::Separator();
        ImGui::TextUnformatted( "3. Manual offline deactivation" );
        ImGui::TextWrapped( "Releases this device's activation without going online, e.g. to decommission it or reuse its license key." );

        if( ImGui::Button( "Get deactivation request blob" ) )
        {
            const std::string licenseKey = ActiveLicenseKey();
            gem::String appId = gem::SdkSettings().getApplicationId();
            if( licenseKey.empty() )
            {
                g_state.message = "Nothing to deactivate - no active CORE activation on this device.";
            }
            else if( appId.empty() )
            {
                g_state.message = "No application token is set on the SDK - cannot build a deactivation request.";
            }
            else
            {
                auto result = gem::ActivationService().getOfflineDeactivationRequestBlob( appId, gem::String( licenseKey.c_str() ), gem::ProductID::CORE );
                if( result.first == gem::KNoError )
                {
                    g_state.deactivationBlob = ToStd( result.second );
                    g_state.message = "Deactivation request blob ready. Scan the QR with the companion app, or send the REST request.";
                }
                else
                {
                    g_state.deactivationBlob.clear();
                    g_state.message = "getOfflineDeactivationRequestBlob failed (" + std::to_string( result.first ) + "): " + ToStd( result.second );
                }
            }
        }

        if( !g_state.deactivationBlob.empty() )
        {
            ImGui::TextUnformatted( "Scan with the Magic Lane companion app:" );
            DrawQrCode( g_state.deactivationBlob, 260.f );

            ImGui::TextUnformatted( "Raw blob:" );
            ImGui::InputTextMultiline( "##dblob", &g_state.deactivationBlob[0], g_state.deactivationBlob.size() + 1, ImVec2( 460, 60 ), ImGuiInputTextFlags_ReadOnly );

            DrawRestRequest( "DELETE", g_state.deactivationBlob, "offline_deactivation_key" );

            ImGui::InputText( "offline_deactivation_key", g_state.offlineDeactivationKeyInput, sizeof( g_state.offlineDeactivationKeyInput ) );
            if( ImGui::Button( "Complete offline deactivation" ) )
            {
                auto result = gem::ActivationService().completeOfflineDeactivation( gem::String( g_state.offlineDeactivationKeyInput ) );
                if( result.first == gem::KNoError )
                {
                    g_state.message = "Offline deactivation completed - the SDK is no longer activated.";
                    g_state.deactivationBlob.clear();
                    std::memset( g_state.offlineDeactivationKeyInput, 0, sizeof( g_state.offlineDeactivationKeyInput ) );
                }
                else
                {
                    g_state.message = "completeOfflineDeactivation failed (" + std::to_string( result.first ) + "): " + ToStd( result.second );
                }
            }
        }
    }

    // Testing aid: returns this device to the state the example starts in - offline and NOT ACTIVATED - so every scenario
    // can be replayed in one run. It disallows the connection and deletes this device's LOCAL activation records
    // (gem::ActivationService::deleteActivation, for activated, pending and expired ones alike). Deleting the activation
    // that holds the Core gate open is a state transition like any other, so the SDK reports onSdkNotActivated at once
    // and the watermark returns; allowing the connection afterwards runs auto-activation again. Nothing is released at
    // Magic Lane Services - those activations stay counted for the application until they expire - so a real device
    // must go through the offline deactivation ceremony above instead.
    void ResetToFirstRun()
    {
        if( g_state.allowOnline )
            SetConnectionAllowed( false );

        gem::ActivationInfoList activations = gem::ActivationService().getActivationsForProduct( gem::ProductID::CORE );
        for( unsigned i = 0; i < activations.size(); ++i )
            gem::ActivationService().deleteActivation( activations.at( i ).id );

        g_state.activationBlob.clear();
        g_state.deactivationBlob.clear();
        std::memset( g_state.licenseKeyInput, 0, sizeof( g_state.licenseKeyInput ) );
        std::memset( g_state.offlineActivationKeyInput, 0, sizeof( g_state.offlineActivationKeyInput ) );
        std::memset( g_state.offlineDeactivationKeyInput, 0, sizeof( g_state.offlineDeactivationKeyInput ) );
        g_state.message = "Reset done - offline, no local activations. Replay any scenario above.";
    }

    void DrawReset()
    {
        ImGui::Separator();
        ImGui::TextUnformatted( "4. Reset to first run (testing aid)" );
        ImGui::TextWrapped( "Disallows the connection and deletes this device's local activation records, so the SDK reports NOT ACTIVATED like on a "
                            "first run and every scenario above can be replayed without restarting. Only the local records are dropped - nothing is "
                            "released at Magic Lane Services; a real device must use the offline deactivation above." );

        if( ImGui::Button( "Reset to first run" ) )
            ResetToFirstRun();
    }

    auto getUiRender()
    {
        return std::bind(
            []( gem::StrongPointer<gem::MapView> mapView )
            {
                // React to whatever the SDK reported since the last frame (the callbacks only set atomics).
                static int lastSeenCounter = -1;
                const int counter = g_state.notificationCounter.load();
                if( counter != lastSeenCounter )
                {
                    lastSeenCounter = counter;
                    if( g_state.notActivated.load() )
                        g_state.lastNotification = std::string( "onSdkNotActivated (" ) + ReasonToText( gem::ESdkNotActivatedReason( g_state.notActivatedReason.load() ) ) + ")";
                    else if( counter > 0 )
                        g_state.lastNotification = "onSdkActivated";
                }
                ApplyWatermark( mapView );

                const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos( ImVec2( main_viewport->WorkPos.x + 0, main_viewport->WorkPos.y + 20 ), ImGuiCond_FirstUseEver );
                ImGui::Begin( "Activation modes", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings );
                ImGui::PushTextWrapPos( ImGui::GetCursorPosX() + 460.f ); // wrap width for TextWrapped in an auto-resizing window

                DrawStatus();
                DrawConnectivity();
                DrawOfflineActivation();
                DrawOfflineDeactivation();
                DrawReset();

                ImGui::PopTextWrapPos();
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

    // Register for the activation-state notifications BEFORE initializing the SDK: with a token but no connection the
    // SDK reports "not activated" already during initialization. The callbacks may run on SDK threads, so they only
    // record the state; the UI thread reacts to it in getUiRender().
    Environment::GetInstance().SetActivationStateCallbacks(
        []( gem::ESdkNotActivatedReason reason )
        {
            g_state.notActivatedReason.store( int( reason ) );
            g_state.notActivated.store( true );
            g_state.notificationCounter.fetch_add( 1 );
        },
        []()
        {
            g_state.notActivated.store( false );
            g_state.notificationCounter.fetch_add( 1 );
        } );

    // Sdk objects can be created & used below this line.
    // goOnline = false: start with the internet connection DISALLOWED, like a device without connectivity, so the
    // not-activated state (and the manual offline ceremony) can be exercised. Allow it from the panel to see
    // auto-activation take over.
    Environment::SdkSession session( projectApiToken, { argc > 1 && argv[1][0] != '-' ? argv[1] : "" } /* SDK API debug logging path */, "", /*goOnline*/ false );

    if( GEM_GET_API_ERROR() != gem::KNoError ) // check for errors after session creation
        return GEM_GET_API_ERROR();

    // If the SDK is already activated from a previous run, nothing was reported; make sure the panel starts consistent.
    if( gem::ActivationService().isActive( gem::ProductID::CORE ) )
        g_state.notActivated.store( false );

    // Create an interactive map view
    CTouchEventListener pTouchEventListener;

    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "ActivationModes", &pTouchEventListener, getUiRender() ) );
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
