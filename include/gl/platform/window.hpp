#pragma once

#include "glad/glad.h"

#include "SDL3/SDL.h"

namespace gl {

    class window {
    public:

        window() = default;
        ~window();

        window(const window&) = delete;
        window& operator=(const window&) = delete;

        window(window&& other) noexcept;
        window& operator=(window&& other) noexcept;

        bool create(unsigned int width, unsigned int height, const char* title);

        bool swapBuffers();

        bool select() const;

        bool captureMouse(bool capture) const;

        int getWidth() const;
        int getHeight() const;

    private:

        void destroy();

        SDL_Window* m_window = nullptr;
        SDL_GLContext m_context = nullptr;

        GLuint m_emptyVAO = 0;

    };

}
