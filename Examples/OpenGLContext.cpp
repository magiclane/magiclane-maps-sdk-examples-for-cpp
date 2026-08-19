// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "OpenGLContext.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif

#if !defined( __MINGW32__ ) && !defined( __MINGW64__ )
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#include <API/GEM_Time.h>

#include <algorithm>
#include <API/GEM_Canvas.h>
#include <API/GEM_Debug.h>
#include <API/GEM_MapView.h>
#include <API/GEM_OperationScheduler.h>

#include "OpenGLContext_GLFW.h"
#include "OpenGLContext_SDL.h"
#include "OpenGLContext_ImGUI.h"
#include "OpenGLContext_LVGL.h"
#include "OpenGLContext_Win32EGL.h"

OpenGLContext::OpenGLContext()
    : m_initialized( false )
{
}

bool OpenGLContext::isInitialized() const
{
    return m_initialized;
}

gem::EImagePixelFormat OpenGLContext::encoding() const
{
    return gem::EImagePixelFormat::ABGR_8888;
}

gem::Rect const& OpenGLContext::viewport() const
{
    return m_viewport;
}

void OpenGLContext::SetViewport( int width, int height )
{
    m_viewport = gem::Rect( 0, 0, width, height );
    m_nWidth = width;
    m_nHeight = height;
    gem::Size newsize( m_nWidth, m_nHeight );
    auto ptr = m_screen.lock();
    if( ptr )
        ptr->resize( newsize );
}

gem::Rect OpenGLContext::GetViewport() const
{
    return m_viewport;
}

void OpenGLContext::attached( gem::Screen& screen )
{
    m_screen = screen.shared_from_this();
    gem::OperationScheduler().timeoutOperation(
        0,
        [&]()
        {
            screen.setRenderingRule( gem::RR_OnDemand );
        },
        gem::ProgressListener(), true );
}

void OpenGLContext::prepare()
{
    if( m_pTouchEventListener )
    {
        auto ptr = m_screen.lock();

        if( ptr )
            m_pTouchEventListener->setParent( ptr );
    }
}

bool OpenGLContext::renderFrame()
{
    if( m_needsRender )
    {
        auto ptr = m_screen.lock();
        if( ptr )
        {
            // Integration diagnostic: an empty viewport means SetViewport() was never
            // called - the engine will render nothing and the window stays at the
            // clear color. Warn once instead of failing silently.
            if( !m_warnedEmptyViewport && ( m_viewport.width <= 0 || m_viewport.height <= 0 ) )
            {
                m_warnedEmptyViewport = true;
                static const char* kEmptyViewportWarning = "Render requested with an empty viewport (%dx%d) - the map will not be visible. "
                                                           "Call SetViewport( clientWidth, clientHeight ) after creating the GL context and on every resize "
                                                           "(see the integration contract in OpenGLContext_Win32EGL.h).";
                // SDK log (reaches the ApiLogger / platform debug output even in windowed apps without a console)
                gem::Debug().log( gem::ELogLevel::LogWarn, "OpenGLContext", __FUNCTION__, __FILE__, __LINE__, kEmptyViewportWarning, m_viewport.width, m_viewport.height );
                // also stdout, for console launches
                std::printf( "OpenGLContext WARNING: " );
                std::printf( kEmptyViewportWarning, m_viewport.width, m_viewport.height );
                std::printf( "\n" );
            }

            m_needsRender = false; //ATTENTION !! always reset before Screen::render because it may trigger other render events
            ptr->render();

#ifdef LOG_RENDER_FPS
            // FPS counter
            ++m_renderFrameCount;
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( now - m_renderWindowStart ).count();
            if( elapsed >= 1000 )
            {
                std::printf( "renderFrame FPS: %d\n", m_renderFrameCount );
                m_renderFrameCount = 0;
                m_renderWindowStart = now;
            }
#endif

            return true;
        }
    }

    return false;
}

ITouchEventsListener* OpenGLContext::GetTouchEventHandler()
{
    return m_pTouchEventListener;
}

gem::StrongPointer<OpenGLContext> OpenGLContext::Produce_GLFW( std::string windowName, ITouchEventsListener* pTouchEventListener, gem::Size windowSize )
{
    return OpenGLContext_GLFW::Produce( windowName, pTouchEventListener, windowSize );
}

gem::StrongPointer<OpenGLContext> OpenGLContext::Produce_SDL( std::string windowName, ITouchEventsListener* pTouchEventListener, gem::Size windowSize, int rotation )
{
    return OpenGLContext_SDL::Produce( windowName, pTouchEventListener, windowSize, rotation );
}

gem::StrongPointer<OpenGLContext> OpenGLContext::Produce_ImGUI( std::string windowName, ITouchEventsListener* pTouchEventListener, UICallbacks uiCallbacks, gem::Size windowSize,
                                                                int rotation )
{
    return OpenGLContext_ImGUI::Produce( windowName, pTouchEventListener, uiCallbacks, windowSize, rotation );
}

gem::StrongPointer<OpenGLContext> OpenGLContext::Produce_LVGL( std::string windowName, ITouchEventsListener* pTouchEventListener, UICallbacks uiCallbacks, gem::Size windowSize,
                                                               int rotation )
{
#if defined( USE_LVGL ) && !defined( USE_GLFW )
    return OpenGLContext_LVGL::Produce( windowName, pTouchEventListener, uiCallbacks, windowSize, rotation );
#else
    ( void ) uiCallbacks;
    return gem::StrongPointer<OpenGLContext>();
#endif
}

gem::StrongPointer<OpenGLContext> OpenGLContext::Produce_Win32EGL( std::string windowName, ITouchEventsListener* pTouchEventListener, gem::Size windowSize, int rotation )
{
    return OpenGLContext_Win32EGL::Produce( windowName, pTouchEventListener, windowSize, rotation );
}
