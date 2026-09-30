// SPDX-FileCopyrightText: 2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"
#include "Icons.h"
#include "Listeners.h"

#include <API/GEM_ContentStore.h>
#include <API/GEM_MapView.h>
#include <API/GEM_NavigationListener.h>
#include <API/GEM_NavigationService.h>
#include <API/GEM_RoutingService.h>
#include <API/GEM_SdkSettings.h>
#include <API/GEM_Sounds.h>

#include <imgui.h>

// MA_NO_NULL: no silent fallback to the Null backend when there is no audio device.
// MA_NO_RESOURCE_MANAGER: no sound files are loaded, so no resource manager and no job thread.
#define MA_NO_NULL
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

static_assert( MA_VERSION_MAJOR > 0 || MA_VERSION_MINOR > 11 || ( MA_VERSION_MINOR == 11 && MA_VERSION_REVISION >= 23 ),
               "miniaudio 0.11.23 or newer is required (ma_dr_mp3dec_frame_info::sample_rate)" );

namespace
{
    constexpr float kSimulationSpeed = 2.f;        // x real time
    constexpr int kRoadblockLengthM = 200;         // road blocked ahead of the position by "Add roadblock"
    constexpr int kStoreListMaxRetries = 5;        // the store rate-limits requests (KBusy): retried with a growing delay
    constexpr double kStoreListRetryStepSec = 2.0; // retry n waits n * step
    constexpr double kPlaybackStallGraceSec = 2.0; // an instruction still not finished this long after its duration is given up
    constexpr ImVec4 kErrorColor( 1.f, 0.4f, 0.4f, 1.f );
    constexpr ImVec4 kWarningColor( 1.f, 0.7f, 0.2f, 1.f );

    ////////////////////////////////////////////////////////////////////////
    // 1. Voice output: the MP3 decoder, the sound player and its registration with the sound playing service
    ////////////////////////////////////////////////////////////////////////

    // A voice instruction is several phrase MP3 files glued together by the SDK ("Nach 250 Metern" + "rechts abbiegen"). The
    // frames are decoded one by one with the minimp3 core: miniaudio's high level decoder (ma_decoder, ma_decode_memory) stops
    // at the frame count of a Xing / Info header of the first file, i.e. after the first phrase.
    bool DecodeMp3( const std::vector<unsigned char>& mp3, std::vector<ma_int16>& pcm, ma_uint32& channels, ma_uint32& sampleRate )
    {
        ma_dr_mp3dec decoder;
        ma_dr_mp3dec_init( &decoder );
        ma_int16 frame[MA_DR_MP3_MAX_SAMPLES_PER_FRAME];
        channels = sampleRate = 0;
        for( size_t pos = 0; pos < mp3.size(); )
        {
            ma_dr_mp3dec_frame_info info;
            const int samples = ma_dr_mp3dec_decode_frame( &decoder, mp3.data() + pos, int( mp3.size() - pos ), frame, &info );
            if( info.frame_bytes <= 0 )
                break;
            pos += size_t( info.frame_bytes );
            if( samples <= 0 || ( channels && ma_uint32( info.channels ) != channels ) )
                continue; // a phrase with another channel count cannot go into the same PCM buffer
            channels = ma_uint32( info.channels );
            sampleRate = ma_uint32( info.sample_rate );
            pcm.insert( pcm.end(), frame, frame + samples * info.channels );
        }
        return !pcm.empty();
    }

    // Sound player for the voice instructions. The contract with the sound playing service:
    //  - Play() receives a whole instruction as one ISoundSource, which the player owns and must Release()
    //  - the service ignores the return value of Play() and waits for exactly one listener->notifyComplete() per Play(), on the
    //    SDK thread: Pump() sends it, also for a failed Play() (the player never notifies from inside the service's Play() call:
    //    the nested completion would start the next queued instruction while this one is still being processed)
    //  - Cancel() must send notifyComplete( KCancel ), synchronously is fine: the service stays in cancelling state until it arrives
    //  - notifyComplete() may re-enter Play(): the prompt is taken out of the list first, the notification comes last
    class VoicePlayer : public gem::ISoundPlayer
    {
    public:
        VoicePlayer()
        {
            m_engineOk = ma_engine_init( nullptr, &m_engine ) == MA_SUCCESS;
        }

        ~VoicePlayer() override
        {
            m_prompts.clear(); // the sounds before the engine
            if( m_engineOk )
                ma_engine_uninit( &m_engine );
        }

        int Play( gem::ISoundSource* source, gem::ISoundPlayingPreferences const& preferences, gem::ISoundPlayingListener* listener ) override
        {
            // Read() returns the number of bytes read or a negative error; the player owns the source
            std::vector<unsigned char> mp3( size_t( std::max( 0, source->GetSize() ) ) );
            mp3.resize( size_t( std::max( 0, source->Read( mp3.data(), int( mp3.size() ) ) ) ) );
            source->Release();

            auto prompt = std::make_unique<Prompt>();
            prompt->listener = listener;
            const float gain = std::pow( float( preferences.GetVolume() ) / float( gem::MaxVolume ), 2.f ); // volume 0..10, perceptual curve
            prompt->playing = m_engineOk && DecodeMp3( mp3, prompt->pcm, prompt->channels, prompt->sampleRate ) && prompt->start( m_engine, gain );
            if( prompt->playing )
                listener->notifyStart( false );
            m_prompts.push_back( std::move( prompt ) );
            return gem::KNoError;
        }

        int Cancel( gem::ISoundPlayingListener* listener ) override
        {
            for( size_t i = 0; i < m_prompts.size(); ++i )
                if( m_prompts[i]->listener == listener )
                {
                    Complete( i, gem::error::KCancel );
                    return gem::KNoError;
                }
            return gem::error::KNotFound;
        }

        bool CanPlayMimeType( gem::EMimeType mimeType ) const override
        {
            return mimeType == gem::EMimeType::MT_Mp3; // human voices; never claim MT_Tts (text to be synthesized)
        }

        int PlayingSoundsCount() const override
        {
            return int( m_prompts.size() );
        }

        void Release() override
        {
            delete this;
        }

        // Sends the pending notifications. Call it on the SDK thread, e.g. from the UI callback.
        void Pump()
        {
            for( size_t i = 0; i < m_prompts.size(); )
            {
                const Prompt& p = *m_prompts[i];
                const bool ended = p.ended();
                if( p.playing && !ended && !p.stalled() )
                    ++i;
                else
                    Complete( i, ended ? gem::KNoError : gem::error::KGeneral );
            }
        }

        bool HasAudioOutput() const
        {
            return m_engineOk;
        }

    private:
        // Takes the prompt out of the list first (stops the sound), notifies last: notifyComplete() may re-enter Play()
        void Complete( size_t i, int result )
        {
            gem::ISoundPlayingListener* listener = m_prompts[i]->listener;
            m_prompts.erase( m_prompts.begin() + i );
            listener->notifyComplete( result );
        }

        struct Prompt
        {
            gem::ISoundPlayingListener* listener = nullptr;
            std::vector<ma_int16> pcm;
            ma_uint32 channels = 0, sampleRate = 0;
            ma_audio_buffer_ref buffer; // wraps pcm, nothing to release
            ma_sound sound;
            bool playing = false; // the sound is initialized and started
            std::chrono::steady_clock::time_point started;

            bool start( ma_engine& engine, float gain )
            {
                ma_audio_buffer_ref_init( ma_format_s16, channels, pcm.data(), pcm.size() / channels, &buffer );
                buffer.sampleRate = sampleRate; // the engine converts the 22.05 kHz mono voice to the device format
                if( ma_sound_init_from_data_source( &engine, &buffer, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, &sound ) != MA_SUCCESS )
                    return false;
                ma_sound_set_volume( &sound, gain );
                started = std::chrono::steady_clock::now();
                if( ma_sound_start( &sound ) == MA_SUCCESS )
                    return true;
                ma_sound_uninit( &sound );
                return false;
            }

            bool ended() const
            {
                return playing && ma_sound_at_end( &sound ); // set by the audio thread, safe to poll
            }

            // The output stopped consuming the data (e.g. the sound server died): the prompt is given up, or the navigation
            // waits for it forever.
            bool stalled() const
            {
                const double duration = channels && sampleRate ? double( pcm.size() / channels ) / sampleRate : 0.0;
                const double elapsed = std::chrono::duration<double>( std::chrono::steady_clock::now() - started ).count();
                return playing && elapsed > duration + kPlaybackStallGraceSec;
            }

            ~Prompt()
            {
                if( playing )
                    ma_sound_uninit( &sound ); // stops the sound and detaches it from the audio thread
            }
        };

        ma_engine m_engine;
        bool m_engineOk = false;
        std::vector<std::unique_ptr<Prompt>> m_prompts;
    };

    // The sound playing service requires a listener for every Play(); this example has nothing to do with the notifications.
    class VoiceListener : public gem::ISoundPlayingListener
    {
    public:
        void notifyStart( bool ) override {}
        void notifyComplete( int, gem::String ) override {}
    };

    // The sound playing service (a new one, owned by the application) with our player registered for the MP3 voice instructions.
    // The service keeps a raw pointer to the player: it is released before the player.
    class VoiceOutput
    {
    public:
        VoiceOutput() = default;
        VoiceOutput( const VoiceOutput& ) = delete;
        VoiceOutput& operator=( const VoiceOutput& ) = delete;

        ~VoiceOutput()
        {
            if( m_service )
                m_service->Release();
            if( m_player )
                m_player->Release();
            if( m_preferences )
                m_preferences->Release();
        }

        int Init()
        {
            int err = gem::ISdk::Instance()->Produce( m_service );
            if( err == gem::KNoError )
                err = gem::ISdk::Instance()->Produce( m_preferences );
            if( err != gem::KNoError )
                return err;
            m_preferences->SetVolume( gem::MaxVolume );
            m_player = new VoicePlayer();
            m_service->SetPlayer( m_player, gem::EMimeType::MT_Mp3 );
            return gem::KNoError;
        }

        // The service joins the phrases of the instruction into one MP3 buffer and calls VoicePlayer::Play()
        void Play( gem::ISound const& sound )
        {
            if( !sound.IsValid() )
                return;
            if( const int err = m_service->Play( sound, &m_listener, m_preferences ); err != gem::KNoError )
                GEM_LOGW( "Voice instruction not played: %d", err );
        }

        bool IsIdle() const
        {
            return m_service->PlayingSoundsCount() == 0;
        }

        // Delivers the finished instructions to the service. Call it on the SDK thread.
        void Pump()
        {
            m_player->Pump();
        }

        bool HasAudioOutput() const
        {
            return m_player->HasAudioOutput();
        }

        gem::ISoundPlayingPreferences& Preferences()
        {
            return *m_preferences;
        }

    private:
        gem::ISoundPlayingService* m_service = nullptr;
        gem::ISoundPlayingPreferences* m_preferences = nullptr;
        VoicePlayer* m_player = nullptr;
        VoiceListener m_listener;
    };

    ////////////////////////////////////////////////////////////////////////
    // 2. Application state
    ////////////////////////////////////////////////////////////////////////

    // Plain values: SDK objects (e.g. gem::Coordinates) must not be created before the SDK is loaded.
    struct Waypoint
    {
        const char* name;
        double latitude, longitude;
    };

    // Munich, Haidhausen (3 km): many instructions to hear - turns, a roundabout, a U-turn and two intermediate destinations
    const Waypoint kRoute[] = {
        { "Orleansplatz", 48.12810, 11.60270 },
        { "Pariser Platz", 48.12960, 11.59680 },
        { "Rosenheimer Platz", 48.12910, 11.59070 },
        { "Haidhausen", 48.12650, 11.59550 },
    };

    // Keep state alive across frames
    struct HumanVoiceUiState
    {
        VoiceOutput voice;

        // Human voices of the content store
        gem::StrongPointer<ProgressListener> listListener = gem::StrongPointerFactory<ProgressListener>();
        bool listPending = false;    // store list requested, result not consumed yet
        bool refreshPressed = false; // voices is the store list (else the downloaded voices)
        int listRetries = 0;
        double listRetryTime = 0; // ImGui::GetTime() of the scheduled retry, 0 = none
        gem::ContentStoreItemList voices;
        std::vector<gem::StrongPointer<ProgressListener>> downloads; // the running downloads
        std::string error;                                           // of the last download / delete
        bool voiceSelected = false;
        ImGuiTextFilter filter;

        // Route + simulation
        gem::StrongPointer<ProgressListener> routeListener = gem::StrongPointerFactory<ProgressListener>();
        gem::RouteList routes;
        bool routeRequested = false;
        bool routeShown = false;
        gem::StrongPointer<gem::INavigationListener> navigationListener;
    };

    ////////////////////////////////////////////////////////////////////////
    // 3. Navigation: the voice instructions of the simulation
    ////////////////////////////////////////////////////////////////////////

    class VoiceNavigationListener : public gem::INavigationListener
    {
    public:
        explicit VoiceNavigationListener( VoiceOutput& voice )
            : m_voice( voice )
        {
        }

        // The navigation / simulation asks the application to play a voice instruction: the SDK never plays it itself.
        void onNavigationSound( gem::ISound const& sound ) override
        {
            m_voice.Play( sound );
        }

        // Asked before each instruction: the navigation delays it while the previous one is still being spoken.
        bool canPlayNavigationSound() override
        {
            return m_voice.IsIdle();
        }

        void onNavigationStarted() override
        {
            GEM_LOGI( "Simulation started" );
        }
        void onNavigationInstructionUpdated( const gem::NavigationInstruction& ) override {}
        void onWaypointReached( const gem::Landmark& ) override {}
        void onDestinationReached( const gem::Landmark& ) override
        {
            GEM_LOGI( "Destination reached" );
        }
        void onNavigationError( int error ) override
        {
            if( error != gem::error::KCancel )
                GEM_LOGE( "Navigation error: %d", error );
        }
        void onRouteUpdated( const gem::Route& ) override {}

    private:
        VoiceOutput& m_voice;
    };

    ////////////////////////////////////////////////////////////////////////
    // 4. Human voices of the content store
    ////////////////////////////////////////////////////////////////////////

    // Requests the human voices of the content store (the result is consumed in PollVoiceList)
    void RequestVoiceList( HumanVoiceUiState& s )
    {
        s.listPending = true;
        s.refreshPressed = true;
        s.listListener->Reset();
        const int err = gem::ContentStore().asyncGetStoreContentList( gem::CT_HumanVoice, s.listListener );
        if( err != gem::KNoError )
            s.listListener->notifyComplete( err );
    }

    // The store rate-limits requests (HTTP 429 -> KBusy), e.g. without an API token while the SDK starts: retried later.
    void PollVoiceList( HumanVoiceUiState& s )
    {
        if( !s.listPending || !s.listListener->IsFinished() )
            return;
        if( s.listListener->GetError() == gem::error::KBusy && s.listRetries < kStoreListMaxRetries )
        {
            if( s.listRetryTime == 0 )
                s.listRetryTime = ImGui::GetTime() + kStoreListRetryStepSec * ( s.listRetries + 1 );
            else if( ImGui::GetTime() >= s.listRetryTime )
            {
                ++s.listRetries;
                s.listRetryTime = 0;
                RequestVoiceList( s );
            }
            return;
        }
        // The store list includes the downloaded voices; offline only the downloaded ones are available.
        s.listPending = false;
        s.listRetries = 0;
        s.voices = s.listListener->GetError() == gem::KNoError ? gem::ContentStore().getStoreContentList( gem::CT_HumanVoice ).first
                                                               : gem::ContentStore().getLocalContentList( gem::CT_HumanVoice );
    }

    // The SDK does not remember the selected voice: select a downloaded one at every start.
    bool SelectFirstDownloadedVoice()
    {
        for( auto& item : gem::ContentStore().getLocalContentList( gem::CT_HumanVoice ) )
            if( item.isCompleted() && GEM_TEST_NOEXCEPT( gem::SdkSettings().setVoiceByPath( item.getFileName() ) ) == gem::KNoError )
                return true;
        return false;
    }

    // When a download finishes and no voice is selected yet, use the downloaded voice right away.
    void PollDownloads( HumanVoiceUiState& s )
    {
        for( auto it = s.downloads.begin(); it != s.downloads.end(); )
            if( ( *it )->IsFinished() )
            {
                it = s.downloads.erase( it );
                if( !s.voiceSelected )
                    s.voiceSelected = SelectFirstDownloadedVoice();
            }
            else
                ++it;
    }

    ////////////////////////////////////////////////////////////////////////
    // 5. Route
    ////////////////////////////////////////////////////////////////////////

    // Functions, not booleans: the Calculate route button changes the answer within the frame.
    bool IsRouteCalculating( HumanVoiceUiState& s )
    {
        return s.routeRequested && !s.routeListener->IsFinished();
    }

    bool HasRoute( HumanVoiceUiState& s )
    {
        return s.routeRequested && !IsRouteCalculating( s ) && s.routeListener->GetError() == gem::KNoError && !s.routes.empty();
    }

    void CalculateRoute( HumanVoiceUiState& s )
    {
        gem::LandmarkList waypoints;
        for( const Waypoint& w : kRoute )
            waypoints.push_back( gem::Landmark( w.name, { w.latitude, w.longitude } ) );
        s.routes = gem::RouteList();
        s.routeListener->Reset();
        s.routeRequested = true;
        s.routeShown = false;
        const int err = gem::RoutingService().calculateRoute(
            s.routes, waypoints, gem::RoutePreferences().setTransportMode( gem::RTM_Car ).setRouteType( gem::RT_Fastest ).setAlternativesSchema( gem::AS_Never ), s.routeListener );
        if( err != gem::KNoError )
            s.routeListener->notifyComplete( err );
    }

    // Shows the calculated route on the map, once
    void PollRoute( HumanVoiceUiState& s, gem::StrongPointer<gem::MapView> mapView )
    {
        if( !HasRoute( s ) || s.routeShown )
            return;
        s.routeShown = true;
        mapView->preferences().routes().clear();
        mapView->preferences().routes().add( s.routes[0], true );
        mapView->centerOnRoute( s.routes[0] );
    }

    ////////////////////////////////////////////////////////////////////////
    // 6. UI
    ////////////////////////////////////////////////////////////////////////

    // The State and Actions cells of a voice: the buttons depend on the download state
    void DrawVoiceRow( HumanVoiceUiState& s, gem::ContentStoreItem item, bool selected )
    {
        const auto status = item.getStatus();
        const bool downloaded = status == gem::EContentStoreItemStatus::CIS_Completed;
        const bool notDownloaded = status == gem::EContentStoreItemStatus::CIS_Unavailable;
        const bool paused = status == gem::EContentStoreItemStatus::CIS_Paused;
        const bool downloading = !downloaded && !notDownloaded; // queued, running or paused

        ImGui::TableNextColumn();
        if( downloaded )
            ImGui::TextUnformatted( selected ? "in use" : "downloaded" );
        else if( notDownloaded )
            ImGui::Text( "%.1f MB", item.getTotalSize() / ( 1024.0 * 1024.0 ) ); // download size
        else
            ImGui::ProgressBar( std::max( 0, item.getDownloadProgress() ) / 100.f, ImVec2( 80, 0 ), paused ? "paused" : nullptr );

        ImGui::TableNextColumn();
        if( notDownloaded || paused )
        {
            if( ImGui::SmallButton( paused ? "Resume" : "Download" ) )
            {
                auto listener = gem::StrongPointerFactory<ProgressListener>();
                const int err = item.asyncDownload( listener, gem::EDataSavePolicy::UseDefault, true );
                s.error = err == gem::KNoError ? "" : "Download could not start (" + std::to_string( err ) + ")";
                if( err == gem::KNoError )
                    s.downloads.push_back( listener );
            }
        }
        else if( downloading && ImGui::SmallButton( "Pause" ) )
            item.pauseDownload();
        if( downloading )
        {
            ImGui::SameLine();
            if( ImGui::SmallButton( "Cancel" ) )
                item.cancelDownload();
        }
        if( downloaded )
        {
            if( !selected )
            {
                // Use the voice for the navigation instructions: the path of the downloaded voice file.
                if( ImGui::SmallButton( "Use" ) )
                    s.voiceSelected = GEM_TEST_NOEXCEPT( gem::SdkSettings().setVoiceByPath( item.getFileName() ) ) == gem::KNoError;
                ImGui::SameLine();
            }
            if( ImGui::SmallButton( "Delete" ) )
            {
                const int err = item.deleteContent();
                s.error = err == gem::KNoError ? "" : "Delete failed (" + std::to_string( err ) + ")";
                // The SDK keeps the deleted voice selected: switch to another downloaded voice, if any.
                if( err == gem::KNoError && selected )
                    s.voiceSelected = SelectFirstDownloadedVoice();
            }
        }
    }

    void DrawVoicesWindow( HumanVoiceUiState& s )
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos( ImVec2( viewport->WorkPos.x + 10, viewport->WorkPos.y + 10 ), ImGuiCond_FirstUseEver );
        ImGui::SetNextWindowSize( ImVec2( 420, 330 ), ImGuiCond_FirstUseEver );
        if( ImGui::Begin( "Human voices" ) )
        {
            if( !s.listPending )
            {
                if( ImGui::SmallButton( "Refresh" ) )
                    RequestVoiceList( s );
                ImGui::SameLine();
            }
            if( s.listPending )
                ImGui::TextUnformatted( s.listRetryTime != 0 ? "Voice store busy, retrying..." : "Loading the voice list..." );
            else if( !s.refreshPressed )
                ImGui::TextWrapped( "Press Refresh to list voices" );
            else if( s.listListener->GetError() != gem::KNoError )
                ImGui::TextWrapped( "Store list failed (%d): showing the downloaded voices", s.listListener->GetError() );
            else
                ImGui::Text( "%d voices", int( s.voices.size() ) );
            if( !s.error.empty() )
                ImGui::TextColored( kErrorColor, "%s", s.error.c_str() );

            const gem::Voice voice = gem::SdkSettings().getVoice();
            const std::string selectedFile = voice.getFileName().toStdString();
            ImGui::Text( "Navigation voice: %s", s.voiceSelected ? voice.getName().toStdString().c_str() : "none (download one)" );

            // ImGuiTextFilter: case insensitive, "deu, eng-GBR" matches either
            ImGui::SetNextItemWidth( -FLT_MIN );
            if( ImGui::InputTextWithHint( "##filter", "filter: deu, eng-GBR, Hannah", s.filter.InputBuf, IM_ARRAYSIZE( s.filter.InputBuf ) ) )
                s.filter.Build();

            if( ImGui::BeginTable( "voices", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit ) )
            {
                ImGui::TableSetupScrollFreeze( 0, 1 );
                ImGui::TableSetupColumn( "Voice", ImGuiTableColumnFlags_WidthFixed, 76 );
                ImGui::TableSetupColumn( "Language", ImGuiTableColumnFlags_WidthFixed, 72 );
                ImGui::TableSetupColumn( "State", ImGuiTableColumnFlags_WidthFixed, 82 );
                ImGui::TableSetupColumn( "Actions", ImGuiTableColumnFlags_WidthStretch );
                ImGui::TableHeadersRow();

                for( size_t i = 0; i < s.voices.size(); ++i )
                {
                    auto item = s.voices[i];
                    const std::string name = item.getName().toStdString();
                    const std::string language = item.getLanguage().getLanguageCode().toStdString() + "-" + item.getLanguage().getRegionCode().toStdString();
                    if( !s.filter.PassFilter( name.c_str() ) && !s.filter.PassFilter( language.c_str() ) )
                        continue;

                    ImGui::PushID( int( i ) );
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( name.c_str() );
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( language.c_str() );
                    DrawVoiceRow( s, item, s.voiceSelected && item.isCompleted() && item.getFileName().toStdString() == selectedFile );
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    // Volume slider with a speaker icon showing the level
    void DrawVolumeSlider( gem::ISoundPlayingPreferences& preferences )
    {
        int volume = preferences.GetVolume();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted( volume == gem::MinVolume ? ICON_FA_VOLUME_XMARK : volume <= gem::MaxVolume / 2 ? ICON_FA_VOLUME_LOW : ICON_FA_VOLUME_HIGH );
        if( ImGui::IsItemHovered() )
            ImGui::SetTooltip( "Volume" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( -FLT_MIN );
        if( ImGui::SliderInt( "##volume", &volume, gem::MinVolume, gem::MaxVolume ) )
            preferences.SetVolume( volume );
    }

    void DrawRouteWindow( HumanVoiceUiState& s, gem::StrongPointer<gem::MapView> mapView )
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos( ImVec2( viewport->WorkPos.x + viewport->WorkSize.x - 360, viewport->WorkPos.y + 10 ), ImGuiCond_FirstUseEver );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 350, 0 ), ImVec2( 350, FLT_MAX ) ); // fixed width, height fits the content
        if( ImGui::Begin( "Route & voice instructions", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
        {
            if( !gem::NavigationService().isSimulationActive() )
            {
                ImGui::TextWrapped( "Munich, Haidhausen: turns, a roundabout, a U-turn and two intermediate destinations" );
                ImGui::BeginDisabled( IsRouteCalculating( s ) || HasRoute( s ) );
                if( ImGui::Button( "Calculate route" ) )
                    CalculateRoute( s );
                ImGui::EndDisabled();

                if( IsRouteCalculating( s ) )
                    ImGui::TextUnformatted( "Calculating..." );
                else if( s.routeRequested && !HasRoute( s ) )
                    ImGui::Text( "Route calculation failed (%d)", s.routeListener->GetError() );
                else if( HasRoute( s ) )
                {
                    ImGui::SameLine();
                    if( ImGui::Button( "Start simulation" ) &&
                        gem::NavigationService().startSimulation( s.routes[0], s.navigationListener, gem::ProgressListener(), kSimulationSpeed ) == gem::KNoError )
                    {
                        s.routeRequested = false; // the navigation recalculates this route (roadblocks): calculate a new one afterwards
                        mapView->startFollowingPosition();
                    }
                    if( !s.voiceSelected )
                    {
                        ImGui::PushStyleColor( ImGuiCol_Text, kWarningColor );
                        ImGui::TextWrapped( "No voice selected: the instructions are not spoken" );
                        ImGui::PopStyleColor();
                    }
                }
            }
            else
            {
                if( ImGui::Button( "Stop simulation" ) )
                    gem::NavigationService().cancelNavigation( s.navigationListener );
                ImGui::SameLine();
                // Blocks the road ahead: the navigation recalculates the route and announces the detour. The roadblock is
                // temporary, the SDK removes it when the navigation ends.
                if( ImGui::Button( "Add roadblock" ) && GEM_TEST_NOEXCEPT( gem::NavigationService().setNavigationRoadBlock( kRoadblockLengthM ) ) != gem::KNoError )
                    GEM_LOGW( "Roadblock not set: %d", GEM_GET_API_ERROR() );
            }

            ImGui::Separator();
            if( !s.voice.HasAudioOutput() )
                ImGui::TextColored( kErrorColor, "No audio output device" );
            DrawVolumeSlider( s.voice.Preferences() );
        }
        ImGui::End();
    }

    std::function<void( gem::StrongPointer<gem::MapView> )> getUiRender( const std::shared_ptr<HumanVoiceUiState>& state )
    {
        // The UI callback runs on the SDK thread
        return [state]( gem::StrongPointer<gem::MapView> mapView )
        {
            state->voice.Pump();     // 1. the finished instructions -> sound playing service
            PollVoiceList( *state ); // 2. state transitions
            PollDownloads( *state );
            PollRoute( *state, mapView );
            DrawVoicesWindow( *state ); // 3. drawing
            DrawRouteWindow( *state, mapView );
        };
    }
} // namespace

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

    auto state = std::make_shared<HumanVoiceUiState>();

    // Sound playing service + our player for the MP3 voice instructions
    if( const int err = state->voice.Init(); err != gem::KNoError )
    {
        GEM_LOGE( "Cannot create the sound playing service (%d)", err );
        return err;
    }
    if( !state->voice.HasAudioOutput() )
        GEM_LOGW( "No audio output device: voice instructions are not played" );

    // Human voices: select a downloaded one and list the downloaded ones (Refresh requests the list of the content store)
    state->voiceSelected = SelectFirstDownloadedVoice();
    state->voices = gem::ContentStore().getLocalContentList( gem::CT_HumanVoice );

    // Navigation listener: receives the voice instructions (kept alive by the state, the SDK holds a weak reference)
    state->navigationListener = gem::StrongPointerFactory<VoiceNavigationListener>( state->voice );

    // Create an interactive map view
    CTouchEventListener pTouchEventListener;

    gem::StrongPointer<gem::MapView> mapView = gem::MapView::produce(
        session.produceOpenGLContext( Environment::WindowFrameworks::ImGUI, "HumanVoiceNavigation", &pTouchEventListener, getUiRender( state ) ) );

    if( !mapView )
    {
        GEM_LOGE( "Error creating gem::MapView: %d", GEM_GET_API_ERROR() );
    }

    WAIT_UNTIL_WINDOW_CLOSE();

    // The UI callback keeps the state alive: it is destroyed together with the window by SdkSession, before the SDK is released
    gem::NavigationService().cancelNavigation( state->navigationListener );

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
