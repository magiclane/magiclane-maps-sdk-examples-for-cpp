// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#ifndef USE_GLFW

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengles2.h>
#if defined(__unix__)
    #include <unistd.h>
#endif

#include <cstring>

// OpenGL context for SDL window system
class OpenGLContext_SDL : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce(std::string windowName, ITouchEventsListener* pEventTouchListener, gem::Size windowSize, int rotation = 0)
    {
        auto ptr = gem::StrongPointerFactory<OpenGLContext_SDL>();
        ptr->m_rotation = rotation;

        if( !ptr->initialize( windowName, pEventTouchListener, windowSize ) )
            ptr.reset();

        return ptr;
    }
    ~OpenGLContext_SDL() override
    {
        cleanupRotationFBO();
        SDL_Quit();
    }

protected:
    virtual bool initialize( std::string windowName, ITouchEventsListener* pEventTouchListener, gem::Size windowSize )
    {
        // Determine logical size (what the app/SDK sees)
        int logW, logH;
        if(windowSize.width > 0 && windowSize.height > 0)
        {
            logW = windowSize.width;
            logH = windowSize.height;
        }
        else
        {
            // Default size
            logW = 800;
            logH = 600;
        }

        // Physical window size: for 90/270 rotation, swap so SDL window matches display orientation
        if (isRotationSwapped())
        {
            m_physWidth = logH;
            m_physHeight = logW;
        }
        else
        {
            m_physWidth = logW;
            m_physHeight = logH;
        }

        // SDL window is always at physical size
        m_nWidth = logW;
        m_nHeight = logH;

        windowName = "Maps SDK for C++ Samples - " + windowName;

        if( ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) == -1 ) )
        {
            printf( "Could not initialize SDL: %s\n", SDL_GetError() );
            return false;
        }

#ifdef __WIN32__
        SDL_SetHint( SDL_HINT_OPENGL_ES_DRIVER, "1" );
        SDL_SetHint( SDL_HINT_VIDEO_WIN_D3DCOMPILER, "none" );
        /* Totally unused; issue: https://github.com/microsoft/vcpkg/issues/30142 */
        eglGetError();
#endif

#if defined(USE_IMGUI)
    #if defined(IMGUI_IMPL_OPENGL_ES3)
        // GL ES 3.0 + GLSL 300 es (ANGLE provides OpenGL ES 3.0)
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    #elif defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    #else
        // Desktop OpenGL - GL 3.0 + GLSL 130
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    #endif
#else
    // Default to GL ES 2.0 for ANGLE compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
        // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
        SDL_SetHint( SDL_HINT_IME_SHOW_UI, "1" );
#endif
        // Create window with graphics context
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
        SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

        m_window = SDL_CreateWindow( windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_physWidth, m_physHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI );
        if( !m_window )
        {
            return false;
        }

        int m_width = 0, m_height = 0, ndwidth = 0, ndheight = 0;
        SDL_GetWindowSize( m_window, &m_width, &m_height );
        SDL_GL_GetDrawableSize( m_window, &ndwidth, &ndheight );
        float ddpi = 120, dr2, dr3;
        int intErr = SDL_GetDisplayDPI( 0, &ddpi, &dr2, &dr3 );
        int openglversiona = 0;
        int openglversionb = 0;
        SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &openglversiona );
        SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &openglversionb );

        m_context = SDL_GL_CreateContext( m_window );

        SDL_GL_MakeCurrent( m_window, m_context );
        SDL_GL_SetSwapInterval( 1 ); // Enable vsync

        int vsync = SDL_GL_GetSwapInterval();

        // Initialize rotation FBO if needed (after GL context is ready)
        if (m_rotation != 0)
        {
            if (!initRotationFBO())
                return false;
        }

        // SDK sees logical size
        SetViewport( m_nWidth, m_nHeight );
        m_pTouchEventListener = pEventTouchListener;

        return OpenGLContext::initialize();
    }

    bool makeCurrent()
    {
        if ( !m_initialized )
            return false;

        if (SDL_GL_MakeCurrent(m_window, m_context) < 0)
            return false;

        // Bind FBO so the map SDK renders into the off-screen texture
        if (m_rotFBO != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_rotFBO);
            glViewport(0, 0, m_nWidth, m_nHeight);
        }

        return true;
    }

    bool doneCurrent()
    {
        if ( !m_initialized )
            return false;

        // Blit the FBO texture rotated to the physical window
        if (m_rotFBO != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, m_physWidth, m_physHeight);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            glDisable(GL_SCISSOR_TEST);
            DrawRotatedTexture(m_rotBlitProgram, m_rotColorTex, m_rotation);
        }

        SDL_GL_SwapWindow(m_window);
        return true;
    }

    bool shouldClose() const override
    {
        return m_bShouldClose;
    }

    void pollEvents() override
    {
        SDL_Event event;
        while( SDL_PollEvent( &event ) != 0 )
        {
            if( processEvent( event ) )
                //process SDL specializations
                continue;

            switch( event.type )
            {
            case SDL_QUIT:
            {
                m_bShouldClose = true;
                break;
            }
            case SDL_KEYDOWN:
            {
                if( event.key.keysym.sym == SDLK_ESCAPE )
                    m_bShouldClose = true;
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                ITouchEventsListener* pTouchHandler = GetTouchEventHandler();
                if(pTouchHandler)
                {
                    if( event.button.button == SDL_BUTTON_LEFT )
                    {
                        int x, y;
                        SDL_GetMouseState( &x, &y );
                        int lx, ly;
                        transformTouchCoords(x, y, lx, ly);
                        pTouchHandler->handleTouchEvent( gem::ETouchEvent::TE_Down, 0, lx, ly );
                    }
                    break;
                }
            }
            case SDL_MOUSEBUTTONUP:
            {
                ITouchEventsListener* pTouchHandler = GetTouchEventHandler();
                if(pTouchHandler)
                {
                    if( event.button.button == SDL_BUTTON_LEFT )
                    {
                        int x, y;
                        SDL_GetMouseState( &x, &y );
                        int lx, ly;
                        transformTouchCoords(x, y, lx, ly);
                        pTouchHandler->handleTouchEvent( gem::ETouchEvent::TE_Up, 0, lx, ly );
                    }
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                ITouchEventsListener* pTouchHandler = GetTouchEventHandler();
                if(pTouchHandler)
                {
                    int x, y;
                    SDL_GetMouseState( &x, &y );
                    int lx, ly;
                    transformTouchCoords(x, y, lx, ly);
                    pTouchHandler->handleTouchEvent( gem::ETouchEvent::TE_Move, 0, lx, ly );
                }
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                ITouchEventsListener* pTouchHandler = GetTouchEventHandler();
                if(pTouchHandler)
                {
                    int x, y;
                    SDL_GetMouseState( &x, &y );
                    int lx, ly;
                    transformTouchCoords(x, y, lx, ly);
                    auto nMouseScroll = event.wheel.y * 1000;
                    pTouchHandler->handleMouseScrollEvent( nMouseScroll, lx, ly );
                }
                break;
            }
            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED
                    || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    if (m_rotation == 0)
                    {
                        // Resize viewport
                        m_nWidth = event.window.data1;
                        m_nHeight = event.window.data2;
                        m_physWidth = m_nWidth;
                        m_physHeight = m_nHeight;
                        SetViewport(m_nWidth, m_nHeight);
                    }
                    // When rotation is active, ignore resize (embedded displays don't resize)
                }
            }
            default: {}
            }
        }
    }

    virtual bool processEvent( SDL_Event &event ) { return false; }

    // Transform physical (SDL window) coordinates to logical (rotated) coordinates
    void transformTouchCoords(int physX, int physY, int& logX, int& logY) const
    {
        switch (m_rotation)
        {
        case 90:
            logX = physY;
            logY = m_physWidth - 1 - physX;
            break;
        case 180:
            logX = m_physWidth - 1 - physX;
            logY = m_physHeight - 1 - physY;
            break;
        case 270:
            logX = m_physHeight - 1 - physY;
            logY = physX;
            break;
        default: // 0
            logX = physX;
            logY = physY;
            break;
        }
    }

    // Initialize the off-screen FBO for rotation rendering
    bool initRotationFBO()
    {
        // Create color texture at logical (rotated) size
        glGenTextures(1, &m_rotColorTex);
        glBindTexture(GL_TEXTURE_2D, m_rotColorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_nWidth, m_nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Create depth+stencil renderbuffer
        // GL ES does not define GL_DEPTH24_STENCIL8 / GL_DEPTH_STENCIL_ATTACHMENT
        // in the core spec; use the OES extension constant when available,
        // otherwise fall back to depth-only.
        glGenRenderbuffers(1, &m_rotDepthRB);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rotDepthRB);
#if defined(GL_DEPTH24_STENCIL8_OES)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, m_nWidth, m_nHeight);
#else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_nWidth, m_nHeight);
#endif

        // Create FBO
        glGenFramebuffers(1, &m_rotFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_rotFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rotColorTex, 0);
#if defined(GL_DEPTH24_STENCIL8_OES)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rotDepthRB);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rotDepthRB);
#else
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rotDepthRB);
#endif

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Rotation FBO incomplete: 0x%x\n", status);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Create blit shader program
        m_rotBlitProgram = createRotationBlitProgram();
        if (!m_rotBlitProgram)
            return false;

        return true;
    }

    void cleanupRotationFBO()
    {
        if (m_rotFBO) { glDeleteFramebuffers(1, &m_rotFBO); m_rotFBO = 0; }
        if (m_rotColorTex) { glDeleteTextures(1, &m_rotColorTex); m_rotColorTex = 0; }
        if (m_rotDepthRB) { glDeleteRenderbuffers(1, &m_rotDepthRB); m_rotDepthRB = 0; }
        if (m_rotBlitProgram) { glDeleteProgram(m_rotBlitProgram); m_rotBlitProgram = 0; }
    }

    static GLuint createRotationBlitProgram()
    {
        const char* vertSrc =
            "attribute vec4 a_position;   \n"
            "attribute vec2 a_texCoord;   \n"
            "varying vec2 v_texCoord;     \n"
            "void main()                  \n"
            "{                            \n"
            "   gl_Position = a_position; \n"
            "   v_texCoord = a_texCoord;  \n"
            "}                            \n";

        const char* fragSrc =
            "precision mediump float;                            \n"
            "varying vec2 v_texCoord;                            \n"
            "uniform sampler2D s_texture;                        \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
            "}                                                   \n";

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        GLint length = static_cast<GLint>(std::strlen(vertSrc));
        glShaderSource(vs, 1, &vertSrc, &length);
        glCompileShader(vs);
        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) return 0;

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        length = static_cast<GLint>(std::strlen(fragSrc));
        glShaderSource(fs, 1, &fragSrc, &length);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) { glDeleteShader(vs); return 0; }

        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glBindAttribLocation(program, 0, "a_position");
        glBindAttribLocation(program, 1, "a_texCoord");
        glLinkProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    }

    // Draw FBO texture as a fullscreen quad with rotation-dependent UVs.
    //
    // FBO texture uses standard GL convention: UV (0,0) = bottom-left of
    // rendered scene, (0,1) = top-left, (1,1) = top-right.
    //
    // Quad vertices (clip space):
    //   TL(-1, 1) = screen top-left      BL(-1,-1) = screen bottom-left
    //   BR( 1,-1) = screen bottom-right   TR( 1, 1) = screen top-right
    //
    // For each rotation we map screen corners to the scene corner that
    // should appear there after rotating the scene CW by the given angle.
    static void DrawRotatedTexture(GLuint programGL, GLuint textureId, int rotation)
    {
        float u0, v0, u1, v1, u2, v2, u3, v3;
        switch (rotation)
        {
        case 90:
            // 90° CW: scene left edge → screen top
            // TL←scene-BL  BL←scene-BR  BR←scene-TR  TR←scene-TL
            u0 = 0.f; v0 = 0.f;   // TL = scene bottom-left
            u1 = 1.f; v1 = 0.f;   // BL = scene bottom-right
            u2 = 1.f; v2 = 1.f;   // BR = scene top-right
            u3 = 0.f; v3 = 1.f;   // TR = scene top-left
            break;
        case 180:
            // 180°: scene flipped both axes
            // TL←scene-BR  BL←scene-TR  BR←scene-TL  TR←scene-BL
            u0 = 1.f; v0 = 0.f;   // TL = scene bottom-right
            u1 = 1.f; v1 = 1.f;   // BL = scene top-right
            u2 = 0.f; v2 = 1.f;   // BR = scene top-left
            u3 = 0.f; v3 = 0.f;   // TR = scene bottom-left
            break;
        case 270:
            // 270° CW: scene right edge → screen top
            // TL←scene-TR  BL←scene-TL  BR←scene-BL  TR←scene-BR
            u0 = 1.f; v0 = 1.f;   // TL = scene top-right
            u1 = 0.f; v1 = 1.f;   // BL = scene top-left
            u2 = 0.f; v2 = 0.f;   // BR = scene bottom-left
            u3 = 1.f; v3 = 0.f;   // TR = scene bottom-right
            break;
        default: // 0 (identity, no flip — FBO bottom-left maps to screen bottom-left)
            u0 = 0.f; v0 = 1.f;   // TL = scene top-left
            u1 = 0.f; v1 = 0.f;   // BL = scene bottom-left
            u2 = 1.f; v2 = 0.f;   // BR = scene bottom-right
            u3 = 1.f; v3 = 1.f;   // TR = scene top-right
            break;
        }

        GLfloat vVertices[] = {
            -1.f,  1.f, 0.0f,   u0, v0,   // TL
            -1.f, -1.f, 0.0f,   u1, v1,   // BL
             1.f, -1.f, 0.0f,   u2, v2,   // BR
             1.f,  1.f, 0.0f,   u3, v3    // TR
        };
        GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

        glUseProgram(programGL);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(0, 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

protected:
    SDL_GLContext m_context = nullptr;
    SDL_Window* m_window = nullptr;
    bool m_bShouldClose = false;

    // Rotation FBO resources
    GLuint m_rotFBO = 0;
    GLuint m_rotColorTex = 0;
    GLuint m_rotDepthRB = 0;
    GLuint m_rotBlitProgram = 0;
};

#else

class OpenGLContext_SDL : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce( std::string, ITouchEventsListener*, gem::Size, int = 0 )
    {
        return gem::StrongPointer<OpenGLContext>();
    }
};

#endif // !USE_GLFW
