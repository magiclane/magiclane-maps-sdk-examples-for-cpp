// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "Environment.h"

#include <API/GEM_StdString.h>
#include <API/GEM_CallLogger.h>
#include <API/GEM_SdkSettings.h>
#include <API/GEM_ContentTypes.h>
#include <API/GEM_ContentStore.h>
#include <API/GEM_Debug.h>

#ifdef GEM_PLUGIN
#include <API/GEM_PluginSdk.h>
#endif

#if defined(__unix__)
	#include <unistd.h>
#endif

#include <filesystem>

class SdkExceptionsImpl : public gem::ISdkExceptions
{
public:
	void onSdkActivationAboutToExpire( gem::ESdkActivationAboutToExpireReason /*reason*/, gem::LargeInteger /*remainingTimeInSeconds*/ ) override {}
};

Environment::Environment()
{ }

void Environment::InitSDK(std::string token, std::string logFilePath, std::string sdkPluginPath, bool goOnline)
{
	int nErr = 0;

	// Initialize SDK
	std::string resPath = GetDataPath();
	if (resPath.empty())
	{
		GEM_ERROR_NOEXCEPT(gem::error::KInvalidInput);
		return;
	}

	std::string cachePath = GetCachePath();
	if (cachePath.empty())
	{
		GEM_ERROR_NOEXCEPT(gem::error::KInvalidInput);
		return;
	}

	if (!m_timer && !(m_timer = new(Timer)))
	{
		GEM_ERROR_NOEXCEPT(gem::error::KNoMemory);
		return;
	}

#ifdef GEM_PLUGIN
	if (sdkPluginPath.empty())
	{
		sdkPluginPath = GetPluginPath();
		if (sdkPluginPath.empty())
		{
			GEM_ERROR_NOEXCEPT(gem::error::KInvalidInput);
			return;
		}
	}
#endif

	m_apiLogger = gem::StrongPointerFactory<ApiLogger>( logFilePath.c_str() );

	// GEM SDK must have variant = 0 OR favor SDK ( 0xFF ) !!!
	auto sdkExceptions = gem::StrongPointerFactory<SdkExceptionsImpl>();
	gem::SdkInitializationParameters initParams( 0, resPath, cachePath, m_timer, sdkExceptions );
	initParams.savePolicy = gem::EDataSavePolicy::UseExternalOnly;
	initParams.logger = m_apiLogger;
	initParams.activationToken = token;

#ifdef GEM_PLUGIN
	nErr = gem::Sdk::load( sdkPluginPath, initParams );
#else
	nErr = gem::Sdk::initialize( initParams );
#endif

	if (nErr != gem::KNoError )
	{
		if( nErr != gem::error::KExist )
		{
			delete m_timer;
			m_timer = nullptr;
            GEM_ERROR_NOEXCEPT(nErr);
            return;
		}
	}

    // app token can also be set later via gem::SdkSettings, except if the SDK is an evaluation one, then it is required on gem::Sdk::load/initialize
	if (!token.empty())
		gem::SdkSettings().setAppAuthorization(token);

	// GO ONLINE & PERFORM APP UPDATE (only if SDK has content/online support)

	if (goOnline && (gem::Sdk::getCapabilities() & gem::SC_Content))
	{
		if (!m_offboardListener)
			m_offboardListener = std::make_shared<OffboardListenerImpl>();

		gem::SdkSettings().setAllowConnection(true, m_offboardListener);

		auto ret = WAIT_UNTIL(std::bind(&OffboardListenerImpl::IsOnline , m_offboardListener.get()), 5000);
		if (!ret)
		{
			GEM_ERROR_NOEXCEPT(gem::error::KNetworkFailed);
			gem::Debug().log(gem::LogFatal, "Env", __FUNCTION__, __FILE__, __LINE__, "Could not go ONLINE!");
		}

		Update();
	}
	else
	{
		if (!(gem::Sdk::getCapabilities() & gem::SC_Content))
			gem::Debug().log(gem::LogWarn, "Env", __FUNCTION__, __FILE__, __LINE__, "Online content support not available in this SDK build");
		else
			gem::SdkSettings().setAllowConnection( false, gem::OffboardListener() );
	}
}

void Environment::ReleaseSDK()
{
    m_openGLContext.reset();

#ifdef GEM_PLUGIN
    gem::Sdk::unload();
#else
    gem::Sdk::release();
#endif

	if (m_timer)
		delete m_timer;
	m_timer = nullptr;

	m_offboardListener.reset();
}

gem::StrongPointer<OpenGLContext> Environment::ProduceOpenGLContext(WindowFrameworks framework, std::string windowName, ITouchEventsListener* pTouchEventListener, UICallbacks uiCallbacks, gem::Size windowSize, int rotation )
{
	if (!m_openGLContext.get())
    {
        switch( framework )
        {
            case WindowFrameworks::GLFW:
                m_openGLContext = OpenGLContext::Produce_GLFW( windowName, pTouchEventListener, windowSize );
                break;
            case WindowFrameworks::SDL:
                m_openGLContext = OpenGLContext::Produce_SDL( windowName, pTouchEventListener, windowSize, rotation );
                break;
            case WindowFrameworks::ImGUI:
                m_openGLContext = OpenGLContext::Produce_ImGUI( windowName, pTouchEventListener, uiCallbacks, windowSize, rotation );
                break;
            case WindowFrameworks::LVGL:
                m_openGLContext = OpenGLContext::Produce_LVGL( windowName, pTouchEventListener, uiCallbacks, windowSize, rotation );
                break;
            case WindowFrameworks::Available:
				if( !(m_openGLContext = OpenGLContext::Produce_GLFW(windowName, pTouchEventListener, windowSize)) )
					m_openGLContext = OpenGLContext::Produce_SDL( windowName, pTouchEventListener, windowSize, rotation );
                break;
            default: {}
        }
    }

	return m_openGLContext;
}

std::string Environment::GetSDKExamplesPath()
{
#if defined(__EMSCRIPTEN__)
    return std::string("/");

    // Points to <Maps-SDK-Examples-for-Cpp> downloaded dir
#elif defined(MAPS_SDK_EXAMPLES_PATH)
    auto value = std::string(MAPS_SDK_EXAMPLES_PATH);
    auto path = std::filesystem::path(value).make_preferred();
    return path.u8string();

#elif defined(WIN32) || defined(WIN64)
    auto value = std::getenv("MAPS_SDK_EXAMPLES_PATH");
    if (value != nullptr)
        return std::string(value);

    char appPath[260], appPathShort[260];
    GetModuleFileNameA(0, appPath, 260);

    std::string possiblePaths[2] =
    {
        appPath + std::string("\\..\\..\\..\\..\\..\\..\\Examples\\Maps-SDK-Examples-for-Cpp"),
		appPath + std::string("\\..\\..\\..\\..\\..\\")
    };

    for (auto& path : possiblePaths)
    {
        PathCanonicalizeA(appPathShort, path.c_str());
        std::string dataPath = appPathShort + std::string("\\Examples");
        if (PathFileExistsA(dataPath.c_str()))
            return appPathShort;
    }

    return std::string();

#elif defined(__unix__)
    const auto value = std::getenv("MAPS_SDK_EXAMPLES_PATH");
    if (value != nullptr)
        return std::string(value);

    char execPath[FILENAME_MAX];
    int lenPath = readlink("/proc/self/exe", execPath, sizeof(execPath));
    execPath[lenPath] = 0;

    if ((lenPath > 0) && (lenPath < (int)sizeof(execPath)))
    {
        std::string execPathStd(execPath);
        std::size_t nSlash = execPathStd.find_last_of("/");
        std::string execName = execPathStd.substr(nSlash + 1, execPathStd.length());

        std::string possiblePaths[1] =
        {
                execPathStd.substr(0, nSlash) + "/../../Examples/Maps-SDK-Examples-for-Cpp"
        };

        char* pData;
        char dataPathC[FILENAME_MAX];

        for (auto& path : possiblePaths)
            if (pData = realpath(path.c_str(), dataPathC))
                return dataPathC;
    }

    return std::string();
#else
    return std::string();
#endif

}

std::string Environment::GetDataPath()
{
#if defined(__EMSCRIPTEN__)
	return std::string( "/" );

// Points to <SDKpath/share> dir
#elif defined(MAPS_SDK_DATA_PATH)
	auto value = std::string( MAPS_SDK_DATA_PATH );
	auto path = std::filesystem::path( value ).make_preferred();
	return path.u8string();

#elif defined(WIN32) || defined(WIN64)
	auto value = std::getenv( "MAPS_SDK_DATA_PATH" );
	if( value != nullptr )
		return std::string( value );

	char appPath[260], appPathShort[260];
	GetModuleFileNameA( 0, appPath, 260 );

	std::string possiblePaths[3] =
	{
		appPath + std::string("\\..\\..\\..\\..\\..\\..\\"),
		appPath + std::string("\\..\\..\\..\\..\\..\\..\\SDK\\share"),
		appPath + std::string("\\..\\..\\..\\..\\..\\SDK\\share")
	};

	for (auto& path : possiblePaths)
	{
		PathCanonicalizeA(appPathShort, path.c_str());
		std::string dataPath = appPathShort + std::string("\\Data");
		if (PathFileExistsA(dataPath.c_str()))
			return appPathShort;
	}

	return std::string();

#elif defined(__unix__)
	const auto value = std::getenv( "MAPS_SDK_DATA_PATH" );
	if( value != nullptr )
		return std::string( value );

	char execPath[FILENAME_MAX];
	int lenPath = readlink( "/proc/self/exe", execPath, sizeof( execPath ) );
	execPath[lenPath] = 0;

	if( (lenPath > 0) && (lenPath < (int)sizeof( execPath )) )
	{
		std::string execPathStd( execPath );
		std::size_t nSlash = execPathStd.find_last_of( "/" );
		std::string execName = execPathStd.substr( nSlash + 1, execPathStd.length() );

		std::string possiblePaths[5] =
		{
			std::string( "/usr/share/" ) + execName + "/share",
				std::string( "/usr/share/CppExamples/share" ),
				execPathStd.substr( 0, nSlash ) + "/../share",
				execPathStd.substr( 0, nSlash ) + "/../../SDK/share",
				execPathStd.substr( 0, nSlash ) + "/../../Maps-SDK-Examples-for-Cpp/SDK/share"
		};

		char* pData;
		char dataPathC[FILENAME_MAX];

		for(auto& path : possiblePaths)
			if( pData = realpath( path.c_str(), dataPathC ) )
				return dataPathC;
	}

	return std::string();
#else
	return std::string();
#endif
}

std::string Environment::GetPluginPath()
{
#if defined(MAPS_SDK_PLUGIN_PATH)
    return std::filesystem::path(MAPS_SDK_PLUGIN_PATH).make_preferred().u8string();
#else
    if (auto env = std::getenv("MAPS_SDK_PLUGIN_PATH"))
        return std::string(env);

    std::string execDir;

#if defined(WIN32) || defined(WIN64)
    char appPath[260];
    GetModuleFileNameA(0, appPath, 260);
    PathRemoveFileSpecA(appPath);
    execDir = appPath;
    const std::string release = execDir + "\\GEM.dll";
    const std::string debug = execDir + "\\GEM_d.dll";
#elif defined(__unix__)
    char execPath[FILENAME_MAX];
    int len = readlink("/proc/self/exe", execPath, sizeof(execPath));
    if (len <= 0 || len >= sizeof(execPath)) return "";
    execPath[len] = 0;
    execDir = std::string(execPath).substr(0, std::string(execPath).find_last_of("/"));
    const std::string release = execDir + "/libGEM.so";
    const std::string debug = execDir + "/libGEM_d.so";
#else
    return "";
#endif

    if (std::filesystem::exists(release)) return release;
    if (std::filesystem::exists(debug)) return debug;

    return release;
#endif
}

std::string Environment::GetCachePath()
{
#if defined(__EMSCRIPTEN__)
	return std::string( "/" );
#else
	#if defined(MAPS_SDK_CACHE_PATH)
		auto value = std::string( MAPS_SDK_CACHE_PATH );
		auto path = std::filesystem::path( value ).make_preferred();
		return path.u8string();
	#else
		auto value = std::getenv( "MAPS_SDK_CACHE_PATH" );
		if( value != nullptr )
			return std::string( value );
	#endif
#endif

#if defined(__unix__)
	auto homeDir = std::getenv( "HOME" );
	if( homeDir != nullptr )
	{
		char execPath[FILENAME_MAX];
		int lenPath = readlink( "/proc/self/exe", execPath, sizeof( execPath ) );
		if( (lenPath > 0) && (lenPath < (int)sizeof( execPath )) )
		{
			execPath[lenPath] = 0;
			std::string execPathStd( execPath );
			std::size_t nSlash = execPathStd.find_last_of( "/" );
			std::string execName = execPathStd.substr( nSlash + 1, execPathStd.length() );
			
			return std::string( homeDir ) + "/TmpData_" + execName;
		}
	}
#endif

	return GetDataPath();
}

void Environment::WaitUntilWindowClose()
{
    if (!m_openGLContext)
        return;

    m_openGLContext->prepare();

    while( !m_openGLContext->shouldClose() )
    {
        //give SDK execution time
        m_timer->Tick();

        //handle window events
        m_openGLContext->pollEvents();

        if( !m_openGLContext->renderFrame() )
            std::this_thread::sleep_for( std::chrono::milliseconds( m_timer->GetPeriod() ) );
    }
}

void Environment::Update()
{
	static constexpr const int kTypes[] = { gem::CT_RoadMap };
	static constexpr const int kNTypes = sizeof(kTypes) / sizeof(int);

	for (auto i = 0; i < kNTypes; ++i)
	{
		auto listenerPtr = std::make_shared<ProgressListener>();

		auto contentUpdate = gem::ContentStore().createContentUpdater(kTypes[i]);

		if(contentUpdate.second != gem::KNoError)
			continue;

		auto err = contentUpdate.first->update(true, listenerPtr);

		if(err != gem::KNoError)
			continue;

		//!!!!! WAIT UNTIL gem::TContentUpdaterStatus::EDownload state OR finished !!!!

		auto ret = WAIT_UNTIL([=]() { return listenerPtr->GetStatus() == int(gem::EContentUpdaterStatus::Download) || listenerPtr->IsFinished(); }, 90000);

		if (listenerPtr->GetStatus() == int(gem::EContentUpdaterStatus::Download))
		{
			//!!!! WAIT UNTIL gem::TContentUpdaterStatus::EFullyReady state !!!!
			ret = WAIT_UNTIL([listenerPtr]() { return listenerPtr->GetStatus() == (int)gem::EContentUpdaterStatus::FullyReady; }, 60000);

			// apply update if possible
			if (contentUpdate.first->canApply())
			{
				auto err = contentUpdate.first->apply();
				if(err != gem::KNoError)
					continue;
			}
		}
	}
}
