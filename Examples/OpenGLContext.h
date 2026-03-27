// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#ifndef NOMINMAX
    #define NOMINMAX // exclude min-max defines
#endif

#include <API/GEM_RenderContext.h>

#include <chrono>
#include <cstdio>

namespace gem
{
    class MapView;
}

class ITouchEventsListener
{
public:
	virtual void handleTouchEvent(int eventType, int pointerId, int x, int y) = 0;
	virtual void handleMouseScrollEvent(int delta, int x, int y) = 0;
	virtual void getCursorPosition(int& x, int& y) = 0;
    virtual void setParent( gem::StrongPointer<gem::Screen> parent ) = 0;
};

class UICallbacks : public std::pair<std::function<bool()>/*initialize callback*/, std::function<void(gem::StrongPointer<gem::MapView>)>/*render callback*/>
{
    using TBase = std::pair<std::function<bool()>/*initialize callback*/, std::function<void(gem::StrongPointer<gem::MapView>)>/*render callback*/>;
public:
    UICallbacks() {}
    template<typename Arg1>
    UICallbacks( const Arg1 &arg ) : TBase{ {}, arg } {}
    template<typename Arg1, typename Arg2>
    UICallbacks( const Arg1 &arg1, const Arg2 &arg2 ) : TBase{ arg1, arg2 } {}
};

//OpenGL context info wrapper
class OpenGLContext : public gem::IOpenGLContext
{
public:
	OpenGLContext();

    bool initialize() override
    {
        m_initialized = true;
        return true;
    }
    bool isInitialized() const;

	gem::EImagePixelFormat encoding() const;
	gem::Rect const& viewport() const;
	void SetViewport(int width, int height);
	gem::Rect GetViewport() const;

	void needsRender() override
	{
		m_needsRender = true;
#ifdef LOG_RENDER_FPS
		++m_needsRenderCount;
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_needsRenderWindowStart).count();
		if (elapsed >= 1000)
		{
			std::printf("needsRender rate: %d calls/sec\n", m_needsRenderCount);
			m_needsRenderCount = 0;
			m_needsRenderWindowStart = now;
		}
#endif
	}
	void clearNeedsRender() { m_needsRender = false; }
	bool isNeedsRender() { return m_needsRender; }

	int getDpi() const
	{
		return int(m_dpi);
	}
	void setDpi(int dpi)
	{
		if ( dpi > 0 )
			m_dpi = dpi;
	}

    void setFontScaleFactor(float factor)
    {
        if ( factor > 0.f )
            m_fontScaleFactor = factor;
    }

    void setTextureScaleFactor(float factor)
    {
        if ( factor > 0.f )
            m_textureScaleFactor = factor;
    }

    float getPixelRatio() const override
    {
        return m_pixelRatio;
    }

    void setPixelRatio(float ratio)
    {
        if ( ratio > 0.f )
            m_pixelRatio = ratio;
    }

    std::pair<float, bool> getFontModifiers() const override
    {
        return { m_fontScaleFactor, false };
    }

    float getTextureScaleFactor() const override
    {
        return m_textureScaleFactor;
    }

    void attached( gem::Screen& screen ) override;
    void prepare();

	virtual bool shouldClose() const = 0;
    virtual void pollEvents() = 0;
    virtual bool renderFrame();

	ITouchEventsListener* GetTouchEventHandler();

    static gem::StrongPointer<OpenGLContext> Produce_GLFW( std::string windowName, ITouchEventsListener* pTouchEventListener, gem::Size windowSize );
    static gem::StrongPointer<OpenGLContext> Produce_SDL( std::string windowName, ITouchEventsListener* pTouchEventListener, gem::Size windowSize, int rotation = 0 );
    static gem::StrongPointer<OpenGLContext> Produce_ImGUI( std::string windowName, ITouchEventsListener* pTouchEventListener, UICallbacks callbacks, gem::Size windowSize, int rotation = 0 );
    static gem::StrongPointer<OpenGLContext> Produce_LVGL(std::string windowName, ITouchEventsListener* pTouchEventListener, UICallbacks callbacks, gem::Size windowSize, int rotation = 0 );

    bool isRotationSwapped() const { return m_rotation == 90 || m_rotation == 270; }
    void setRotation(int r) { m_rotation = r; }
    int getRotation() const { return m_rotation; }

protected:
    int m_nWidth;
    int m_nHeight;
    int m_physWidth = 0;
    int m_physHeight = 0;
    int m_rotation = 0;
	bool m_initialized;
	bool m_needsRender = true;
#ifdef LOG_RENDER_FPS
	int m_needsRenderCount = 0;
	std::chrono::steady_clock::time_point m_needsRenderWindowStart = std::chrono::steady_clock::now();
#endif

#ifdef LOG_RENDER_FPS
	int m_renderFrameCount = 0;
	std::chrono::steady_clock::time_point m_renderWindowStart = std::chrono::steady_clock::now();
#endif
	gem::Rect m_viewport;
	int m_dpi = 96;
    float m_pixelRatio = 1.0f;
	ITouchEventsListener* m_pTouchEventListener;
    gem::WeakPointer<gem::Screen> m_screen;

    // modifiers
    float m_fontScaleFactor = 1.f;
    float m_textureScaleFactor = 1.f;
};
