#include "gl/raii/texture2d.hpp"

#include "stb/stb_image.h"

#include "spdlog/spdlog.h"

#include <cmath>
#include <array>

gl::texture2d::~texture2d() {
    if (m_id != 0) glDeleteTextures(1, &m_id);
}

gl::texture2d::texture2d(texture2d&& other) noexcept {
    m_id = other.m_id;
    m_width = other.m_width;
    m_height = other.m_height;
    m_mips = other.m_mips;

    other.m_id = 0;
    other.m_width = 0;
    other.m_height = 0;
    other.m_mips = 0;
}

gl::texture2d& gl::texture2d::operator=(texture2d&& other) noexcept {
    if (this != &other) {
        if (m_id != 0) {
            SPDLOG_TRACE("texture2d move-assign discarded an existing texture");
            glDeleteTextures(1, &m_id);
        }
        m_id = other.m_id;
        m_width = other.m_width;
        m_height = other.m_height;
        m_mips = other.m_mips;

        other.m_id = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_mips = 0;
    }
    return *this;
}

bool gl::texture2d::create(GLsizei width, GLsizei height, GLenum internalFormat, bool mipmapping) {
    if (width <= 0 || height <= 0) {
        SPDLOG_ERROR("texture2d::create: invalid dimensions {}x{}", width, height);
        return false;
    }

    if (m_id != 0) {
        SPDLOG_DEBUG("texture2d::create: reallocating existing texture");
        glDeleteTextures(1, &m_id);
    }

    m_id = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);

    if (m_id == 0) {
        SPDLOG_ERROR("glCreateTextures failed");
        return false;
    }

    GLsizei mips = 1;
    if (mipmapping) mips = 1 + std::floor(std::log2(std::max(width, height)));

    glTextureStorage2D(m_id, mips, internalFormat, width, height);

    m_width = width;
    m_height = height;
    m_mips = mips;

    return true;
}

bool gl::texture2d::create(const char* file, GLenum internalFormat, bool mipmapping) {
    stbi_set_flip_vertically_on_load(true);

    void* data = nullptr;
    bool hdr = stbi_is_hdr(file);
    GLenum dataType = hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

    int w, h, n; w = h = n  = 0;
    if (hdr) data = stbi_loadf(file, &w, &h, &n, 0);
    else data = stbi_load(file, &w, &h, &n, 0);

    if (n < 1 || n > 4) { SPDLOG_ERROR("unsupported channel count {}", n); stbi_image_free(data); return false; }

    if (data == nullptr) {
        [[maybe_unused]] const char* reason = stbi_failure_reason();
        SPDLOG_ERROR("stbi_load failed for \"{}\": {}", file, reason != nullptr ? reason : "unknown");
        return false;
    }

    if (create(w, h, internalFormat, mipmapping) == false) {
        stbi_image_free(data);
        return false;
    }

    std::array<GLenum, 5> dataFormatLUT = {GL_RGBA, GL_R, GL_RG, GL_RGB, GL_RGBA };
    if (loadData(dataFormatLUT.at(n), dataType, data) == false) {
        stbi_image_free(data);
        return false;
    }

    stbi_image_free(data);
    return true;
}

bool gl::texture2d::loadData(GLenum dataFormat, GLenum dataType, const void* data) const {
    if (m_id == 0) {
        SPDLOG_WARN("texture2d::loadData called without a texture");
        return false;
    }

    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, dataFormat, dataType, data);
    return true;
}

bool gl::texture2d::generateMipmaps() const {
    if (m_id == 0) {
        SPDLOG_WARN("texture2d::generateMipmaps called without a texture");
        return false;
    }

    if (m_mips == 1) {
        SPDLOG_TRACE("texture2d::generateMipmaps: no mip levels allocated");
        return false;
    }

    glGenerateTextureMipmap(m_id);
    return true;
}

bool gl::texture2d::bind(GLuint bindingPoint) const {
    if (m_id == 0) {
        SPDLOG_WARN("texture2d::bind called without a texture");
        return false;
    }

    glBindTextureUnit(bindingPoint, m_id);
    return true;
}