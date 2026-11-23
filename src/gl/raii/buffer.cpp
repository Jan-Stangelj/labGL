#include "gl/raii/buffer.hpp"

#include "spdlog/spdlog.h"

gl::buffer::~buffer() {
    if (m_id != 0) glDeleteBuffers(1, &m_id);
}

gl::buffer::buffer(buffer&& other) noexcept {
    m_id = other.m_id;
    m_size = other.m_size;

    other.m_id = 0;
    other.m_size = 0;
}

gl::buffer& gl::buffer::operator=(buffer&& other) noexcept {
    if (this != &other) {
        if (m_id != 0) {
            SPDLOG_TRACE("buffer move-assign discarded an existing buffer");
            glDeleteBuffers(1, &m_id);
        }
        m_id = other.m_id;
        m_size = other.m_size;

        other.m_id = 0;
        other.m_size = 0;
    }
    return *this;
}

bool gl::buffer::create(GLsizeiptr size, const void* data) {
    if (size == 0) {
        SPDLOG_ERROR("buffer::create: size must be greater than 0");
        return false;
    }

    if (m_id != 0) {
        SPDLOG_DEBUG("buffer::create: reallocating existing buffer");
        glDeleteBuffers(1, &m_id);
    }

    m_id = 0;
    glCreateBuffers(1, &m_id);

    if (m_id == 0) {
        SPDLOG_ERROR("glCreateBuffers failed");
        return false;
    }

    m_size = size;
    
    glNamedBufferStorage(m_id, size, data, GL_DYNAMIC_STORAGE_BIT);

    return true;
}

bool gl::buffer::write(GLintptr offset, GLsizeiptr size, const void* data) const {
    if (m_id == 0) {
        SPDLOG_WARN("buffer::write called without a buffer");
        return false;
    }
    if (data == nullptr) {
        SPDLOG_ERROR("buffer::write: null data");
        return false;
    }
    if (offset < 0 || size < 0 || offset > m_size || size > m_size - offset) {
        SPDLOG_ERROR("buffer::write: range [{}, {}) exceeds buffer size {}", offset, offset + size, m_size);
        return false;
    }

    glNamedBufferSubData(m_id, offset, size, data);
    return true;
}

bool gl::buffer::bind(bufferTarget type, GLuint bindingPoint) const {
    if (m_id == 0) {
        SPDLOG_WARN("buffer::bind called without a buffer");
        return false;
    }

    glBindBufferBase(static_cast<GLenum>(type), bindingPoint, m_id);
    return true;
}