// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#ifdef USE_IMGUI

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "go_regular_ttf.h"

// OpenGL context for SDL window system
class OpenGLContext_ImGUI : public OpenGLContext_SDL
{
public:
    static gem::StrongPointer<OpenGLContext> Produce( std::string windowName, ITouchEventsListener* pEventTouchListener, UICallbacks uiCallbacks, gem::Size windowSize, int rotation = 0 )
    {
        auto ptr = gem::StrongPointerFactory<OpenGLContext_ImGUI>();

        ptr->m_uiCallbacks = uiCallbacks;
        ptr->m_rotation = rotation;
        if( !ptr->initialize( windowName, pEventTouchListener, windowSize ) )
            ptr.reset();

        return ptr;
    }
private:
    enum TContextStatus
    {
        ELocked,
        EWaitMakeCurrent,
        EWaitDoneCurrent
    };
protected:
    bool initialize( std::string windowName, ITouchEventsListener* pEventTouchListener, gem::Size windowSize ) override
    {
        if( !OpenGLContext_SDL::initialize( windowName, pEventTouchListener, windowSize ) )
            return false;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.IniFilename = nullptr;

        if( m_uiCallbacks.first && !m_uiCallbacks.first() )
            return false;

        ImGui::StyleColorsDark();  // ImGui::StyleColorsLight();

        if ((io.Fonts->AddFontFromMemoryCompressedTTF((const void*)goRegular_ttf_compressed_data, goRegular_ttf_compressed_size_bytes, 18) == nullptr) ||
            (io.Fonts->AddFontFromMemoryCompressedTTF((const void*)goRegular_ttf_compressed_data, goRegular_ttf_compressed_size_bytes, 27) == nullptr) ||
            (io.Fonts->AddFontFromMemoryCompressedTTF((const void*)goRegular_ttf_compressed_data, goRegular_ttf_compressed_size_bytes, 36) == nullptr))
            return false;
        io.FontDefault = io.Fonts->Fonts[0];

        // Setup Platform/Renderer backends
        if ( !ImGui_ImplSDL2_InitForOpenGL(m_window, m_context) )
            return false;

        const char* glsl_version;
#if defined(IMGUI_IMPL_OPENGL_ES3)
        // OpenGL ES 3.0 + GLSL ES 3.00 (ANGLE on Windows)
        glsl_version = "#version 300 es";
#elif defined(IMGUI_IMPL_OPENGL_ES2)
        // OpenGL ES 2.0 + GLSL ES 1.00
        glsl_version = "#version 100";
#else
        // Desktop OpenGL 3.0+ + GLSL 1.30+
        glsl_version = "#version 130";
#endif

        if ( !ImGui_ImplOpenGL3_Init(glsl_version) ) {
            // Try auto-detect as fallback
            if ( !ImGui_ImplOpenGL3_Init(nullptr) ) {
                return false;
            }
        }

        return true;
    }

    bool renderFrame() override
    {
        if( !OpenGLContext::renderFrame() )
        {
            //just render UI
            makeCurrent();
            doneCurrent();

            return false;
        }

        return true;
    }

    bool makeCurrent()
    {
        if( m_status == EWaitMakeCurrent )
        {
            if( !OpenGLContext_SDL::makeCurrent() )
                return false;

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            // Override DisplaySize to logical dimensions when rotation is active.
            // ImGui SDL2 backend reads physical window size, but we want ImGui to
            // lay out at the logical (rotated) size.
            if (m_rotation != 0)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = ImVec2(static_cast<float>(m_nWidth), static_cast<float>(m_nHeight));
            }

            m_status = EWaitDoneCurrent;
        }

        return true;
    }

    bool doneCurrent()
    {
        if( m_status == EWaitDoneCurrent )
        {
            //do external render
            if( m_uiCallbacks.second )
            {
                auto ptr = m_screen.lock();
                if( ptr )
                {
                    ptr->iterateViews( [&]( gem::StrongPointer<gem::MapView> view )
                        {
                            //call external render of first view only ( by design examples have only 1 view )
                            m_status = ELocked; //avoid doing make / done operation
                            m_uiCallbacks.second( view );
                            return false;
                        }
                    );
                }
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            m_status = EWaitMakeCurrent;

            return OpenGLContext_SDL::doneCurrent();
        }

        return true;
    }

    bool shouldClose() const override
    {
        return m_bShouldClose;
    }

    bool processEvent( SDL_Event& event ) override
    {
        // Transform mouse coordinates in the event before passing to ImGui
        // so that ImGui sees logical (rotated) coordinates.
        if (m_rotation != 0)
        {
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
            {
                int lx, ly;
                transformTouchCoords(event.motion.x, event.motion.y, lx, ly);
                event.motion.x = lx;
                event.motion.y = ly;
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                int lx, ly;
                transformTouchCoords(event.button.x, event.button.y, lx, ly);
                event.button.x = lx;
                event.button.y = ly;
                break;
            }
            default:
                break;
            }
        }

        ImGui_ImplSDL2_ProcessEvent(&event);

        ImGuiIO& io = ImGui::GetIO();

        return io.WantCaptureMouse;
    }

private:
    TContextStatus m_status = EWaitMakeCurrent;
    UICallbacks m_uiCallbacks;
};

#else

class OpenGLContext_ImGUI : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce( std::string, ITouchEventsListener*, UICallbacks, gem::Size, int = 0)
    {
        return gem::StrongPointer<OpenGLContext>();
    }
};

#endif // USE_IMGUI
