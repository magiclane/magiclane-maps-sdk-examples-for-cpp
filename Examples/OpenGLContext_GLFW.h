// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#ifdef USE_GLFW

#if defined(__MINGW32__) || defined(__MINGW64__)
	#include <glad/glad.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace
{
    static void glfwErrorCallback(int error, const char* description) {}

    void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods ) {}

    void mouseButtonCallback( GLFWwindow* window, int button, int action, int mods )
    {
        OpenGLContext* pOpenGLContext = (OpenGLContext*)glfwGetWindowUserPointer( window );
        long long int inputTime = gem::Time::getUniversalTime().asInt();
        //handle TouchEvent will receive as first parameter 0 - Touch/Mouse Down , 1 - Mouse Move / Touch Move ,2 - Touch Up/ Mouse up
        //fprintf(stderr,"mouseButtonCallback( button %d action %d mods %d ) t %lld\n",button,action,mods,(long long int)inputTime);
        if( pOpenGLContext && pOpenGLContext->GetTouchEventHandler() )
        {
            //get the stored cursor position because a button event does not include it
            int xpos, ypos;
            pOpenGLContext->GetTouchEventHandler()->getCursorPosition( xpos, ypos );
            if( button == GLFW_MOUSE_BUTTON_LEFT )
            {
                if( action == GLFW_PRESS )
                {
                    pOpenGLContext->GetTouchEventHandler()->handleTouchEvent( gem::ETouchEvent::TE_Down, 0, xpos, ypos );
                }
                else if( action == GLFW_RELEASE )
                {
                    pOpenGLContext->GetTouchEventHandler()->handleTouchEvent( gem::ETouchEvent::TE_Up, 0, xpos, ypos );
                }
            }
        }
    }

    void cursorPositionCallback( GLFWwindow* window, double xpos, double ypos )
    {
        OpenGLContext* pOpenGLContext = (OpenGLContext*)glfwGetWindowUserPointer( window );
        if( pOpenGLContext )
        {
            // Convert to logical pixels
            float pixelRatio = pOpenGLContext->getPixelRatio();
            int logicalX = static_cast<int>(xpos / pixelRatio);
            int logicalY = static_cast<int>(ypos / pixelRatio);

            //xpos and ypos needs to pe integer(pixels)
            //fprintf(stderr,"cursorPositionCallback( %.3f %.3f )\n",xpos,ypos);
            if( pOpenGLContext->GetTouchEventHandler() )
            {
                pOpenGLContext->GetTouchEventHandler()->handleTouchEvent( gem::ETouchEvent::TE_Move, 0, (int)logicalX, (int)logicalY );
            }
        }
    }

    void mouseScrollCallback( GLFWwindow* window, double xoffset, double yoffset )
    {
        OpenGLContext* pOpenGLContext = (OpenGLContext*)glfwGetWindowUserPointer( window );
        if( pOpenGLContext && pOpenGLContext->GetTouchEventHandler() )
        {
            int xpos, ypos;
            pOpenGLContext->GetTouchEventHandler()->getCursorPosition( xpos, ypos );
            //xpos and ypos needs to pe integer(pixels)
            //fprintf(stderr, "mouseScrollCallback( %.3f %.3f )\n", xoffset, yoffset);
            if( pOpenGLContext->GetTouchEventHandler() )
            {
                pOpenGLContext->GetTouchEventHandler()->handleMouseScrollEvent( (int)yoffset, (int)xpos, (int)ypos );
            }
        }
    }

    void framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        OpenGLContext* pOpenGLContext = (OpenGLContext*)glfwGetWindowUserPointer(window);
        if (pOpenGLContext)
        {
            float pixelRatio = pOpenGLContext->getPixelRatio();
            int logicalWidth = static_cast<int>(width / pixelRatio);
            int logicalHeight = static_cast<int>(height / pixelRatio);

            pOpenGLContext->SetViewport(logicalWidth + 1, logicalHeight + 1);
        }
    }
}

// OpenGL context for GLFW window system
class OpenGLContext_GLFW : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce(std::string windowName, ITouchEventsListener* pEventTouchListener, gem::Size windowSize)
    {
        auto ptr = gem::StrongPointerFactory<OpenGLContext_GLFW>();

        // Default size
        int nWidth = 800;
        int nHeight = 600;

        if (windowSize.width > 0 && windowSize.height > 0)
        {
            nWidth = windowSize.width;
            nHeight = windowSize.height;
        }

        windowName = "Maps SDK for C++ Samples - " + windowName;

        float nXScale = 1.0f, nYScale = 1.0f;

        glfwSetErrorCallback(glfwErrorCallback);

        if (!glfwInit())
        {
            glfwTerminate();
            return gem::StrongPointer<OpenGLContext>();
        }

#if defined(__MINGW32__) || defined(__MINGW64__)
        // Load GL extensions using glad
        if ( !gladLoadGLLoader((GLADloadproc) glfwGetProcAddress) )
        {
            glfwTerminate();
            return gem::StrongPointer<OpenGLContext>();
        }
#endif

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        /* Force EGL on Windows and non-desktop GLES -- needed by ANGLE:
        https://stackoverflow.com/a/58904181/4084782 . Might be useful on
        other platforms as well (Mac?), not tested yet. */
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
        /* Totally unused; issue: https://github.com/microsoft/vcpkg/issues/30142 */
        eglGetError();
#endif
        ptr->m_window = glfwCreateWindow(nWidth, nHeight, windowName.c_str(), NULL, NULL);
        if (!ptr->m_window)
        {
            glfwTerminate();
            return gem::StrongPointer<OpenGLContext>();
        }
        glfwSetFramebufferSizeCallback(ptr->m_window, framebufferSizeCallback);
        glfwSetKeyCallback(ptr->m_window, keyCallback);
        glfwSetMouseButtonCallback(ptr->m_window, mouseButtonCallback);
        glfwSetCursorPosCallback(ptr->m_window, cursorPositionCallback);
        glfwSetScrollCallback(ptr->m_window, mouseScrollCallback);

        glfwMakeContextCurrent(ptr->m_window);
        GLFWmonitor* const monitor = glfwGetPrimaryMonitor();
        if (monitor)
            glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &nXScale, &nYScale);

        ptr->setPixelRatio(std::max<float>(nXScale, nYScale));

        ptr->SetViewport(nWidth, nHeight);

        ptr->m_pTouchEventListener = pEventTouchListener;
        glfwSetWindowUserPointer( ptr->m_window, ptr.get() );

        ptr->initialize();

        return ptr;
    }

    ~OpenGLContext_GLFW() override
    {
        if( m_window )
            glfwDestroyWindow( m_window );

        glfwTerminate();
    }

private:
    bool makeCurrent() override
    {
        if( !m_initialized )
	        return false;

        glfwMakeContextCurrent(m_window);

        return true;
    }

    bool doneCurrent() override
    {
        if( !m_initialized )
	        return false;


        glfwSwapBuffers(m_window);

        return true;
    }

    bool shouldClose() const override
    {
        if( m_window )
    		return glfwWindowShouldClose( m_window );

        return true;
    }

    void pollEvents() override
    {
        glfwPollEvents();
    }

private:
    GLFWwindow* m_window = nullptr;
};

#else

class OpenGLContext_GLFW : public OpenGLContext
{
public:
    static gem::StrongPointer<OpenGLContext> Produce( std::string, ITouchEventsListener*, gem::Size )
    {
        return gem::StrongPointer<OpenGLContext>();
    }
};

#endif // USE_GLFW
