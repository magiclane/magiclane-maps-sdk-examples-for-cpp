// SPDX-FileCopyrightText: 2024-2026 Magic Lane International B.V. <info@magiclane.com>
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
    static gem::StrongPointer<OpenGLContext> Produce( std::string windowName, ITouchEventsListener* pEventTouchListener, UICallbacks uiCallbacks, gem::Size windowSize,
                                                      int rotation = 0 )
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
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.IniFilename = nullptr;

        if( m_uiCallbacks.first && !m_uiCallbacks.first() )
            return false;

        ImGui::StyleColorsDark(); // ImGui::StyleColorsLight();

        static const ImWchar kGlyphRanges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin-1 Supplement
            0x0100, 0x024F, // Latin Extended-A + Latin Extended-B
            0x0370, 0x03FF, // Greek (modern monotonic)
            0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
            0x1E00, 0x1EFF, // Latin Extended Additional (only partially in Go Regular)
            0x2000, 0x206F, // General punctuation: typographic quotes, dashes, ellipsis
            0,
        };

        if( ( io.Fonts->AddFontFromMemoryCompressedTTF( ( const void* ) goRegular_ttf_compressed_data, goRegular_ttf_compressed_size_bytes, 18, nullptr, kGlyphRanges ) ==
              nullptr ) ||
            ( io.Fonts->AddFontFromMemoryCompressedTTF( ( const void* ) goRegular_ttf_compressed_data, goRegular_ttf_compressed_size_bytes, 27, nullptr, kGlyphRanges ) ==
              nullptr ) ||
            ( io.Fonts->AddFontFromMemoryCompressedTTF( ( const void* ) goRegular_ttf_compressed_data, goRegular_ttf_compressed_size_bytes, 36, nullptr, kGlyphRanges ) ==
              nullptr ) )
            return false;
        io.FontDefault = io.Fonts->Fonts[0];

        // Setup Platform/Renderer backends
        if( !ImGui_ImplSDL2_InitForOpenGL( m_window, m_context ) )
            return false;

        const char* glsl_version;
#if defined( IMGUI_IMPL_OPENGL_ES3 )
        // OpenGL ES 3.0 + GLSL ES 3.00 (ANGLE on Windows)
        glsl_version = "#version 300 es";
#elif defined( IMGUI_IMPL_OPENGL_ES2 )
        // OpenGL ES 2.0 + GLSL ES 1.00
        glsl_version = "#version 100";
#else
        // Desktop OpenGL 3.0+ + GLSL 1.30+
        glsl_version = "#version 130";
#endif

        if( !ImGui_ImplOpenGL3_Init( glsl_version ) )
        {
            // Try auto-detect as fallback
            if( !ImGui_ImplOpenGL3_Init( nullptr ) )
            {
                return false;
            }
        }

        return true;
    }

    bool renderFrame() override
    {
        // A pending UI change must be repainted into EVERY buffer of the swapchain.
        // With flip-style presentation (e.g. ANGLE's GL backend, real WGL double
        // buffering) the buffers alternate on swap, so repainting a single frame
        // leaves the previous UI in the other buffer(s) - the two then alternate on
        // screen as continuous flicker. ANGLE's D3D11 backend masks this by copying
        // an offscreen buffer on present (preserved swap semantics).
        if( m_forceRepaintFrames > 0 )
        {
            --m_forceRepaintFrames;
            needsRender();
        }

        if( !OpenGLContext::renderFrame() )
        {
            // Map not dirty: still run the ImGui frame (input state, change detection),
            // but doneCurrent() will NOT present it - drawing semi-transparent widgets
            // over a stale buffer that already contains the UI accumulates their alpha
            // (visible pulsing on flip-model presentation). If the UI did change, the
            // scheduled repaints present a full map + UI frame on the next iteration.
            // Save / restore instead of set / clear: UI callbacks may pump nested
            // renderFrame() calls (e.g. WAIT_UNTIL in a button handler).
            const bool wasUiOnly = m_uiOnlyFrame;
            m_uiOnlyFrame = true;
            makeCurrent();
            doneCurrent();
            m_uiOnlyFrame = wasUiOnly;

            return false;
        }

        return true;
    }

    bool makeCurrent() override
    {
        if( m_status == EWaitMakeCurrent )
        {
            if( !OpenGLContext_SDL::makeCurrent() )
                return false;

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();

            // Override DisplaySize to logical dimensions when rotation is active.
            if( m_rotation != 0 )
            {
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = ImVec2( static_cast<float>( m_nWidth ), static_cast<float>( m_nHeight ) );

                if( SDL_GetMouseFocus() == m_window )
                {
                    int physX = 0, physY = 0;
                    SDL_GetMouseState( &physX, &physY );
                    int lx = 0, ly = 0;
                    transformTouchCoords( physX, physY, lx, ly );
                    io.AddMousePosEvent( static_cast<float>( lx ), static_cast<float>( ly ) );
                }
            }

            ImGui::NewFrame();

            m_status = EWaitDoneCurrent;
        }

        return true;
    }

    bool doneCurrent() override
    {
        if( m_status == EWaitDoneCurrent )
        {
            //do external render
            if( m_uiCallbacks.second )
            {
                auto ptr = m_screen.lock();
                if( ptr )
                {
                    ptr->iterateViews(
                        [&]( gem::StrongPointer<gem::MapView> view )
                        {
                            //call external render of first view only ( by design examples have only 1 view )
                            m_status = ELocked; //avoid doing make / done operation
                            m_uiCallbacks.second( view );
                            return false;
                        } );
                }
            }

            ImGui::Render();
            ImDrawData* drawData = ImGui::GetDrawData();

            // Detect UI content changes that do not come from input events (async
            // results arriving, panels appearing / hiding, text updating). When the UI
            // changes while the map is idle, schedule full repaints so every swapchain
            // buffer gets the new frame - see the comment in renderFrame().
            // The detection hashes the actual draw data contents: mere vertex / index
            // counts miss same-length text changes (e.g. new coordinates replacing old
            // ones), which would then never be presented.
            const unsigned int uiHash = HashDrawData( drawData );
            if( uiHash != m_lastUiHash )
            {
                m_lastUiHash = uiHash;
                m_forceRepaintFrames = KForceRepaintFrames;
            }

            // On a UI-only frame nothing valid can be presented: the backbuffer holds a
            // stale frame with the UI already baked in, so blending the UI again would
            // accumulate the alpha of semi-transparent widgets. Discard the frame; the
            // repaints scheduled above (if the UI changed) present it properly next
            // iteration, composited over a freshly rendered map.
            if( m_uiOnlyFrame )
            {
                m_status = EWaitMakeCurrent;
                return true;
            }

            ImGui_ImplOpenGL3_RenderDrawData( drawData );
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
        if( m_rotation != 0 )
        {
            switch( event.type )
            {
                case SDL_MOUSEMOTION:
                {
                    int lx, ly;
                    transformTouchCoords( event.motion.x, event.motion.y, lx, ly );
                    event.motion.x = lx;
                    event.motion.y = ly;
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                {
                    int lx, ly;
                    transformTouchCoords( event.button.x, event.button.y, lx, ly );
                    event.button.x = lx;
                    event.button.y = ly;
                    break;
                }
                default:
                    break;
            }
        }

        ImGui_ImplSDL2_ProcessEvent( &event );

        ImGuiIO& io = ImGui::GetIO();

        // ImGui is consuming this mouse event (e.g. dragging its window, clicking a
        // button), so the map's touch handler never sees it and the on-demand renderer
        // stays idle. Schedule full repaints so the map beneath the UI is redrawn into
        // every swapchain buffer - see the comment in renderFrame().
        if( io.WantCaptureMouse )
            m_forceRepaintFrames = KForceRepaintFrames;

        return io.WantCaptureMouse;
    }

private:
    TContextStatus m_status = EWaitMakeCurrent;
    UICallbacks m_uiCallbacks;

    // Full-repaint scheduling for UI changes: enough consecutive frames to refresh
    // every buffer of a double- or triple-buffered swapchain.
    static constexpr int KForceRepaintFrames = 3;
    int m_forceRepaintFrames = 0;

    // True while renderFrame() runs the ImGui frame without a map render beneath it;
    // such frames are never presented (see doneCurrent()).
    bool m_uiOnlyFrame = false;

    // Hash of the last submitted ImGui draw data, used to detect UI content changes
    // that do not come from input events.
    unsigned int m_lastUiHash = 0;

    // FNV-1a over the draw data's vertex & index buffers. Vertex UVs / colors change
    // whenever glyphs or widget states change, so this catches same-size content edits
    // that vertex counts alone would miss.
    static unsigned int HashDrawData( const ImDrawData* drawData )
    {
        unsigned int h = 2166136261u;
        auto mix = [&h]( const void* data, size_t size )
        {
            const unsigned char* p = static_cast<const unsigned char*>( data );
            for( size_t i = 0; i < size; ++i )
            {
                h ^= p[i];
                h *= 16777619u;
            }
        };

        for( int i = 0; i < drawData->CmdListsCount; ++i )
        {
            const ImDrawList* cmdList = drawData->CmdLists[i];
            mix( cmdList->VtxBuffer.Data, ( size_t ) cmdList->VtxBuffer.Size * sizeof( ImDrawVert ) );
            mix( cmdList->IdxBuffer.Data, ( size_t ) cmdList->IdxBuffer.Size * sizeof( ImDrawIdx ) );
        }

        return h;
    }
};

#else

class OpenGLContext_ImGUI : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce( std::string, ITouchEventsListener*, UICallbacks, gem::Size, int = 0 )
    {
        return gem::StrongPointer<OpenGLContext>();
    }
};

#endif // USE_IMGUI
