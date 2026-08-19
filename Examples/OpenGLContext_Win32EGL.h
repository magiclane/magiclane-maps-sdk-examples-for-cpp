// SPDX-FileCopyrightText: 2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

// Minimal, dependency-free OpenGL context for embedding the Maps SDK into a
// native Win32 window via EGL (ANGLE). Intended as the reference for
// integrating the SDK into an existing application or UI framework (custom
// engines, HMI toolkits, etc.) without SDL/GLFW.
//
// The integration contract every custom IOpenGLContext must fulfil:
//   1. Create the window and the EGL context.
//   2. Call SetViewport( clientWidth, clientHeight ) once the context is
//      current, and again on every resize. viewport() is how the SDK learns
//      its render area - with an empty viewport the map renders NOTHING
//      (the screen stays at the engine clear color, typically black).
//   3. Store the touch listener and forward input events to it.
//   4. Call OpenGLContext::initialize() when setup succeeded.
//   5. makeCurrent()/doneCurrent() bracket SDK rendering; present the frame
//      (eglSwapBuffers) in doneCurrent().
//
// When embedded next to a host renderer (an application with its own WGL or
// EGL context on the same thread), makeCurrent() saves the host's current
// context and doneCurrent() restores it, so SDK rendering never disturbs the
// host's state. Standalone, this save/restore is a no-op.
//
// Display rotation (90/180/270) is NOT implemented here; see
// OpenGLContext_SDL for the off-screen FBO pattern it requires.

#if ( defined( _WIN32 ) || defined( _WIN64 ) ) && !defined( __MINGW32__ ) && !defined( __MINGW64__ )

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>

#include <EGL/egl.h>

#include <cstdio>
#include <string>

// OpenGL context for a plain Win32 window, EGL/ANGLE backend
class OpenGLContext_Win32EGL : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce( std::string windowName, ITouchEventsListener* pTouchEventListener, gem::Size windowSize, int rotation = 0 )
    {
        auto ptr = gem::StrongPointerFactory<OpenGLContext_Win32EGL>();

        if( !ptr->initialize( windowName, pTouchEventListener, windowSize, rotation ) )
            ptr.reset();

        return ptr;
    }

    ~OpenGLContext_Win32EGL() override
    {
        if( m_eglDisplay != EGL_NO_DISPLAY )
        {
            eglMakeCurrent( m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
            if( m_eglContext != EGL_NO_CONTEXT )
                eglDestroyContext( m_eglDisplay, m_eglContext );
            if( m_eglSurface != EGL_NO_SURFACE )
                eglDestroySurface( m_eglDisplay, m_eglSurface );
            eglTerminate( m_eglDisplay );
        }

        if( m_hdc && m_hwnd )
            ReleaseDC( m_hwnd, m_hdc );
        if( m_hwnd )
            DestroyWindow( m_hwnd );
    }

    bool makeCurrent() override
    {
        if( !m_initialized )
            return false;

        // Host-renderer coexistence: remember whatever the host application had
        // current (WGL and/or EGL) so doneCurrent() can hand the thread back
        // exactly as it found it.
        m_prevWglHDC = wglGetCurrentDC();
        m_prevWglHGLRC = wglGetCurrentContext();
        m_prevEglDisplay = eglGetCurrentDisplay();
        m_prevEglDraw = eglGetCurrentSurface( EGL_DRAW );
        m_prevEglRead = eglGetCurrentSurface( EGL_READ );
        m_prevEglContext = eglGetCurrentContext();

        return eglMakeCurrent( m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext ) == EGL_TRUE;
    }

    bool doneCurrent() override
    {
        if( !m_initialized )
            return false;

        // Present the frame the SDK just rendered
        eglSwapBuffers( m_eglDisplay, m_eglSurface );

        // Restore the host's EGL context if one (other than ours) was current.
        // Standalone (no host renderer): both branches are no-ops.
        if( m_prevEglContext != EGL_NO_CONTEXT && m_prevEglContext != m_eglContext )
            eglMakeCurrent( m_prevEglDisplay, m_prevEglDraw, m_prevEglRead, m_prevEglContext );

        // Restore the host's WGL context if making ours current displaced it.
        // ANGLE's default D3D backend leaves WGL untouched; its OpenGL backend does not.
        if( m_prevWglHGLRC && wglGetCurrentContext() != m_prevWglHGLRC )
            wglMakeCurrent( m_prevWglHDC, m_prevWglHGLRC );

        return true;
    }

    bool shouldClose() const override
    {
        return m_bShouldClose;
    }

    void pollEvents() override
    {
        MSG msg = {};
        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            if( msg.message == WM_QUIT )
            {
                m_bShouldClose = true;
                break;
            }
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

protected:
    virtual bool initialize( const std::string& windowName, ITouchEventsListener* pTouchEventListener, gem::Size windowSize, int rotation )
    {
        if( rotation != 0 )
        {
            std::printf( "OpenGLContext_Win32EGL: display rotation is not supported by this context\n" );
            return false;
        }
        m_rotation = rotation;

        // Logical size the SDK will see (client area, not outer window size)
        m_nWidth = windowSize.width > 0 ? windowSize.width : 800;
        m_nHeight = windowSize.height > 0 ? windowSize.height : 600;

        if( !createNativeWindow( "Maps SDK for C++ Samples - " + windowName ) )
            return false;

        if( !initializeEGL() )
            return false;

        // Contract step 2: report the actual client area to the SDK
        RECT clientRect;
        GetClientRect( m_hwnd, &clientRect );
        SetViewport( clientRect.right - clientRect.left, clientRect.bottom - clientRect.top );

        // Contract step 3: keep the listener; window proc forwards input to it
        m_pTouchEventListener = pTouchEventListener;

        ShowWindow( m_hwnd, SW_SHOW );
        UpdateWindow( m_hwnd );

        // Contract step 4
        return OpenGLContext::initialize();
    }

private:
    bool createNativeWindow( const std::string& windowName )
    {
        HINSTANCE hInstance = GetModuleHandle( nullptr );

        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof( WNDCLASSEX );
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
        wcex.lpszClassName = kWindowClassName;

        if( !RegisterClassExW( &wcex ) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS )
        {
            std::printf( "OpenGLContext_Win32EGL: RegisterClassExW failed (%lu)\n", GetLastError() );
            return false;
        }

        // Size the window so the CLIENT area matches the requested logical size
        RECT rect = { 0, 0, m_nWidth, m_nHeight };
        AdjustWindowRect( &rect, WS_OVERLAPPEDWINDOW, FALSE );

        std::wstring title( windowName.begin(), windowName.end() );
        m_hwnd = CreateWindowW( kWindowClassName, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr,
                                nullptr, hInstance, this );

        if( !m_hwnd )
        {
            std::printf( "OpenGLContext_Win32EGL: CreateWindowW failed (%lu)\n", GetLastError() );
            return false;
        }

        return true;
    }

    bool initializeEGL()
    {
        m_hdc = GetDC( m_hwnd );
        m_eglDisplay = eglGetDisplay( m_hdc );
        if( m_eglDisplay == EGL_NO_DISPLAY )
            return false;

        EGLint majorVersion = 0, minorVersion = 0;
        if( !eglInitialize( m_eglDisplay, &majorVersion, &minorVersion ) )
            return false;

        // Match the reference configuration used by OpenGLContext_SDL:
        // RGBA8888 + depth 24 + stencil 8
        const EGLint configAttribs[] = { EGL_RENDERABLE_TYPE,
                                         EGL_OPENGL_ES2_BIT,
                                         EGL_SURFACE_TYPE,
                                         EGL_WINDOW_BIT,
                                         EGL_RED_SIZE,
                                         8,
                                         EGL_GREEN_SIZE,
                                         8,
                                         EGL_BLUE_SIZE,
                                         8,
                                         EGL_ALPHA_SIZE,
                                         8,
                                         EGL_DEPTH_SIZE,
                                         24,
                                         EGL_STENCIL_SIZE,
                                         8,
                                         EGL_NONE };

        EGLConfig config = nullptr;
        EGLint numConfigs = 0;
        if( !eglChooseConfig( m_eglDisplay, configAttribs, &config, 1, &numConfigs ) || numConfigs < 1 )
            return false;

        m_eglSurface = eglCreateWindowSurface( m_eglDisplay, config, m_hwnd, nullptr );
        if( m_eglSurface == EGL_NO_SURFACE )
            return false;

        // Prefer an ES3 context, fall back to ES2
        for( EGLint version : { 3, 2 } )
        {
            const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, version, EGL_NONE };
            m_eglContext = eglCreateContext( m_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs );
            if( m_eglContext != EGL_NO_CONTEXT )
                break;
        }
        if( m_eglContext == EGL_NO_CONTEXT )
            return false;

        return eglMakeCurrent( m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext ) == EGL_TRUE;
    }

    static LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        if( uMsg == WM_NCCREATE )
        {
            auto pCreate = reinterpret_cast<CREATESTRUCTW*>( lParam );
            SetWindowLongPtrW( hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pCreate->lpCreateParams ) );
            return DefWindowProc( hwnd, uMsg, wParam, lParam );
        }

        auto pThis = reinterpret_cast<OpenGLContext_Win32EGL*>( GetWindowLongPtrW( hwnd, GWLP_USERDATA ) );
        if( !pThis )
            return DefWindowProc( hwnd, uMsg, wParam, lParam );

        switch( uMsg )
        {
            case WM_DESTROY:
            {
                PostQuitMessage( 0 );
                return 0;
            }
            case WM_KEYDOWN:
            {
                if( wParam == VK_ESCAPE )
                    pThis->m_bShouldClose = true;
                return 0;
            }
            case WM_SIZE:
            {
                // Contract step 2 (again): keep the SDK's render area in sync
                const int width = LOWORD( lParam );
                const int height = HIWORD( lParam );
                if( pThis->m_initialized && wParam != SIZE_MINIMIZED && width > 0 && height > 0 )
                    pThis->SetViewport( width, height );
                return 0;
            }
            case WM_LBUTTONDOWN:
            {
                SetCapture( hwnd );
                if( auto pListener = pThis->GetTouchEventHandler() )
                    pListener->handleTouchEvent( gem::ETouchEvent::TE_Down, 0, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
                return 0;
            }
            case WM_LBUTTONUP:
            {
                ReleaseCapture();
                if( auto pListener = pThis->GetTouchEventHandler() )
                    pListener->handleTouchEvent( gem::ETouchEvent::TE_Up, 0, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
                return 0;
            }
            case WM_MOUSEMOVE:
            {
                if( auto pListener = pThis->GetTouchEventHandler() )
                    pListener->handleTouchEvent( gem::ETouchEvent::TE_Move, 0, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
                return 0;
            }
            case WM_MOUSEWHEEL:
            {
                if( auto pListener = pThis->GetTouchEventHandler() )
                {
                    // WM_MOUSEWHEEL reports screen coordinates - convert to client
                    POINT pt = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };
                    ScreenToClient( hwnd, &pt );
                    const int nMouseScroll = GET_WHEEL_DELTA_WPARAM( wParam ) * 1000 / WHEEL_DELTA;
                    pListener->handleMouseScrollEvent( nMouseScroll, pt.x, pt.y );
                }
                return 0;
            }
            default:
            {
                return DefWindowProc( hwnd, uMsg, wParam, lParam );
            }
        }
    }

    static constexpr const wchar_t* kWindowClassName = L"MagicLaneWin32EGLWindowClass";

    HWND m_hwnd = nullptr;
    HDC m_hdc = nullptr;
    bool m_bShouldClose = false;

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    EGLSurface m_eglSurface = EGL_NO_SURFACE;
    EGLContext m_eglContext = EGL_NO_CONTEXT;

    // Host-renderer coexistence: what was current before makeCurrent()
    HDC m_prevWglHDC = nullptr;
    HGLRC m_prevWglHGLRC = nullptr;
    EGLDisplay m_prevEglDisplay = EGL_NO_DISPLAY;
    EGLSurface m_prevEglDraw = EGL_NO_SURFACE;
    EGLSurface m_prevEglRead = EGL_NO_SURFACE;
    EGLContext m_prevEglContext = EGL_NO_CONTEXT;
};

#else

// Win32/EGL context is Windows-only; produce nothing on other platforms
class OpenGLContext_Win32EGL : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce( std::string, ITouchEventsListener*, gem::Size, int = 0 )
    {
        return gem::StrongPointer<OpenGLContext>();
    }
};

#endif // Win32, not MinGW
