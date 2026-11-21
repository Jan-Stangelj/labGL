#include "gl/platform/window.hpp"

#include "glad/glad.h"

#include <iostream>

namespace {
    void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);
}

void gl::window::destroy() {
    if (m_window == nullptr && m_context == nullptr && m_emptyVAO == 0) return;

    std::cout << "NOTE: Deleting existing window, context and empty VAO.\n";

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
        std::cerr << "ERROR: Failed to create SDL window, aborting: " << SDL_GetError() << '\n';
        return false;
    }
    m_context = SDL_GL_CreateContext(m_window);
    if (m_context == nullptr) {
        std::cerr << "ERROR: Failed to create OpenGL context, aborting: " << SDL_GetError() << '\n';
        destroy();
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize GLAD. Aborting.\n";
        destroy();
        return false;
    }

    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glGenVertexArrays(1, &m_emptyVAO);
    glBindVertexArray(m_emptyVAO);

    return true;
}

int gl::window::getWidth() const {
    int result = 0;
    if (m_window == nullptr || !SDL_GetWindowSize(m_window, &result, nullptr)) {
        std::cout << "NOTE: width called with no window present. Did nothing.\n";
        return 0;
    }
    return result;
}

int gl::window::getHeight() const {
    int result = 0;
    if (m_window == nullptr || !SDL_GetWindowSize(m_window, nullptr, &result)) {
        std::cout << "NOTE: height called with no window present. Did nothing.\n";
        return 0;
    }
    return result;
}

bool gl::window::swapBuffers() {
    if (m_window == nullptr) {
        std::cout << "NOTE: swapBuffers called with no window present. Did nothing.\n";
        return false;
    }
    if (!SDL_GL_SwapWindow(m_window)) {
        std::cout << "WARNING: Failed to swap buffers: " << SDL_GetError() << '\n';
        return false;
    }
    return true;
}

bool gl::window::select() const {
    if (m_window == nullptr || m_context == nullptr) {
        std::cout << "NOTE: select called with no window or context present. Did nothing.\n";
        return false;
    }
    if (!SDL_GL_MakeCurrent(m_window, m_context)) {
        std::cout << "WARNING: Failed to make context current: " << SDL_GetError() << '\n';
        return false;
    }
    return true;
}

bool gl::window::captureMouse(bool capture) const {
    if (m_window == nullptr) {
        std::cout << "NOTE: captureMouse called with no window present. Did nothing.\n";
        return false;
    }
    if (!SDL_SetWindowRelativeMouseMode(m_window, capture)) {
        std::cout << "NOTE: Failed to capture mouse.\n";
        return false;
    }
    return true;
}

// Source: https://learnopengl.com/In-Practice/Debugging
namespace {

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {

    // Prevent unused warnings
    (void)length;
    (void)userParam;

    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

} // anonymous namespace
