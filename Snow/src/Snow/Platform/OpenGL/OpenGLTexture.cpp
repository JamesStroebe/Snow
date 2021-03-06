#include <spch.h>
#include "Snow/Platform/OpenGL/OpenGLTexture.h"

#include <glad/glad.h>

#include <stb_image.h>

namespace Snow {
    namespace Render {

        GLenum SnowToOpenGLFormat(API::TextureFormat format) {
            switch(format) {
            case API::TextureFormat::RGB:   return GL_RGB;
            case API::TextureFormat::RGBA:   return GL_RGBA;
            case API::TextureFormat::Float16:   return GL_RGBA16F;
            }
            return 0;
        }

        GLenum SnowToOpenGLWrap(API::TextureWrap wrap) {
            switch(wrap) {
            case API::TextureWrap::Repeat:   return GL_REPEAT;
            case API::TextureWrap::Clamp:   return GL_CLAMP;
            }
            return 0;
        }

        OpenGLTexture2D::OpenGLTexture2D(API::TextureFormat format, uint32_t width, uint32_t height, API::TextureWrap wrap) :
            m_Format(format), m_Width(width), m_Height(height), m_Wrap(wrap), m_Locked(false) {

            

            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, SnowToOpenGLWrap(m_Wrap));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, SnowToOpenGLWrap(m_Wrap));

            glTexImage2D(GL_TEXTURE_2D, 0, SnowToOpenGLFormat(m_Format), m_Width, m_Height, 0, SnowToOpenGLFormat(m_Format), GL_UNSIGNED_BYTE, nullptr);

            glBindTexture(GL_TEXTURE_2D, 0);

            m_ImageData.Allocate(width * height * API::Texture::GetBPP(m_Format));
        }

        OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb) :
            m_Path(path), m_Locked(false) {
            int width, height, channels;
            stbi_set_flip_vertically_on_load(true);

            m_ImageData.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
            m_Format = channels == 4 ? API::TextureFormat::RGBA : API::TextureFormat::RGB;

            if(!m_ImageData.Data)
                return;

            m_Width = width;
            m_Height = height;

            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            GLenum internalFormat = SnowToOpenGLFormat(m_Format);
            GLenum format = srgb ? GL_SRGB8 : (SnowToOpenGLFormat(m_Format));
            GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, type, m_ImageData.Data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);

            stbi_image_free(m_ImageData.Data);
        }

        OpenGLTexture2D::~OpenGLTexture2D() {
            glDeleteTextures(1, &m_RendererID);
        }

        void OpenGLTexture2D::Bind(uint32_t slot) const {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, m_RendererID);
        }

        void OpenGLTexture2D::ResizeBuffer(uint32_t width, uint32_t height) {
            SNOW_CORE_ASSERT(m_Locked, "Trying to write to texture without it being locked");
            m_ImageData.Allocate(width * height * API::Texture::GetBPP(m_Format));
            m_ImageData.ZeroInitialize();
        }

        Buffer OpenGLTexture2D::GetWriteableBuffer() {
            SNOW_CORE_ASSERT(m_Locked, "Texture is currently uploaded to gpu");
            return m_ImageData;
        }

        void OpenGLTexture2D::Lock() {
            m_Locked = true;
        }

        void OpenGLTexture2D::Unlock() {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, SnowToOpenGLFormat(m_Format), GL_UNSIGNED_BYTE, m_ImageData.Data);
            m_Locked = false;
        }

        void OpenGLTexture2D::SetData(void* data, uint32_t size) {
            uint32_t bbp = SnowToOpenGLFormat(m_Format) == GL_RGBA ? 4 : 3;
            if (!(size == m_Width * m_Height * bbp)) SNOW_CORE_ERROR("Data must be entire texture");
            glBindTexture(GL_TEXTURE_2D, m_RendererID);
            glTexImage2D(GL_TEXTURE_2D, 0, SnowToOpenGLFormat(m_Format), m_Width, m_Height, 0, SnowToOpenGLFormat(m_Format), GL_UNSIGNED_BYTE, data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        OpenGLTextureCube::OpenGLTextureCube(const std::string& path, bool srgb) :
            m_Path(path) {

            int width, height, channels;
            stbi_set_flip_vertically_on_load(true);

            m_ImageData.Data = stbi_load(path.c_str(), &width, &height, &channels, srgb ? STBI_rgb : STBI_rgb_alpha);
            m_Format = channels == 4 ? API::TextureFormat::RGBA : API::TextureFormat::RGB;

            if (!m_ImageData.Data)
                return;

            m_Width = width;
            m_Height = height;

            std::array<byte*, 6> cubeTextureData;

            uint32_t cx = 0, cy = 0;
            uint32_t yLoop = 0, xLoop = 0;
            uint32_t face = 0;
            if (m_Height < m_Width) {
                cy = m_Height / 3;
                cx = m_Width / 4;
                yLoop = 3;
                xLoop = 4;
            }
            else if (m_Width < m_Height) {
                cy = m_Height / 4;
                cx = m_Width / 3;
                yLoop = 4;
                xLoop = 3;
            }

            for (uint32_t sy = 0; sy < yLoop; sy++) {
                for (uint32_t sx = 0; sx < xLoop; sx++) {
                    if (yLoop == 4) {
                        if (sy == 0 || sy == 2 || sy == 3) {
                            if (sx != 1)
                                continue;
                        }
                    }
                    else if (xLoop == 4) {
                        if (sx == 0 || sx == 2 || sx == 3) {
                            if (sy != 1)
                                continue;
                        }
                    }
                    size_t totalbits = (cx * cy * channels);
                    cubeTextureData[face] = new byte[totalbits];
                    uint32_t index = 0;
                    for (uint32_t y = 0; y < cy; y++) {
                        uint32_t offset = y;

                        uint32_t yp = sy * cy + offset;
                        for (uint32_t x = 0; x < cx; x++) {
                            offset = x;

                            uint32_t xp = sx * cx + offset;
                            for (uint32_t c = 0; c < channels; c++)
                                cubeTextureData[face][(x + y * cx) * channels + c] = m_ImageData.Data[(xp + yp * m_Width) * channels + c];
                        }
                    }
                    face++;
                }
            }

            glGenTextures(1, &m_RendererID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

            GLenum internalFormat = SnowToOpenGLFormat(m_Format);
            GLenum format = srgb ? GL_SRGB8 : (SnowToOpenGLFormat(m_Format));
            GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, cx, cy, 0, format, type, cubeTextureData[3]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalFormat, cx, cy, 0, format, type, cubeTextureData[1]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalFormat, cx, cy, 0, format, type, cubeTextureData[0]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalFormat, cx, cy, 0, format, type, cubeTextureData[xLoop == 4 ? 5 : 4]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalFormat, cx, cy, 0, format, type, cubeTextureData[2]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalFormat, cx, cy, 0, format, type, cubeTextureData[xLoop == 4 ? 4 : 5]);

            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

            for (uint32_t f = 0; f < 6; f++) {
                delete[] cubeTextureData[f];
            }
            

            stbi_image_free(m_ImageData.Data);
        }

        OpenGLTextureCube::~OpenGLTextureCube() {
            glDeleteTextures(1, &m_RendererID);
        }

        void OpenGLTextureCube::Bind(uint32_t slot) const {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
        }
    }
}