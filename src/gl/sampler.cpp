#include "gl/sampler.hpp"

#include <algorithm>
#include <iostream>

gl::sampler::sampler() {
    glCreateSamplers(1, &m_id);

    setWrap(REPEAT);
    setFilter(ANISOTROPIC);
    setAnisotropy(16.0f);
}
gl::sampler::~sampler() {
    glDeleteSamplers(1, &m_id);
}

gl::sampler::sampler(sampler&& other) noexcept {
    m_id = other.m_id;
    other.m_id = 0;
}
gl::sampler& gl::sampler::operator=(sampler&& other) noexcept {
    if (this != &other) {
        if (m_id != 0) {
            std::cout << "WARNING: sampler already present on move destination. Overwriting.\n";
            glDeleteSamplers(1, &m_id);
        }
        m_id = other.m_id;
        other.m_id = 0;
    }
    return *this;
}

bool gl::sampler::bind(GLuint bindingPoint) const {
    if (m_id == 0) {
        std::cout << "NOTE: bind called with no sampler present. Did nothing.\n";
        return false;
    }
    glBindSampler(bindingPoint, m_id);
    return true;
}

void gl::sampler::setWrap(samplerWrap wrap) {
    if (m_id == 0) {
        std::cout << "NOTE: setWrap called with no sampler present. Did nothing.\n";
        return;
    }
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_S, wrap);
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_T, wrap);
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_R, wrap);
}

void gl::sampler::setFilter(samplerFilter filter) {
    if (m_id == 0) {
        std::cout << "NOTE: setFilter called with no sampler present. Did nothing.\n";
        return;
    }
    if (filter == BILINEAR) {
        glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else if (filter == TRILINEAR) {
        glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else if (filter == ANISOTROPIC) {
        glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        float maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        glSamplerParameterf(m_id, GL_TEXTURE_MAX_ANISOTROPY, std::min(maxAniso, 16.0f));
    }
}

void gl::sampler::setAnisotropy(GLfloat level) {
    if (m_id == 0) {
        std::cout << "NOTE: setAnisotropy called with no sampler present. Did nothing.\n";
        return;
    }
    float maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
    glSamplerParameterf(m_id, GL_TEXTURE_MAX_ANISOTROPY, std::min(level, maxAniso));
}
