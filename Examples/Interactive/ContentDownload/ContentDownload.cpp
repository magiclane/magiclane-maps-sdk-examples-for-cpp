// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_MapView.h>
#include <API/GEM_ContentStore.h>

#include "Listeners.h"

#include <imgui.h>
#include <memory>

namespace
{
    // Keep state alive across frames (ImGui render callback is executed every frame).
    struct ContentDownloadUiState
    {
        gem::EContentType m_contentType = gem::EContentType::CT_RoadMap;

        bool m_queryIssued = false;
        bool m_hasContentList = false;

        gem::ContentStoreItemList m_storeItems;
        int m_selectedIndex = -1;

        ProgressListener m_listListener;
        bool m_isListing = false;

        ProgressListener m_downloadListener;
        bool m_isDownloading = false;

        int m_lastStartResult = gem::KNoError;
        gem::String m_lastHint;
    };

    static const char* ToString( gem::EContentStoreItemStatus s )
    {
        switch ( s )
        {
        case gem::EContentStoreItemStatus::CIS_Unavailable: return "Unavailable";
        case gem::EContentStoreItemStatus::CIS_Completed: return "Completed";
        case gem::EContentStoreItemStatus::CIS_Paused: return "Paused";
        case gem::EContentStoreItemStatus::CIS_DownloadQueued: return "Queued";
        case gem::EContentStoreItemStatus::CIS_DownloadWaitingNetwork: return "Waiting network";
        case gem::EContentStoreItemStatus::CIS_DownloadWaitingFreeNetwork: return "Waiting free network";
        case gem::EContentStoreItemStatus::CIS_DownloadRunning: return "Downloading";
        case gem::EContentStoreItemStatus::CIS_UpdateWaiting: return "Update waiting";
        default: return "Unknown";
        }
    }

    static gem::ContentStoreItemList GetLocalList( gem::EContentType type )
    {
        return gem::ContentStore().getLocalContentList( int( type ) );
    }

    static void StartListingStoreContent( ContentDownloadUiState& state, gem::EContentType type )
    {
        state.m_isListing = true;
        state.m_hasContentList = false;
        state.m_storeItems.clear();
        state.m_selectedIndex = -1;

        state.m_listListener.Reset();
        gem::ContentStore().asyncGetStoreContentList( int( type ), &state.m_listListener );
    }

    static void PollListing( ContentDownloadUiState& state )
    {
        if ( !state.m_isListing )
            return;

        if ( state.m_listListener.IsFinished() )
        {
            state.m_isListing = false;

            if ( state.m_listListener.GetError() == gem::KNoError )
            {
                // getStoreContentList returns pair<ContentStoreItemList, bool>
                auto res = gem::ContentStore().getStoreContentList( int( state.m_contentType ) );
                state.m_storeItems = res.first;
                state.m_hasContentList = res.second;
                state.m_selectedIndex = ( state.m_hasContentList && !state.m_storeItems.empty() ) ? 0 : -1;
            }
            else
            {
                state.m_lastHint = state.m_listListener.GetHint();
                state.m_hasContentList = false;
            }
        }
    }

    static void PollDownload( ContentDownloadUiState& state )
    {
        if ( !state.m_isDownloading )
            return;

        if ( state.m_downloadListener.IsFinished() )
        {
            state.m_isDownloading = false;
            state.m_lastHint = state.m_downloadListener.GetHint();
        }
    }
}

static std::function<void( gem::StrongPointer<gem::MapView> )> getUiRender( const std::shared_ptr<ContentDownloadUiState>& state )
{
    return std::bind( [state]( gem::StrongPointer<gem::MapView> /*view*/ )
        {
            if ( !state )
                return;

            // Auto-query once.
            if ( !state->m_queryIssued )
            {
                state->m_queryIssued = true;
                StartListingStoreContent( *state, state->m_contentType );
            }

            PollListing( *state );
            PollDownload( *state );

            ImGui::SetNextWindowSize( ImVec2( 520, 520 ), ImGuiCond_FirstUseEver );
            if ( ImGui::Begin( "ContentDownload" ) )
            {
                ImGui::TextUnformatted( "Download a roadmap package from the ContentStore." );
                ImGui::Separator();

                ImGui::Text( "Content type: %s", "CT_RoadMap" );
                ImGui::SameLine();
                if ( ImGui::Button( "Refresh store list" ) && !state->m_isListing )
                    StartListingStoreContent( *state, state->m_contentType );

                // Listing status
                if ( state->m_isListing )
                {
                    ImGui::Text( "Listing store content... %d%%", state->m_listListener.GetProgressValue() );
                    ImGui::ProgressBar( state->m_listListener.GetProgressValue() / 100.0f, ImVec2( -1, 0 ) );
                }
                else if ( !state->m_hasContentList )
                {
                    if ( state->m_listListener.GetError() != gem::KNoError )
                        ImGui::Text( "Listing failed (err=%d): %s", state->m_listListener.GetError(), state->m_lastHint.toStdString().c_str() );
                    else
                        ImGui::TextUnformatted( "No store list (press Refresh)." );
                }

                ImGui::Separator();

                // Local content summary
                {
                    auto local = GetLocalList( state->m_contentType );
                    int completed = 0;
                    for ( auto& it : local )
                        if ( it.getStatus() == gem::EContentStoreItemStatus::CIS_Completed )
                            ++completed;

                    ImGui::Text( "Local roadmaps: %d (completed: %d)", (int)local.size(), completed );
                }

                ImGui::Separator();

                // Store items list + selection
                if ( state->m_hasContentList )
                {
                    ImGui::Text( "Store roadmaps: %d", (int)state->m_storeItems.size() );

                    ImGui::BeginChild( "store_list", ImVec2( 0, 170 ), true );
                    for ( int i = 0; i < (int)state->m_storeItems.size(); ++i )
                    {
                        auto item = state->m_storeItems[(size_t)i];
                        const bool selected = ( i == state->m_selectedIndex );

                        std::string label;
                        auto title = item.getName();
                        if ( !title.empty() )
                            label = title.toStdString();
                        else
                            label = std::string( "Content #" ) + std::to_string( (long long)item.getId() );

                        label += " [";
                        label += ToString( item.getStatus() );
                        label += "]";

                        if ( ImGui::Selectable( label.c_str(), selected ) )
                            state->m_selectedIndex = i;
                    }
                    ImGui::EndChild();

                    ImGui::Separator();

                    // Selected item actions
                    if ( state->m_selectedIndex >= 0 && state->m_selectedIndex < (int)state->m_storeItems.size() )
                    {
                        auto item = state->m_storeItems[(size_t)state->m_selectedIndex];
                        auto status = item.getStatus();

                        ImGui::Text( "Selected id: %lld", (long long)item.getId() );
                        ImGui::Text( "Status: %s", ToString( status ) );

                        const bool canDownload =
                            ( status == gem::EContentStoreItemStatus::CIS_Unavailable ) ||
                            ( status == gem::EContentStoreItemStatus::CIS_Paused ) ||
                            ( status == gem::EContentStoreItemStatus::CIS_DownloadQueued ) ||
                            ( status == gem::EContentStoreItemStatus::CIS_DownloadWaitingNetwork ) ||
                            ( status == gem::EContentStoreItemStatus::CIS_DownloadWaitingFreeNetwork );

                        const bool canPause =
                            ( status == gem::EContentStoreItemStatus::CIS_DownloadRunning );

                        const bool canCancel = item.canDeleteContent();

                        if ( ImGui::Button( "Download / Resume" ) )
                        {
                            if ( !state->m_isDownloading && canDownload )
                            {
                                state->m_downloadListener.Reset();
                                state->m_lastStartResult =
                                    item.asyncDownload( &state->m_downloadListener, gem::EDataSavePolicy::UseDefault, false );

                                if ( state->m_lastStartResult == gem::KNoError )
                                {
                                    state->m_isDownloading = true;
                                    state->m_lastHint.clear();
                                }
                                else
                                {
                                    state->m_isDownloading = false;
                                    state->m_lastHint = "asyncDownload() failed";
                                }
                            }
                        }

                        ImGui::SameLine();
                        if ( ImGui::Button( "Pause" ) )
                        {
                            if ( canPause )
                                item.pauseDownload();
                        }

                        ImGui::SameLine();
                        if ( ImGui::Button( "Cancel / Delete" ) )
                        {
                            if ( canCancel )
                                item.deleteContent();
                        }

                        const int itemProgress = item.getDownloadProgress(); // -1 when unknown
                        if ( state->m_isDownloading || itemProgress >= 0 )
                        {
                            float p = itemProgress < 0 ? ( state->m_downloadListener.GetProgressValue() / 100.0f )
                                : ( itemProgress / 100.0f );

                            ImGui::Text( "Download progress: %d%%", itemProgress < 0 ? state->m_downloadListener.GetProgressValue() : itemProgress );
                            ImGui::ProgressBar( p, ImVec2( -1, 0 ) );
                        }

                        if ( state->m_downloadListener.IsFinished() )
                        {
                            ImGui::Text( "Last download result: err=%d %s",
                                state->m_downloadListener.GetError(),
                                state->m_lastHint.empty() ? "" : state->m_lastHint.toStdString().c_str() );
                        }
                        else if ( state->m_isDownloading )
                        {
                            ImGui::Text( "Downloading... (listener=%d%%)", state->m_downloadListener.GetProgressValue() );
                        }
                    }
                }
            }
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
	if ( value != nullptr )
		projectApiToken = value;
#endif

	// Sdk objects can be created & used below this line
	Environment::SdkSession session(projectApiToken, { argc > 1 ? argv[1] : "" }); // SDK API debug logging path 

	if (GEM_GET_API_ERROR() != gem::KNoError) // check for errors after session creation
		return GEM_GET_API_ERROR();

    auto uiState = std::make_shared<ContentDownloadUiState>();

	// Create an interactive map view
	CTouchEventListener pTouchEventListener;

	gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "ContentDownload", &pTouchEventListener, getUiRender( uiState ) ) );

    if ( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    WAIT_UNTIL_WINDOW_CLOSE();

    // Ensure SDK objects are destroyed before SdkSession uninitializes the SDK.
    uiState.reset();
    mapView = nullptr;

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
