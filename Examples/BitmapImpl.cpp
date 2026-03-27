// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "BitmapImpl.h"

#include <GLES2/gl2platform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <cstring>

BitmapImpl::BitmapImpl( int width, int height )
    : m_buffer( nullptr )
{
    resize( width, height );
}

BitmapImpl::~BitmapImpl()
{
    if( m_buffer )
    {
        delete[] m_buffer;
    }
}

unsigned int BitmapImpl::LoadTextureIntoGPU(int width, int height, void* data)
{
    unsigned int textureId;

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Required on WebGL for non power-of-two textures

    // Upload pixels into texture
#ifdef GL_UNPACK_ROW_LENGTH
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    return textureId;
}

gem::EImagePixelFormat BitmapImpl::encoding() const
{
    return gem::EImagePixelFormat::ABGR_8888;
}

const gem::Rect& BitmapImpl::viewport() const
{
    return m_viewport;
}

void* BitmapImpl::begin()
{
    return (void*)m_buffer;
}

void BitmapImpl::end()
{

}

gem::Size BitmapImpl::size() const
{
    return m_size;
}

int BitmapImpl::alignment() const
{
    return 1;
}

void BitmapImpl::clear()
{
    memset( m_buffer, 0xFFFFFFFF, (size_t)m_size.width * (size_t)m_size.height * 4 );
}

void BitmapImpl::resize( int w, int h )
{
    m_size = gem::Size( w, h );
    m_viewport = gem::Rect( 0, 0, w, h );

    if( m_buffer )
        delete[] m_buffer;

    if( w && h )
    {
        m_buffer = new unsigned char[(size_t)w * (size_t)h * 4];
        clear();
    }
}
