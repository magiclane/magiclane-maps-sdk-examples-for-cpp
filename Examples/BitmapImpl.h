// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once

#include <API/GEM_RenderContext.h>

class BitmapImpl : public gem::IBitmap
{
public:
    BitmapImpl( int width, int height );
    ~BitmapImpl();

    static unsigned int LoadTextureIntoGPU(int width, int height, void* data);

    // gem::IRenderContext methods
    gem::EImagePixelFormat encoding() const override;

    const gem::Rect& viewport() const override;

    // gem::IBitmap methods
    void* begin() override;
    void end() override;

    gem::Size size() const override;
    
    int alignment() const override;

    void clear();

    void resize( int w, int h );

private:
    gem::Size m_size;
    gem::Rect m_viewport;
    unsigned char* m_buffer;
};
