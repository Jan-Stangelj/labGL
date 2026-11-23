#include "gl/platform/window.hpp"
#include "gl/platform/init.hpp"

#include "glad/glad.h"

#include "spdlog/spdlog.h"

#include <string>

namespace {
    void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

    [[maybe_unused]] std::string glString(GLenum name);
    const char* debugSourceName(GLenum source);
    const char* debugTypeName(GLenum type);
    spdlog::level::level_enum debugLevel(GLenum severity);
}

void gl::window::destroy() {
    if (m_window == nullptr && m_context == nullptr && m_emptyVAO == 0) return;

    SPDLOG_TRACE("destroying window, GL context and empty VAO");

    if (m_window != nullptr && m_context != nullptr) select();

    if (m_emptyVAO != 0) glDeleteVertexArrays(1, &m_emptyVAO);
    if (m_context != nullptr) SDL_GL_DestroyContext(m_context);
    if (m_window != nullptr) SDL_DestroyWindow(m_window);

    m_emptyVAO = 0;
    m_context = nullptr;
    m_window = nullptr;
}

gl::window::~window() {
    destroy();
}

gl::window::window(window&& other) noexcept {
    m_window = other.m_window;
    m_context = other.m_context;
    m_emptyVAO = other.m_emptyVAO;

    other.m_window = nullptr;
    other.m_context = nullptr;
    other.m_emptyVAO = 0;
}

gl::window& gl::window::operator=(window&& other) noexcept {
    if (this != &other) {
        destroy();

        m_window = other.m_window;
        m_context = other.m_context;
        m_emptyVAO = other.m_emptyVAO;

        other.m_window = nullptr;
        other.m_context = nullptr;
        other.m_emptyVAO = 0;
    }
    return *this;
}

bool gl::window::create(unsigned int width, unsigned int height, const char* title) {
    destroy();

    m_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL);
    if (m_window == nullptr) {
        SPDLOG_ERROR("SDL_CreateWindow(\"{}\", {}x{}) failed: {}", title, width, height, SDL_GetError());
        return false;
    }
    SPDLOG_INFO("window created: {}x{}", width, height);

    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == nullptr) {
        SPDLOG_ERROR("SDL_GL_CreateContext failed: {}", SDL_GetError());
        destroy();
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SPDLOG_CRITICAL("gladLoadGLLoader failed");
        destroy();
        return false;
    }

    SPDLOG_INFO("OpenGL: {} | {} | {}", glString(GL_VERSION), glString(GL_RENDERER), glString(GL_VENDOR));

    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    } else if (debugContextRequested()) {
        SPDLOG_WARN("OpenGL debug context not available, driver messages will not be reported");
    }

    glGenVertexArrays(1, &m_emptyVAO);
    glBindVertexArray(m_emptyVAO);

    return true;
}

int gl::window::getWidth() const {
    int result = 0;
    if (m_window == nullptr || !SDL_GetWindowSize(m_window, &result, nullptr)) {
        SPDLOG_WARN("getWidth called without a window, returning 0");
        return 0;
    }
    return result;
}

int gl::window::getHeight() const {
    int result = 0;
    if (m_window == nullptr || !SDL_GetWindowSize(m_window, nullptr, &result)) {
        SPDLOG_WARN("getHeight called without a window, returning 0");
        return 0;
    }
    return result;
}

bool gl::window::swapBuffers() {
    if (m_window == nullptr) {
        SPDLOG_WARN("swapBuffers called without a window");
        return false;
    }
    if (!SDL_GL_SwapWindow(m_window)) {
        SPDLOG_ERROR("SDL_GL_SwapWindow failed: {}", SDL_GetError());
        return false;
    }
    return true;
}

bool gl::window::select() const {
    if (m_window == nullptr || m_context == nullptr) {
        SPDLOG_WARN("select called without a window or context");
        return false;
    }
    if (!SDL_GL_MakeCurrent(m_window, m_context)) {
        SPDLOG_ERROR("SDL_GL_MakeCurrent failed: {}", SDL_GetError());
        return false;
    }
    return true;
}

bool gl::window::captureMouse(bool capture) const {
    if (m_window == nullptr) {
        SPDLOG_WARN("captureMouse called without a window");
        return false;
    }
    if (!SDL_SetWindowRelativeMouseMode(m_window, capture)) {
        SPDLOG_WARN("SDL_SetWindowRelativeMouseMode failed: {}", SDL_GetError());
        return false;
    }
    return true;
}

// Source: https://learnopengl.com/In-Practice/Debugging
namespace {

    [[maybe_unused]] std::string glString(GLenum name) {
        const auto* value = glGetString(name);
        return value != nullptr ? std::string(reinterpret_cast<const char*>(value)) : std::string("(unknown)");
    }

    const char* debugSourceName(GLenum source) {
        switch (source) {
            case GL_DEBUG_SOURCE_API:             return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "window system";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY:     return "third party";
            case GL_DEBUG_SOURCE_APPLICATION:     return "application";
            case GL_DEBUG_SOURCE_OTHER:           return "other";
            default:                              return "unknown";
        }
    }

    const char* debugTypeName(GLenum type) {
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               return "error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated behaviour";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined behaviour";
            case GL_DEBUG_TYPE_PORTABILITY:         return "portability";
            case GL_DEBUG_TYPE_PERFORMANCE:         return "performance";
            case GL_DEBUG_TYPE_MARKER:              return "marker";
            case GL_DEBUG_TYPE_PUSH_GROUP:          return "push group";
            case GL_DEBUG_TYPE_POP_GROUP:           return "pop group";
            case GL_DEBUG_TYPE_OTHER:               return "other";
            default:                                return "unknown";
        }
    }

    spdlog::level::level_enum debugLevel(GLenum severity) {
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:         return spdlog::level::err;
            case GL_DEBUG_SEVERITY_MEDIUM:       return spdlog::level::warn;
            case GL_DEBUG_SEVERITY_LOW:          return spdlog::level::info;
            case GL_DEBUG_SEVERITY_NOTIFICATION: return spdlog::level::debug;
            default:                             return spdlog::level::debug;
        }
    }

    void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {

        // Prevent unused warnings
        (void)length;
        (void)userParam;

        // NVIDIA specific chatter: buffer memory hints, shader info, recompile notices, texture state warnings.
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(), debugLevel(severity), "[GL {}] {} (id {}): {}",
                        debugTypeName(type), debugSourceName(source), id, message);
    }

}
