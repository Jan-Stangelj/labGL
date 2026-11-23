#include "gl/platform/init.hpp"

#include "SDL3/SDL.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <memory>

namespace {

    bool g_debugContextRequested = false;

    bool setAttribute(SDL_GLAttr attribute, const char* name, int value) {
        if (SDL_GL_SetAttribute(attribute, value)) return true;

        SPDLOG_ERROR("SDL_GL_SetAttribute({}) failed: {}", name, SDL_GetError());
        return false;
    }

}

bool gl::debugContextRequested() {
    return g_debugContextRequested;
}

bool gl::init(bool debug) {
    g_debugContextRequested = debug;

    spdlog::set_default_logger(std::make_shared<spdlog::logger>("gl", std::make_shared<spdlog::sinks::stdout_color_sink_mt>()));
    spdlog::set_level(debug ? spdlog::level::trace : spdlog::level::info);
    spdlog::flush_on(spdlog::level::warn);

    // Explain missing trace/debug output in builds that strip it at compile time.
#if SPDLOG_ACTIVE_LEVEL > SPDLOG_LEVEL_DEBUG
    if (debug) {
        SPDLOG_WARN("built with compile-time log floor {}, trace/debug messages are not available", spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL)));
    }
#endif

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SPDLOG_CRITICAL("SDL_Init(SDL_INIT_VIDEO) failed: {}", SDL_GetError());
        return false;
    }

    if (!setAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, "context major version", 4)) return false;
    if (!setAttribute(SDL_GL_CONTEXT_MINOR_VERSION, "context minor version", 6)) return false;
    if (!setAttribute(SDL_GL_CONTEXT_PROFILE_MASK, "core profile", SDL_GL_CONTEXT_PROFILE_CORE)) return false;
    if (!setAttribute(SDL_GL_DEPTH_SIZE, "depth size", 24)) return false;
    if (debug && !setAttribute(SDL_GL_CONTEXT_FLAGS, "debug flag", SDL_GL_CONTEXT_DEBUG_FLAG)) return false;

    SPDLOG_INFO("SDL initialized, OpenGL 4.6 core requested");
    return true;
}

bool gl::quit() {
    SPDLOG_INFO("shutting down SDL");
    SDL_Quit();
    spdlog::shutdown();
    return true;
}
