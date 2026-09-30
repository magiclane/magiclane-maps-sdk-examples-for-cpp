// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#include <API/GEM_Sdk.h>
#include <API/GEM_Debug.h>

#include "Timer.h"
#include "Listeners.h"
#include "ApiLogger.h"
#include "OpenGLContext.h"

#include <functional>
#include <climits>

#define LOG_TAG "CppExamples"

#if defined( WIN32 ) || defined( WIN64 )
    #include <shlwapi.h>
#endif

class Environment
{
public:
    enum class WindowFrameworks
    {
        GLFW,
        SDL,
        ImGUI,
        LVGL,
        Win32EGL, // dependency-free native Win32 + EGL/ANGLE context (Windows only, see OpenGLContext_Win32EGL.h)
        Available
    };

public:
    class SdkSession
    {
    public:
        SdkSession( std::string token = "", std::string logFilePath = "", std::string sdkPluginPath = "", bool goOnline = true )
        {
            Environment::GetInstance().InitSDK( token, logFilePath, sdkPluginPath, goOnline );
        }

        ~SdkSession()
        {
            Environment::GetInstance().ReleaseSDK();
        }

        gem::StrongPointer<OpenGLContext> produceOpenGLContext( Environment::WindowFrameworks framework, std::string windowName = "", ITouchEventsListener* pListener = nullptr,
                                                                UICallbacks uiCallbacks = {}, gem::Size windowSize = gem::Size( 0, 0 ), int rotation = 0, float fontScale = 1.f,
                                                                float textureScale = 1.f )
        {
            auto oglContext = Environment::GetInstance().ProduceOpenGLContext( framework, windowName, pListener, uiCallbacks, windowSize, rotation );

            if( oglContext )
            {
                oglContext->setFontScaleFactor( fontScale );
                oglContext->setTextureScaleFactor( textureScale );
            }

            return oglContext;
        }
    };

public:
    static Environment& GetInstance()
    {
        static Environment instance;

        return instance;
    }

    // Resolve the project API token, most explicit source first:
    //   1. --token=<value> (or --token <value>) on the command line
    //   2. the GEM_TOKEN environment variable
    //   3. gem_token.txt next to the executable, then <home>/.magiclane/gem_token.txt
    //   4. the compile-time GEM_TOKEN definition (baked in at configure time)
    // Logs which source was used (never the token itself) and prints a hint
    // when no token is found anywhere.
    static std::string ResolveApiToken( int argc = 0, char** argv = nullptr );

    // When --help, -h or /? is present on the command line, print the common
    // example options and exit.
    static void HandleHelpOption( int argc, char** argv );

    template<typename F>
    bool WaitUntil( F TFunct, int nTimeOut, const char* szCallerFile = nullptr, int nCallerLine = 0 )
    {
        gem::Time initialTime = gem::Time::getUniversalTime();

        do
        {
            gem::Time time = gem::Time::getUniversalTime();
            m_timer->Tick();
            if( m_openGLContext )
            {
                m_openGLContext->renderFrame();
                m_openGLContext->pollEvents();
                if( m_openGLContext->shouldClose() )
                    return false;
            }
            unsigned ms = unsigned( gem::Time::getUniversalTime().asInt() - time.asInt() );
            if( m_timer->GetPeriod() > ms )
                std::this_thread::sleep_for( std::chrono::duration<int, std::milli>( m_timer->GetPeriod() - ms ) );

            if( ( gem::Time::getUniversalTime().asInt() - initialTime.asInt() ) > nTimeOut )
            {
                gem::Debug().log( gem::ELogLevel::LogWarn, LOG_TAG, "WAIT_UNTIL", szCallerFile ? szCallerFile : __FILE__, nCallerLine,
                                  gem::String::formatString( u"timed out after %d ms", nTimeOut ) );
                return false;
            }

        } while( !TFunct() );

        return true;
    }

    void WaitTimeout( int nTimeOut )
    {
        gem::Time initialTime = gem::Time::getUniversalTime();

        while( true )
        {
            gem::Time time = gem::Time::getUniversalTime();
            m_timer->Tick();
            if( m_openGLContext )
            {
                m_openGLContext->renderFrame();
                m_openGLContext->pollEvents();
                if( m_openGLContext->shouldClose() )
                    return;
            }
            unsigned ms = unsigned( ( gem::Time::getUniversalTime().asInt() - time.asInt() ) );
            if( m_timer->GetPeriod() > ms )
                std::this_thread::sleep_for( std::chrono::duration<int, std::milli>( m_timer->GetPeriod() - ms ) );

            // check timeout
            if( ( gem::Time::getUniversalTime().asInt() - initialTime.asInt() ) > nTimeOut )
                break;
        }
    }

    void WaitUntilWindowClose();

    void InitSDK( std::string token = "", std::string logFilePath = "", std::string sdkPluginPath = "", bool goOnline = true );

    void ReleaseSDK();

    gem::StrongPointer<OpenGLContext> ProduceOpenGLContext( WindowFrameworks framework, std::string windowName = "", ITouchEventsListener* pListener = nullptr,
                                                            UICallbacks uiRender = {}, gem::Size windowSize = gem::Size( 0, 0 ), int rotation = 0 );

    gem::StrongPointer<OffboardListenerImpl> GetOffboardListener()
    {
        return m_offboardListener;
    }

    // Activation-state notifications raised by auto-activation SDKs (gem::ISdkExceptions::onSdkNotActivated /
    // onSdkActivated). An example registers callbacks to react, e.g. to show or hide an "SDK not activated" watermark.
    // Register BEFORE creating the SdkSession: the SDK may already report "not activated" during initialization when a
    // token is provided but the device is offline. Callbacks may be invoked from SDK threads - keep them cheap and
    // thread-safe (e.g. set an atomic flag the UI thread consumes). Defaults to no-op, so other examples are unaffected.
    using NotActivatedCallback = std::function<void( gem::ESdkNotActivatedReason reason )>;
    using ActivatedCallback = std::function<void()>;

    void SetActivationStateCallbacks( NotActivatedCallback onNotActivated, ActivatedCallback onActivated )
    {
        m_onSdkNotActivated = std::move( onNotActivated );
        m_onSdkActivated = std::move( onActivated );
    }

    void NotifySdkNotActivated( gem::ESdkNotActivatedReason reason )
    {
        if( m_onSdkNotActivated )
            m_onSdkNotActivated( reason );
    }

    void NotifySdkActivated()
    {
        if( m_onSdkActivated )
            m_onSdkActivated();
    }

    std::string GetSDKExamplesPath();

    std::string GetDataPath();

    std::string GetPluginPath();

    std::string GetCachePath();

private:
    void Update();

    Environment();

    Timer* m_timer = nullptr;
    gem::StrongPointer<ApiLogger> m_apiLogger;
    gem::StrongPointer<OpenGLContext> m_openGLContext;

    std::shared_ptr<OffboardListenerImpl> m_offboardListener;

    NotActivatedCallback m_onSdkNotActivated;
    ActivatedCallback m_onSdkActivated;

    // Command line captured by HandleHelpOption / ResolveApiToken so shared
    // options (--size, --rotation, --ui_backend) reach ProduceOpenGLContext
    // without every example forwarding argc/argv again.
    int m_argc = 0;
    char** m_argv = nullptr;
};

#undef WAIT_UNTIL
#define WAIT_UNTIL( funct, timeout ) Environment::GetInstance().WaitUntil( funct, timeout, __FILE__, __LINE__ )
#undef WAIT_TIME_OUT
#define WAIT_TIME_OUT( timeout ) Environment::GetInstance().WaitTimeout( timeout )
#undef WAIT_UNTIL_WINDOW_CLOSE
#define WAIT_UNTIL_WINDOW_CLOSE Environment::GetInstance().WaitUntilWindowClose

#define FRM( a ) u##a
#define GEM_LOGV( frm, ... ) gem::Debug().log( gem::ELogLevel::LogVerbose, LOG_TAG, __FUNCTION__, __FILE__, __LINE__, gem::String::formatString( FRM( frm ), ##__VA_ARGS__ ) )
#define GEM_LOGD( frm, ... ) gem::Debug().log( gem::ELogLevel::LogDebug, LOG_TAG, __FUNCTION__, __FILE__, __LINE__, gem::String::formatString( FRM( frm ), ##__VA_ARGS__ ) )
#define GEM_LOGI( frm, ... ) gem::Debug().log( gem::ELogLevel::LogInfo, LOG_TAG, __FUNCTION__, __FILE__, __LINE__, gem::String::formatString( FRM( frm ), ##__VA_ARGS__ ) )
#define GEM_LOGW( frm, ... ) gem::Debug().log( gem::ELogLevel::LogWarn, LOG_TAG, __FUNCTION__, __FILE__, __LINE__, gem::String::formatString( FRM( frm ), ##__VA_ARGS__ ) )
#define GEM_LOGE( frm, ... ) gem::Debug().log( gem::ELogLevel::LogError, LOG_TAG, __FUNCTION__, __FILE__, __LINE__, gem::String::formatString( FRM( frm ), ##__VA_ARGS__ ) )
