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

#if !defined(__MINGW32__) && !defined(__MINGW64__)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#include <API/GEM_Time.h>

#include <algorithm>
#include <API/GEM_Canvas.h>
#include <API/GEM_MapView.h>
#include <API/GEM_OperationScheduler.h>

#include "OpenGLContext_GLFW.h"
#include "OpenGLContext_SDL.h"
#include "OpenGLContext_ImGUI.h"
#include "OpenGLContext_LVGL.h"

OpenGLContext::OpenGLContext() : m_initialized(false)
{
}

bool OpenGLContext::isInitialized() const
{
	return true;
}

gem::EImagePixelFormat OpenGLContext::encoding() const
{
	return gem::EImagePixelFormat::ABGR_8888;
}

gem::Rect const& OpenGLContext::viewport() const
{
	return m_viewport;
}

void OpenGLContext::SetViewport(int width, int height)
{
	m_viewport = gem::Rect(0, 0, width, height);
    m_nWidth = width;
    m_nHeight = height;
    gem::Size newsize(m_nWidth, m_nHeight);
    auto ptr = m_screen.lock();
    if ( ptr ) ptr->resize(newsize);
}

gem::Rect OpenGLContext::GetViewport() const
{
	return m_viewport;
}

void OpenGLContext::attached( gem::Screen &screen )
{
    m_screen = screen.shared_from_this();
    gem::OperationScheduler().timeoutOperation( 0, [&]() { screen.setRenderingRule( gem::RR_OnDemand ); }, gem::ProgressListener(), true );
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
            m_needsRender = false; //ATTENTION !! always reset before Screen::render because it may trigger other render events
            ptr->render();

#ifdef LOG_RENDER_FPS
            // FPS counter
            ++m_renderFrameCount;
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_renderWindowStart).count();
            if (elapsed >= 1000)
            {
                std::printf("renderFrame FPS: %d\n", m_renderFrameCount);
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

gem::StrongPointer<OpenGLContext> OpenGLContext::Produce_ImGUI( std::string windowName, ITouchEventsListener* pTouchEventListener, UICallbacks uiCallbacks, gem::Size windowSize, int rotation )
{
    return OpenGLContext_ImGUI::Produce( windowName, pTouchEventListener, uiCallbacks, windowSize, rotation );

}

gem::StrongPointer<OpenGLContext> OpenGLContext::Produce_LVGL(std::string windowName, ITouchEventsListener* pTouchEventListener, UICallbacks uiCallbacks, gem::Size windowSize, int rotation)
{
#if defined(USE_LVGL) && !defined(USE_GLFW)
    return OpenGLContext_LVGL::Produce(windowName, pTouchEventListener, uiCallbacks, windowSize, rotation );
#else
    (void)uiCallbacks;
    return gem::StrongPointer<OpenGLContext>();
#endif
}
