// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#include <chrono>
#include <thread>

#if defined(USE_LVGL) && !defined(USE_GLFW)

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include <SDL_opengles2.h>

/*
 * OpenGLContext_LVGL
 * ------------------
 *
 * Purpose
 * Provide an SDL + OpenGL ES rendering context that can host an LVGL UI overlay on top of the SDK map.
 *
 * How it works (high-level pipeline)
 * 1) SDL creates the window + OpenGL context (handled by `OpenGLContext_SDL`).
 * 2) LVGL renders into an in-memory draw buffer and calls our `flush_cb` (`s_disp_flush`) with updated tiles.
 * 3) `s_disp_flush` copies those tiles into a full-screen RGBA framebuffer (`m_lvglRgbaFrameBuffer`).
 * 4) On the next frame, the framebuffer is uploaded to an OpenGL texture (`m_lvglTextureGL`).
 * 5) The base map renders first (`OpenGLContext::renderFrame()`), then we alpha-blend the LVGL texture on top.
 * 6) SDL swaps buffers.
 *
 * Input routing (mouse/touch)
 * - We keep pointer state via SDL events and expose it to LVGL through `s_pointer_read`.
 * - Events are conditionally forwarded either to LVGL (UI) or to the map (SDK) in `processEvent()`.
 *
 * Notes / limitations (example-focused)
 * - Uses a single static instance (`s_instance`) because LVGL callbacks are plain C function pointers.
 * (This example assumes one LVGL window/context at a time.)
 * - The LVGL output is treated as an RGBA texture. If LVGL color settings change, the copy/upload format may need updates.
*/

class OpenGLContext_LVGL : public OpenGLContext_SDL
{
private:
    // LVGL draws in "tiles" into a driver-provided draw buffer. Using a fraction of the full screen
    // keeps memory usage low while still allowing incremental flushes.
    static constexpr int kLvglDrawBufferDivisor = 4; // 1 / kLvglDrawBufferDivisor of full screen

    // LVGL tick granularity. The loop sleeps and increments by the same amount.
    static constexpr uint32_t kLvglTickMs = 5;

    // Optional periodic callback interval (kept because it can be used to integrate SDL pumping, etc.).
    static constexpr uint32_t kLvglTimerPeriodMs = 10;

    // Map wheel scaling (matches existing behavior: LVGL not involved)
    static constexpr int kScrollDeltaScale = 1000;

public:
    static gem::StrongPointer<OpenGLContext> Produce(std::string windowName, ITouchEventsListener* pEventTouchListener, UICallbacks uiCallbacks = {}, gem::Size windowSize = gem::Size(0,0), int rotation = 0)
    {
        auto ptr = gem::StrongPointerFactory<OpenGLContext_LVGL>();
        ptr->m_uiCallbacks = std::move(uiCallbacks);
        ptr->m_rotation = rotation;

        if (!ptr->initialize(std::move(windowName), pEventTouchListener, windowSize))
            ptr.reset();
        return ptr;
    }

    ~OpenGLContext_LVGL() override
    {
        if (m_lvglRgbaFrameBuffer) { free(m_lvglRgbaFrameBuffer); m_lvglRgbaFrameBuffer = nullptr; }
        if (m_lvglTextureGL) { glDeleteTextures(1, &m_lvglTextureGL); m_lvglTextureGL = 0; }
        if (m_blitProgramGL) { glDeleteProgram(m_blitProgramGL); m_blitProgramGL = 0; }
    }

protected:

    bool initialize(std::string windowName, ITouchEventsListener* pEventTouchListener, gem::Size windowSize) override
    {
        if (!OpenGLContext_SDL::initialize(windowName, pEventTouchListener, windowSize))
            return false;

        // LVGL uses C callbacks, so we keep a single active instance for callback dispatch.
        s_instance = this;

        if (!InitLvglCore())
            return false;

        if (!InitLvglCompositionResources())
            return false;

        if (!InitLvglDisplay())
            return false;

        if (!InitLvglInput())
            return false;

        InitLvglTiming();
        return InitExampleUi();
    }

    virtual bool makeCurrent() override
    {
        return true;
    }

    virtual bool doneCurrent() override
    {
        return true;
    }

    // Render path aligned with BaseLvglWindow: map render + LVGL composite + swap
    bool renderFrame() override
    {
        // Process pending work (screen transitions, model updates) BEFORE
        // lv_timer_handler() so LVGL renders the NEW screen into the
        // framebuffer — not the old one that was just replaced.
        if (m_uiCallbacks.second)
        {
            auto screen = m_screen.lock();
            if (screen)
            {
                screen->iterateViews([&](gem::StrongPointer<gem::MapView> view)
                    {
                        m_uiCallbacks.second(view);
                        return false; // first view only
                    });
            }
        }

        // Now let LVGL render the current (possibly just-changed) screen
        lv_timer_handler();

        // Always make GL current
        SDL_GL_MakeCurrent(m_window, m_context);

        if (m_lvglTextureDirty)
        {
            glBindTexture(GL_TEXTURE_2D, m_lvglTextureGL);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_nWidth, m_nHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_lvglRgbaFrameBuffer);
            OpenGLContext::needsRender();
            m_lvglTextureDirty = false;
        }

        if (m_rotFBO != 0)
        {
            // Rotation active: bind FBO so map renders into off-screen texture
            glBindFramebuffer(GL_FRAMEBUFFER, m_rotFBO);
            glViewport(0, 0, m_nWidth, m_nHeight);
        }

        // Map render (on-demand): let base drive it
        bool mapRendered = OpenGLContext::renderFrame();

        if (mapRendered)
        {
            // Draw LVGL overlay (un-rotated, at logical size, into FBO if rotation active)
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            DrawTexture(m_blitProgramGL, m_lvglTextureGL);
        }

        if (m_rotFBO != 0)
        {
            // Unbind FBO and blit rotated to physical window
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, m_physWidth, m_physHeight);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_SCISSOR_TEST);
            DrawRotatedTexture(m_rotBlitProgram, m_rotColorTex, m_rotation);
        }

        // Swap
        SDL_GL_SwapWindow(m_window);

        // On DPI displays (e.g. HyperPixel 4) vsync may not work,
        // causing visible tearing/shaking.  Cap at ~60 FPS manually.
        if (SDL_GL_GetSwapInterval() == 0)
        {
            static auto lastFrame = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrame);
            constexpr auto kFrameTime = std::chrono::milliseconds(16); // ~60 FPS
            if (elapsed < kFrameTime)
                std::this_thread::sleep_for(kFrameTime - elapsed);
            lastFrame = std::chrono::steady_clock::now();
        }

        return mapRendered;
    }

    bool processEvent(SDL_Event& event) override
    {
        static bool mapIsDragging = false;

        // Keep pointer state updated for LVGL's `read_cb` (`s_pointer_read`).
        // LVGL will poll these values when it processes input inside `lv_timer_handler()`.
        // Transform physical coords to logical for LVGL.
        switch (event.type)
        {
        case SDL_MOUSEMOTION:
        {
            int px, py;
            SDL_GetMouseState(&px, &py);
            transformTouchCoords(px, py, m_mouseX, m_mouseY);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                m_mousePressed = true;
                int px, py;
                SDL_GetMouseState(&px, &py);
                transformTouchCoords(px, py, m_mouseX, m_mouseY);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                m_mousePressed = false;
                int px, py;
                SDL_GetMouseState(&px, &py);
                transformTouchCoords(px, py, m_mouseX, m_mouseY);
            }
            break;
        case SDL_FINGERDOWN:
        {
            m_mousePressed = true;
            int px = static_cast<int>(event.tfinger.x * m_physWidth);
            int py = static_cast<int>(event.tfinger.y * m_physHeight);
            transformTouchCoords(px, py, m_mouseX, m_mouseY);
            break;
        }
        case SDL_FINGERUP:
        {
            m_mousePressed = false;
            int px = static_cast<int>(event.tfinger.x * m_physWidth);
            int py = static_cast<int>(event.tfinger.y * m_physHeight);
            transformTouchCoords(px, py, m_mouseX, m_mouseY);
            break;
        }
        case SDL_FINGERMOTION:
        {
            int px = static_cast<int>(event.tfinger.x * m_physWidth);
            int py = static_cast<int>(event.tfinger.y * m_physHeight);
            transformTouchCoords(px, py, m_mouseX, m_mouseY);
            break;
        }
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                if (m_rotation != 0)
                    break; // Ignore resize when rotation is active (embedded displays don't resize)

                // Resize viewport and LVGL buffers
                m_nWidth = event.window.data1;
                m_nHeight = event.window.data2;
                SetViewport(m_nWidth, m_nHeight);

                lv_display_set_resolution(m_display, m_nWidth, m_nHeight);

                if (m_lvglRgbaFrameBuffer) free(m_lvglRgbaFrameBuffer);
                m_lvglRgbaFrameBuffer = (uint32_t*)malloc(sizeof(uint32_t) * m_nWidth * m_nHeight);
                memset(m_lvglRgbaFrameBuffer, 0, m_nWidth * m_nHeight * sizeof(uint32_t));

                if (m_lvglTextureGL) glDeleteTextures(1, &m_lvglTextureGL);
                m_lvglTextureGL = CreateTexture(m_lvglRgbaFrameBuffer, m_nWidth, m_nHeight);

                lv_display_set_buffers(m_display, m_lvglRgbaFrameBuffer, nullptr,
                    static_cast<uint32_t>(m_nWidth * m_nHeight * sizeof(uint32_t)),
                    LV_DISPLAY_RENDER_MODE_DIRECT);

                // Force LVGL to redraw after resize
                lv_obj_invalidate(lv_screen_active());

                m_lvglTextureDirty = true;
            }
            break;
        case SDL_QUIT:
            return false; // let base handle quit
        default: break;
        }

        /*
        Input routing rules (summary)
        - LVGL decides "hover hit-testing" using `UIWantCaptureMouse()`:
        * true  => pointer is over a non-root LVGL object (button, slider, etc.)
        * false => pointer is only over the empty root screen/layers
        - Map "captures" the drag after the initial left mouse down is forwarded to the map.
        Once captured, subsequent motion continues to go to the map until the button is released.
        - Mouse wheel is always forwarded to the map (zoom), regardless of UI hover state.
        */
        const bool mapCapturedEvent = mapIsDragging || (event.type == SDL_MOUSEWHEEL);
        const bool uiWantsMouse = UIWantCaptureMouse();

        // If UI wants pointer events AND the map hasn't captured the gesture, consume the event here.
        // (LVGL will use the updated pointer state on the next `lv_timer_handler()` call.)
        if (uiWantsMouse && !mapCapturedEvent)
            return true;

        // Forward to map like BaseLvglWindow (use transformed logical coords)
        switch (event.type)
        {
        case SDL_MOUSEWHEEL:
        {
            ITouchEventsListener* pTouch = GetTouchEventHandler();
            if (pTouch)
            {
                int px, py; SDL_GetMouseState(&px, &py);
                int lx, ly; transformTouchCoords(px, py, lx, ly);
                const int delta = event.wheel.y * kScrollDeltaScale;
                pTouch->handleMouseScrollEvent(delta, lx, ly);
            }
            return true;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            ITouchEventsListener* pTouch = GetTouchEventHandler();
            if (pTouch && event.button.button == SDL_BUTTON_LEFT)
            {
                int px, py; SDL_GetMouseState(&px, &py);
                int lx, ly; transformTouchCoords(px, py, lx, ly);
                pTouch->handleTouchEvent(gem::ETouchEvent::TE_Down, 0, lx, ly);
                mapIsDragging = true;
            }
            return true;
        }
        case SDL_MOUSEBUTTONUP:
        {
            ITouchEventsListener* pTouch = GetTouchEventHandler();
            if (pTouch && event.button.button == SDL_BUTTON_LEFT)
            {
                int px, py; SDL_GetMouseState(&px, &py);
                int lx, ly; transformTouchCoords(px, py, lx, ly);
                pTouch->handleTouchEvent(gem::ETouchEvent::TE_Up, 0, lx, ly);
                mapIsDragging = false;
            }
            return true;
        }
        case SDL_MOUSEMOTION:
        {
            ITouchEventsListener* pTouch = GetTouchEventHandler();
            if (pTouch)
            {
                int px, py; SDL_GetMouseState(&px, &py);
                int lx, ly; transformTouchCoords(px, py, lx, ly);
                pTouch->handleTouchEvent(gem::ETouchEvent::TE_Move, 0, lx, ly);
            }
            return true;
        }
        case SDL_FINGERDOWN:
        {
            ITouchEventsListener* pTouch = GetTouchEventHandler();
            if (pTouch)
            {
                int px = static_cast<int>(event.tfinger.x * m_physWidth);
                int py = static_cast<int>(event.tfinger.y * m_physHeight);
                int lx, ly; transformTouchCoords(px, py, lx, ly);
                pTouch->handleTouchEvent(gem::ETouchEvent::TE_Down, 0, lx, ly);
                mapIsDragging = true;
            }
            return true;
        }
        case SDL_FINGERUP:
        {
            ITouchEventsListener* pTouch = GetTouchEventHandler();
            if (pTouch)
            {
                int px = static_cast<int>(event.tfinger.x * m_physWidth);
                int py = static_cast<int>(event.tfinger.y * m_physHeight);
                int lx, ly; transformTouchCoords(px, py, lx, ly);
                pTouch->handleTouchEvent(gem::ETouchEvent::TE_Up, 0, lx, ly);
                mapIsDragging = false;
            }
            return true;
        }
        case SDL_FINGERMOTION:
        {
            ITouchEventsListener* pTouch = GetTouchEventHandler();
            if (pTouch)
            {
                int px = static_cast<int>(event.tfinger.x * m_physWidth);
                int py = static_cast<int>(event.tfinger.y * m_physHeight);
                int lx, ly; transformTouchCoords(px, py, lx, ly);
                pTouch->handleTouchEvent(gem::ETouchEvent::TE_Move, 0, lx, ly);
            }
            return true;
        }
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                SDL_Event ev;
                std::memset(&ev, 0, sizeof(ev));
                ev.type = SDL_QUIT;
                SDL_PushEvent(&ev);
                return true;
            }
            return false;
        default:
            break;
        }

        return false;
    }

private:
    bool InitLvglCore()
    {
        lv_init();
        return true;
    }

    bool InitLvglCompositionResources()
    {
        m_lvglRgbaFrameBuffer = (uint32_t*)malloc(sizeof(uint32_t) * m_nWidth * m_nHeight);
        if (!m_lvglRgbaFrameBuffer)
            return false;

        memset(m_lvglRgbaFrameBuffer, 0, m_nWidth * m_nHeight * sizeof(uint32_t));

        // GL program + texture for LVGL blit.
        m_blitProgramGL = CreateShaderProgram();
        if (!m_blitProgramGL)
            return false;

        m_lvglTextureGL = CreateTexture(m_lvglRgbaFrameBuffer, m_nWidth, m_nHeight);
        if (!m_lvglTextureGL)
            return false;

        m_lvglTextureDirty = true;
        return true;
    }

    bool InitLvglDisplay()
    {
        // Create display with v9 API.
        m_display = lv_display_create(m_nWidth, m_nHeight);
        lv_display_set_color_format(m_display, LV_COLOR_FORMAT_ARGB8888);

        // Use DIRECT render mode: LVGL renders straight into the
        // full-screen framebuffer (m_lvglRgbaFrameBuffer).  The flush
        // callback just flags a GL texture upload on the last flush.
        // This matches TutorialAppLVGL and eliminates flicker on DPI
        // displays (HyperPixel 4) where partial tile flushes cause
        // visible artifacts.
        lv_display_set_buffers(m_display, m_lvglRgbaFrameBuffer, nullptr,
            static_cast<uint32_t>(m_nWidth * m_nHeight * sizeof(uint32_t)),
            LV_DISPLAY_RENDER_MODE_DIRECT);
        lv_display_set_flush_cb(m_display, &OpenGLContext_LVGL::s_disp_flush);

        // IMPORTANT: Set screen background to fully transparent so the LVGL overlay
        // is alpha-blended on top of the map.
        lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
        return true;
    }

    bool InitLvglInput()
    {
        // Input driver (pointer) — v9 API.
        lv_indev_t* indev = lv_indev_create();
        lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev, &OpenGLContext_LVGL::s_pointer_read);
        return true;
    }

    void InitLvglTiming()
    {
        // Tick thread for LVGL.
        SDL_CreateThread(&OpenGLContext_LVGL::tick_thread, "lvgl_tick", nullptr);

        // No LVGL timer needed — lv_timer_handler() is called directly
        // in renderFrame(). TutorialAppLVGL doesn't use one either.
        // A dummy timer at 10ms caused unnecessary LVGL refresh cycles.
    }

    bool InitExampleUi()
    {
        // External UI init (ImGUI-like) or fallback sanity UI.
        if (m_uiCallbacks.first)
        {
            if (!m_uiCallbacks.first())
                return false;
        }
        else
        {
            create_sanity_ui();
        }
        return true;
    }

private:
    static int tick_thread(void*)
    {
        while (1)
        {
            SDL_Delay(kLvglTickMs);
            lv_tick_inc(kLvglTickMs);
        }
        return 0;
    }

    static void sdl_event_timer_cb(lv_timer_t*)
    {
        // Let SDL pump events regularly (OpenGLContext_SDL::pollEvents does the actual polling)
        // Here we merely ensure LVGL keeps ticking frequently.
    }

    static void s_pointer_read(lv_indev_t* indev, lv_indev_data_t* data)
    {
        (void)indev;
        auto* self = s_instance;
        if (!self) { data->state = LV_INDEV_STATE_RELEASED; data->point.x = 0; data->point.y = 0; return; }
        data->point.x = static_cast<int32_t>(self->m_mouseX);
        data->point.y = static_cast<int32_t>(self->m_mouseY);
        data->state = self->m_mousePressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    }

    static void s_disp_flush(lv_display_t* disp, const lv_area_t* /*area*/, uint8_t* /*px_map*/)
    {
        auto* self = s_instance;
        if (!self) { lv_display_flush_ready(disp); return; }

        // In DIRECT mode LVGL renders straight into m_lvglRgbaFrameBuffer.
        // We only need to flag a GL texture upload on the last flush.
        if (lv_display_flush_is_last(disp))
            self->m_lvglTextureDirty = true;

        lv_display_flush_ready(disp);
    }

    bool UIWantCaptureMouse() const
    {
        lv_point_t pos{ (int32_t)m_mouseX, (int32_t)m_mouseY };

        // Find the top-most LVGL object under the pointer.
        lv_obj_t* obj = lv_indev_search_obj(lv_layer_top(), &pos);
        if (!obj) obj = lv_indev_search_obj(lv_screen_active(), &pos);

        if (!obj)
            return false;

        // IMPORTANT:
        // lv_indev_search_obj will often return the active screen itself even when the UI is "empty".
        // Treat the root screen/layers as "no UI hit", otherwise the map never receives drag events.
        if (obj == lv_screen_active() || obj == lv_layer_top() || obj == lv_layer_sys())
            return false;

        return true;
    }

    // Static callback for button
    static void btn_event_cb(lv_event_t* e)
    {
        auto* self = s_instance;
        if (!self) return;

        // Handle button click (e.g., center map or trigger route calculation)
        //("Button clicked!");
    }

    // Minimal sanity UI (label) so we see something
    void create_sanity_ui()
    {
        // Create a button directly on the active screen
        lv_obj_t* btn = lv_button_create(lv_screen_active());
        lv_obj_set_pos(btn, 10, 10);
        lv_obj_set_size(btn, 120, 50);

        // Add label to button
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, "Click Me");
        lv_obj_center(label);

        // Optional: add event handler
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, nullptr);
    }

    // GL helpers
    static GLuint CreateShaderProgram()
    {
        const char* vertex_shader =
            "attribute vec4 a_position;   \n"
            "attribute vec2 a_texCoord;   \n"
            "varying vec2 v_texCoord;     \n"
            "void main()                  \n"
            "{                            \n"
            "   gl_Position = a_position; \n"
            "   v_texCoord = a_texCoord;  \n"
            "}                            \n";

        const char* fragment_shader =
            "precision mediump float;                            \n"
            "varying vec2 v_texCoord;                            \n"
            "uniform sampler2D s_texture;                        \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
            "}                                                   \n";

        GLuint vs, fs, program;

        vs = glCreateShader(GL_VERTEX_SHADER);
        fs = glCreateShader(GL_FRAGMENT_SHADER);

        GLint length = static_cast<GLint>(strlen(vertex_shader));
        glShaderSource(vs, 1, (const GLchar**)&vertex_shader, &length);
        glCompileShader(vs);

        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            return 1;
        }

        length = static_cast<GLint>(strlen(fragment_shader));
        glShaderSource(fs, 1, (const GLchar**)&fragment_shader, &length);
        glCompileShader(fs);

        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            return 1;
        }

        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glBindAttribLocation(program, 0, "a_position");
        glBindAttribLocation(program, 1, "a_texCoord");
        glLinkProgram(program);
        return program;
    }

    static GLuint CreateTexture(uint32_t* pBuffer, int width, int height)
    {
        GLuint textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBuffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        return textureId;
    }

    static void DrawTexture(GLuint programGL, GLuint textureId)
    {
        GLfloat vVertices[] = { -1.f,  1.f, 0.0f,  // Position 0
            0.0f,  0.0f,        // TexCoord 0
            -1.f, -1.0f, 0.0f,  // Position 1
            0.0f,  1.0f,        // TexCoord 1
            1, -1.f, 0.0f,  // Position 2
            1.0f,  1.0f,        // TexCoord 2
            1.f,  1.f, 0.0f,  // Position 3
            1.0f,  0.0f         // TexCoord 3
        };
        GLushort indices[] =
        { 0, 1, 2, 0, 2, 3 };
        glUseProgram(programGL);

        // Load the vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices);
        // Load the texture coordinate
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // Set the sampler texture unit to 0
        glUniform1i(0, 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

private:
    static OpenGLContext_LVGL* s_instance;

    // GL (LVGL overlay blit)
    GLuint m_blitProgramGL = 0;
    GLuint m_lvglTextureGL = 0;

    // LVGL composition: single framebuffer in DIRECT mode (matches TutorialAppLVGL).
    // LVGL renders directly into this buffer; flush callback flags a GL texture upload.
    uint32_t* m_lvglRgbaFrameBuffer = nullptr;
    bool m_lvglTextureDirty = false;

    // LVGL display
    lv_display_t* m_display = nullptr;
    std::vector<uint8_t> m_lvglDrawBuffer;

    // Pointer state
    int  m_mouseX = 0;
    int  m_mouseY = 0;
    bool m_mousePressed = false;

    UICallbacks m_uiCallbacks;
};

inline OpenGLContext_LVGL* OpenGLContext_LVGL::s_instance = nullptr;

#endif // USE_LVGL && !USE_GLFW
